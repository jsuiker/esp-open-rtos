/*
 * Part of esp-open-rtos
 * Copyright (C) 2016 Jonathan Hartsuiker (https://github.com/jsuiker)
 * BSD Licensed as described in the file LICENSE
 *
 */

char* http_assemble_header(char* key, char* separator, char* value);
char* http_get_request(char* host, char* endpoint);
char* http_post_request(char* host, char* endpoint, char* content_type, char* body);
bool http_process_request(char* server, char* port, char* request, uint16_t max_buffer_length);