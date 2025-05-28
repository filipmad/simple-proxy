/**
 * For parsing and reading HTTP requests
 */

#ifndef _HTTP_REQUEST_H_
#define _HTTP_REQUEST_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdint.h>
#include <assert.h>

/**
 * FORMATTING
 */
#define REQUEST_LINE_FORMAT "%s %s %s"
#define HTTP_HEADER_FORMAT "http://%63[^:/]/%127[^\n]"
#define HOSTNAME_FORMAT "http://%63[^:/]/"
#define CONTENT_LEN_HEADER "Content-Length:"
#define CACHE_CONTROL_HEADER "Cache-Control:"
#define MAX_AGE_FIELD "max-age="

/**
 * CONSTANTS
 */
#define REQUEST_LINE_NUM 3
#define DEFAULT_HEADER_NUM 4
#define HTTP_REQUEST_LEN 10000  // change to dynamic
#define METHOD_LEN 4
#define HOST_LEN 256
#define PATH_LEN 2048
#define URI_LEN (HOST_LEN + PATH_LEN)
#define PROTOCOL_LEN 16
#define USER_AGENT_LEN 64
#define ACCEPT_LEN 64
#define TAIL_LEN 8192
#define REQUEST_LINE_LEN (METHOD_LEN + 1 + URI_LEN + 1 + PROTOCOL_LEN)
#define CACHE_CONTROL_LEN 64


/**
 * STRUCTS
 */

// contains the header lines of a http request
struct http_header {
    char http_request[HTTP_REQUEST_LEN];    // change
    char method[METHOD_LEN];
    char uri[URI_LEN];
    char protocol[PROTOCOL_LEN];
    char host[HOST_LEN];                // full domain name has max 253 chars
    char path[PATH_LEN];                // max length of URL is 2048 chars according to some sources
    char user_agent[USER_AGENT_LEN];
    char accept[ACCEPT_LEN];
    char tail[TAIL_LEN];                // this must be changed to dynamic
};

/* FUNCTION INTERFACE */

// extracts the request line of http request
void extract_line(const char *buffer, char *line, size_t buffer_size);

// parse http request, stores method in [method], URL in [URL], protocol in [protocol]
void parse_request(const char *buffer, int bytes_read, struct http_header *request);

// extract the host name and path from uri
void parse_uri(struct http_header *request);

// prepends char c to string str
void prepend(char *str, char c);

// builds the request to be sent to origin, stores in buffer
void build_request(char *buffer, struct http_header request);

// extracts the last line, stores in tail
void extract_tail(const char *buffer, char *tail);

// extracts the Content-Length header value as an int from response
int content_length(const char *response);

// extracts the Cache-Control header
void cache_control(const char *response);

// extracts the string in the header indicated by [header], stores in [buffer]
void extract_header(const char *message, const char *key, char *buffer, size_t buffer_size);

// extracts and returns the uin32_t value stored in max_age field
uint32_t extract_max_age(const char *line);

// converts string to lower case
void to_lowercase(char* str);

#endif