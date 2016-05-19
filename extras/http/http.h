/*
 * Part of esp-open-rtos
 * Copyright (C) 2016 Jonathan Hartsuiker (https://github.com/jsuiker)
 * BSD Licensed as described in the file LICENSE
 *
 */

char* http_assemble_header(char* key, char* separator, char* value);
char* http_assemble_request(const char* req, char** headers, int header_count, char* body);
bool http_process_request(char* server, char* port, char* request, uint16_t max_buffer_length);