#ifndef DATA_H
#define DATA_H

extern char* http_resp;

struct __attribute__((packed)) data_payload{
    int len; //length of data_payload instance
    int id;
    char username[32];
    char password[32];
    char data[32]; 
} ;

char* parse(struct data_payload *data_p);
    

#endif