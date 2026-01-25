#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>
#include "data.h"

#define PORT "3490"
#define BACKLOG 10



void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET)
        return &(((struct sockaddr_in*)sa)->sin_addr);
    return &(((struct sockaddr_in6*)sa)->sin6_addr);
}

void st_to_data(struct data_payload *out, char *str) {
    char temp[256];
    strncpy(temp, str, sizeof(temp));
    temp[sizeof(temp)-1] = '\0';

    char *token = strtok(temp, "$");
    int field = 0;

    while (token != NULL) {
        switch (field) {
            case 0:
                out->len = atoi(token);
                break;
            case 1:
                out->id = atoi(token);
                break;
            case 2:
                strncpy(out->username, token, sizeof(out->username));
                out->username[31] = '\0';
                break;
            case 3:
                strncpy(out->password, token, sizeof(out->password));
                out->password[31] = '\0';
                break;
            case 4:
                strncpy(out->data, token, sizeof(out->data));
                out->data[31] = '\0';
                break;
        }
        field++;
        token = strtok(NULL, "$");
    }
}


void safe_to_file(FILE *fp, struct data_payload *data) {
    if (!feof(fp)) {
        fseek(fp, 0L, SEEK_END);
    }

    size_t total_size = ftell(fp);
    char* str_data = parse(data);
    size_t write_size = strlen(str_data);
    size_t bytes_wrote = 0;
    size_t total_bytes_written = 0;
    while ((bytes_wrote = fwrite(str_data, sizeof(char), write_size, fp)) < total_bytes_written) {
        
       if (ferror(fp)) {
            perror("error with write to file");
            break;
       }
       
       total_bytes_written += bytes_wrote;
    }
    fwrite("\n", 1, 1, fp);
}


int main() {
    int sockfd, new_fd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_addr;
    socklen_t sin_size;
    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;
    FILE *fp;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((fp = fopen("data.txt", "a")) == NULL) {
        perror("File error."); 
    }

    if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
        exit(1);
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == -1) continue;

        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));

        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            close(sockfd);
            continue;
        }
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(1);
    }

    freeaddrinfo(servinfo);

    if (listen(sockfd, BACKLOG) == -1) {
        perror("listen");
        exit(1);
    }

    printf("server: waiting for connections on port %s...\n", PORT);


    while (1) {
        sin_size = sizeof their_addr;
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) continue;

        inet_ntop(their_addr.ss_family,
                  get_in_addr((struct sockaddr *)&their_addr),
                  s, sizeof s);

        printf("server: got connection from %s\n", s);

        struct data_payload data;
        char buf_to_recv[256];
        ssize_t n = recv(new_fd, buf_to_recv, sizeof(buf_to_recv), 0);

        if (n <= 0) {
            perror("recv");
            close(new_fd);
            continue;
        }

        st_to_data(&data, buf_to_recv);
        safe_to_file(fp, &data);
        
        printf("Received struct:\n");
        printf("len: %d\n", data.len);
        printf("id: %d\n", data.id);
        printf("username: %s\n", data.username);
        printf("password: %s\n", data.password);
        printf("data: %s\n", data.data);

        char reply[] = "Server received your struct!";
        send(new_fd, reply, strlen(reply), 0);
        
        close(new_fd);
    }
    fclose(fp);
    return 0;
}

































