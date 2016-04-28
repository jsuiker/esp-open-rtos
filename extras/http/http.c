/*
 * Part of esp-open-rtos
 * Copyright (C) 2016 Jonathan Hartsuiker (https://github.com/jsuiker)
 * BSD Licensed as described in the file LICENSE
 *
 */

#include "lwip/err.h"
#include "lwip/sockets.h"
#include "lwip/sys.h"
#include "lwip/netdb.h"
#include "lwip/dns.h"
#include "http_parser.h"
#include <string.h>

typedef struct {
    int sock;
    void* buffer;
    int buf_len;
} custom_data_t;


void http_process_response(char* buffer, uint8_t buffer_length) {
    for (uint8_t i = 0; i < buffer_length; i++) {
        printf("%c", buffer[i]);
    }
}

char* http_assemble_header(char* key, char* separator, char* value) {
    int size = strlen(key) + strlen(separator) + strlen(value) + 4;
    char* result = calloc((size), sizeof(char));
    snprintf(result, size, "%s%s %s\r\n", key, separator, value);
    return result;
}

char* http_get_request(char* host, char* endpoint) {
    char* request = http_assemble_header("GET ", endpoint, "HTTP/1.1");
    char* host_header = http_assemble_header("Host", ":", host);
    char* user_agent_header = http_assemble_header("User-Agent", ":", "esp-open-rtos/0.1 esp8266");
    char* connection_header = http_assemble_header("Connection", ":", "Close");

    int request_size = strlen(request) + strlen(host_header) + strlen(user_agent_header) + strlen(connection_header) + 3;
    char* result = calloc((request_size), sizeof(char));
    snprintf(result, request_size, "%s%s%s%s\r\n", request, host_header, user_agent_header, connection_header);
    return result;
}

char* http_post_request(char* host, char* endpoint, char* content_type, char* body) {
    char* request = http_assemble_header("POST ", endpoint, "HTTP/1.1");
    char* host_header = http_assemble_header("Host", ":" , host);
    char* user_agent_header = http_assemble_header("User-Agent", ":", "esp-open-rtos/0.1 esp8266");

    char content_length[10]; sprintf(content_length, "%d", strlen(body));

    char* content_type_header = http_assemble_header("Content-Type", ":", content_type);
    char* content_length_header = http_assemble_header("Content-Length", ":", content_length);

    int request_size = strlen(request) + strlen(host_header) + strlen(user_agent_header) + strlen(content_type_header) + strlen(content_length_header) + strlen(body) + 5;
    char* result = calloc((request_size), sizeof(char));
    snprintf(result, request_size, "%s%s%s%s%s\r\n%s\r\n", request, host_header, user_agent_header, content_type_header, content_length_header, body);
    return result;
}

int on_body_data(http_parser* parser, const char *at, size_t length) {
    /*    access to thread local custom_data_t struct.
     *    Use this access save parsed data for later use into thread local
     *    buffer, or communicate over socket
     */
    

    // parser->data;
    return 0;
}

int on_header_field_callback(http_parser* parser, const char *at, size_t length) {





    return 0;
}

bool http_process_request(char* server, char* port, char* request,  uint16_t max_buffer_length) {

    char out_buf[max_buffer_length];
    bzero(out_buf, max_buffer_length);

    http_parser_settings parser_settings;
    parser_settings.on_body = on_body_data;
    parser_settings.on_header_field = on_header_field_callback;

    printf("sizeof(http_parser): %d\r\n", sizeof(http_parser));
    http_parser *parser = malloc(sizeof(http_parser));

    const struct addrinfo hints = {
        .ai_family = AF_INET,
        .ai_socktype = SOCK_STREAM,
    };

    struct addrinfo *res;

    int err = getaddrinfo(server, port, &hints, &res);

    if(err != 0 || res == NULL) {
        printf("DNS lookup failed err=%d res=%p\r\n", err, res);
        if(res)
            freeaddrinfo(res);
        return false;
    }

    /* Note: inet_ntoa is non-reentrant, look at ipaddr_ntoa_r for "real" code */
    // struct in_addr *addr = &((struct sockaddr_in *)res->ai_addr)->sin_addr;
    // printf("DNS lookup succeeded. IP=%s\r\n", inet_ntoa(*addr));

    int s = socket(res->ai_family, res->ai_socktype, 0);
    if(s < 0) {
        printf("... Failed to allocate socket.\r\n");
        freeaddrinfo(res);
        return false;
    }

    if(connect(s, res->ai_addr, res->ai_addrlen) != 0) {
        close(s);
        freeaddrinfo(res);
        printf("... socket connect failed.\r\n");
        return false;
    }

    freeaddrinfo(res);

    printf("\r\n%s\r\n", request);

    if (write(s, request, strlen(request)) < 0) {
        printf("... socket send failed\r\n");
        close(s);
        return false;
    }
    
    http_parser_init(parser, HTTP_RESPONSE);

    static char recv_buf[128];
    int r;
    do {
        bzero(recv_buf, 128);
        r = read(s, recv_buf, 127);
        if(r > 0) {
            strcat(out_buf, recv_buf);
            // printf("%s", recv_buf);
        }
    } while(r > 0);
    close(s);

    http_parser_execute(parser, &parser_settings, out_buf, 0);
    return true;
}