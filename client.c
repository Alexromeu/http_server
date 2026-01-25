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
#define MAXDATASIZE 100
#define IP_ADDRESS "127.0.0.1"


void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in*)sa)->sin_addr);
    } 

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

struct socket_info {
    int sockfd, numbytes, new_fd;
    int rv;
    int sin_size;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage their_address;
    char s[INET6_ADDRSTRLEN];
    char buf[MAXDATASIZE];
};

int create_socket(struct socket_info *sock_inf) {
    memset(&(sock_inf->hints), 0, sizeof sock_inf->hints);
    sock_inf->hints.ai_family = AF_UNSPEC;
    sock_inf->hints.ai_socktype = SOCK_STREAM;

    if ((sock_inf->rv = getaddrinfo(IP_ADDRESS, PORT, &(sock_inf->hints), &(sock_inf->servinfo))) != 0) {
        perror("error getting address info");
        return -1;
    }

    for (sock_inf->p = sock_inf->servinfo; sock_inf->p != NULL; sock_inf->p = sock_inf->p->ai_next) {
            if ((sock_inf->sockfd = socket(sock_inf->p->ai_family,
                                        sock_inf->p->ai_socktype, 
                                        sock_inf->p->ai_protocol)) == -1) {
                perror("socket error");
                continue;                        
            }

            if (connect(sock_inf->sockfd, sock_inf->p->ai_addr, sock_inf->p->ai_addrlen) == -1) {
                close(sock_inf->sockfd);
                perror("error connecting to socket");
                continue;
            }

            break;
    }

    if (sock_inf->p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        return -1;
    }
    
    inet_ntop(sock_inf->p->ai_family, get_in_addr((struct sockaddr*)sock_inf->p->ai_addr), 
                    sock_inf->s, sizeof sock_inf->s);
    
    printf("client connected to %s\n",sock_inf->s);
    freeaddrinfo(sock_inf->servinfo);

    return 1;
}  

int send_all(int sck, char* buf, int *len) {
    int total = 0;
    int bytesleft = *len;
    int n;

    while (total < *len) {
        n = send(sck, buf + total, bytesleft, 0);
        if (n == -1) break;
        total += n;
        bytesleft -= n;
    }

    *len = total;
    return n == -1 ? -1 : 0;
}

int recv_data(int sockfd, struct data_payload *data) {
    ssize_t recv_bytes = 0;
    ssize_t bytes_left = data->len;
    ssize_t n;

    while (recv_bytes < data->len) {
        n = recv(sockfd, data, data->len, 0);
        if (n == -1) break;
        recv_bytes += n;
        bytes_left -= n;
    }

    data->len = recv_bytes;
    return n == -1 ? -1 : 0;
}


void send_info(struct socket_info *sinfo, char* buf) {
    sinfo->sin_size = sizeof sinfo->their_address;
    int buf_len = strlen(buf);
    
    if (send_all(sinfo->sockfd, buf, &buf_len) == -1) {
        perror("send error");
    }
        
    printf("Data sent successfully. Waiting 1 second...\n");

    close(sinfo->sockfd);
}


int main() {
    struct socket_info sock_inf;
    struct data_payload data;
    char* conv_to_string;
    data.len = sizeof(data);
    data.id = 1;
    memcpy(&(data.username), "alexander", sizeof("alexander"));
    memcpy(&(data.password), "alex123", sizeof("alex123"));
    memcpy(&(data.data), "this is a test", sizeof("this is a test"));

    int i = create_socket(&sock_inf);
    if (i <= 0) {
        perror("socket creating error");
        return -1;
    }

    conv_to_string = parse(&data);
    send_info(&sock_inf, conv_to_string);
    return 0;
}
