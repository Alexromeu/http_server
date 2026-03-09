#include "data.h"
#include <stdio.h>

char* http_resp =     
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: text/html\r\n"
    "Connection: close\r\n"
    "\r\n"  
    "<!DOCTYPE html>\n"
    "<html lang='en'>\n"
    "<head>\n"
    "    <meta charset='UTF-8'>\n"
    "    <meta name='viewport' content='width=device-width, initial-scale=1.0'>\n"
    "    <title>Socket Server 2026</title>\n"
    "    <style>body { font-family: sans-serif; text-align: center; padding: 50px; }</style>\n"
    "</head>\n"
    "<body>\n"
    "    <h1>Connected Successfully!</h1>\n"
    "    <p>This page was sent from a C socket server.</p>\n"
    "</body>\n"
    "</html>";


char* parse(struct data_payload *data_p) {
    static char res[256];

    int len = sizeof(*data_p);
    snprintf(res, sizeof(res), 
    "%d$%d$%s$%s$%s",
    len,
    data_p->id,
    data_p->username,
    data_p->password,
    data_p->data
    );

    return res;
}

   