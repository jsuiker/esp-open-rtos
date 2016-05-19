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

/*
 *
 *  The following callbacks are designated as Data Callbacks by the http_parser documentation...
 *
 */

int on_status(http_parser* parser, const char *at, size_t length) {

	printf("HTTP/%i.%i %i ", parser->http_major, parser->http_minor, parser->status_code);
	printf("%.*s\r\n", length, at);

	return 0;
}

int on_header_field(http_parser* parser, const char *at, size_t length) {

	printf("%.*s: ", length, at);

	return 0;
}

int on_header_value(http_parser* parser, const char *at, size_t length) {

	printf("%.*s\r\n", length, at);

	return 0;
}

int on_body(http_parser* parser, const char *at, size_t length) {

	printf("%.*s\r\n", length, at);

	return 0;
}

int on_url(http_parser* parser, const char *at, size_t length) {
	// NOT YET IMPLEMENTED
	return 0;
}

/*
 *
 *  The following callbacks are designated as Notification Callbacks by the http_parser documentation...
 *
 */

int on_headers_complete(http_parser* parser) {
	
	printf("\r\n");

	return 0;
}

int on_message_begin(http_parser* parser) {
	// NOT YET IMPLEMENTED
	return 0;
}

int on_message_complete(http_parser* parser) {
	// NOT YET IMPLEMENTED
	return 0;
}

int on_chunk_header(http_parser* parser) {
	// NOT YET IMPLEMENTED
	return 0;
}

int on_chunk_complete(http_parser* parser) {
	// NOT YET IMPLEMENTED
	return 0;
}



bool http_process_request(char* server, char* port, char* request,  uint16_t max_buffer_length) {

	char out_buf[max_buffer_length];
	bzero(out_buf, max_buffer_length);

	http_parser_settings ps;

	ps.on_status               = on_status;
	ps.on_header_field         = on_header_field;
	ps.on_header_value         = on_header_value;
	ps.on_body                 = on_body;
	ps.on_url                  = on_url;
	ps.on_headers_complete     = on_headers_complete;
	ps.on_message_begin        = on_message_begin;
	ps.on_message_complete     = on_message_complete;
	ps.on_chunk_header         = on_chunk_header;
	ps.on_chunk_complete       = on_chunk_complete;


	http_parser *parser = malloc(sizeof(http_parser));
	http_parser_init(parser, HTTP_RESPONSE);

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

	if (write(s, request, strlen(request)) < 0) {
		printf("... socket send failed\r\n");
		close(s);
		return false;
	}

	static char recv_buf[128];
	int r;
	do {
		bzero(recv_buf, 128);
		r = read(s, recv_buf, 127);
		if(r > 0) {
			strcat(out_buf, recv_buf);
		}
	} while(r > 0);
	close(s);

	// printf("\r\n=============== RAW   RESPONSE ================\r\n");
	// printf("%s\r\n", out_buf);
	printf("\r\n============== PARSED RESPONSE ================\r\n");
	http_parser_execute(parser, &ps, out_buf, strlen(out_buf));

	return true;
}

char* http_assemble_header(char* key, char* separator, char* value) {
	size_t key_len = strlen(key);
	size_t sep_len = strlen(separator);
	size_t val_len = strlen(value);
	size_t header_len = key_len + sep_len + val_len + 1;

	char* header = (char*) calloc(header_len, sizeof(char));
	strncat(strncat(strncat(header, key, key_len), separator, sep_len), value, val_len);
	
	return header;
}

char* http_assemble_request(const char* r, char** headers, int header_count, char* body)
{
	size_t body_len = strlen(body);
	size_t header_len = 0;
	size_t request_len = strlen(r) + 1;
	
	char* request = (char*) calloc(request_len, sizeof(char));
	strncat(request, r, strlen(r));

	for(int i = 0; i < header_count; ++i)
	{
		header_len = strlen(headers[i]);
		request_len += header_len + 2;
		request = (char*) realloc(request, request_len);
		strncat(strncat(request, "\r\n", 2), headers[i], header_len);
	}

	request_len += 4 + body_len;
	request = (char*) realloc(request, request_len);
	strncat(strncat(request, "\r\n\r\n", 4), body, body_len);

	return request;
}