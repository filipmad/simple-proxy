// COMP30023 Project 2: Filip Madyarov & Jarin Lewis 2025
// Code influenced by Beej's Guide to Network Programming
// HTTP request parsing, header retrieval and functionality for Stage 3

// Modules
#include "http_request.h"


/******************** PRIVATE FUNCTIONS ********************/

/**
 * Removes the headers from the body, null terminates and stores the resulting body in buffer
 */
static void null_term_header(const char *message, char *buffer, int buffer_len) {
    const char *header_end = strstr(message, "\r\n\r\n");
    header_end += strlen("\r\n\r\n");    // skip past the CRLF
    
    int header_len = header_end - message;
    if (header_len >= buffer_len) {
        fprintf(stderr, "header extraction buffer too small\n");
        return;
    }

    memcpy(buffer, message, header_len);
    buffer[header_len] = '\0';
}

/**
 * Extract the tail from the body buffer and store it in tail
 */
void extract_tail(const char *buffer, char *tail) {
    char *tail_end = strstr(buffer, "\r\n\r\n");

    if (!tail_end) {
        fprintf(stderr, "Invalid HTTP request\n");
        exit(EXIT_FAILURE);
    }

    char *tail_start = tail_end;

    // while character before tail_start != '\n'
    while (tail_start > buffer && tail_start[-1] != '\n') {
        tail_start--;
    }

    // length of the last line
    int len = tail_end - tail_start;

    // copy bytes excluding the \r\n\r\n characters
    strncpy(tail, tail_start, len);
    tail[len] = '\0';
}


/**
 * Extracts the first line of the request and stores it in the the buffer
 */
void extract_line(const char *request, char *buffer, size_t buffer_size) {
    int i = 0;

    while (request[i] != '\n' && i < buffer_size-1) {
        buffer[i] = request[i];
        i++;
    }
    buffer[i] = '\0';
}

/**
 * Takes the URI from the request and parses it into a host and path
 */
void parse_uri(struct http_header *request) {
    // parse host name and path
    if (sscanf(request->uri, HTTP_HEADER_FORMAT, request->host, request->path) == 2) {
        // successfully parsed hostname and path
        prepend(request->path, '/');
        return;
    }
    else if (sscanf(request->uri, HOSTNAME_FORMAT, request->host) == 1) {
        // there is no path
        strcpy(request->path, "/");
    }
    else {
        fprintf(stderr, "Error; request has not been parsed\n.");
    }
}


/**
 * Adds a character in front of a string 
 */
void prepend(char *str, char c) {
    int len = strlen(str);
    memmove(str + 1, str, len + 1);
    str[0] = c;
}


/**
 * Checks to see if custom header was added to the tail of HTTP request.
 */ 
static int check_custom_header(struct http_header request) {
    char tmp[TAIL_LEN];
    strcpy(tmp, request.tail);
    int i = 0;
    while (tmp[i] != ':') i++;
    tmp[i] = '\0';

    if (!strcmp(tmp, "Connection") || !strcmp(tmp, "Proxy-Connection")) {
        return 0;
    }
    return 1;
}

/******************** PUBLIC FUNCTIONS ********************/


/**
 * Takes a string buffer and extracts relevant information to
 * build the full request for analysis. Stores it in request
 */ 
void build_request(char *buffer, struct http_header request) {
    // custom headers were added to client request
    if (check_custom_header(request)) {
        const char http_request_format[] =
        "%s %s %s\r\n"  // method, path, protocol
        "Host: %s\r\n"
        "User-Agent: %s\r\n"
        "Accept: %s\r\n"
        "Connection: close\r\n"
        "%s\r\n"
        "\r\n";
    
        sprintf(buffer, http_request_format, 
            request.method, request.path, request.protocol,
            request.host,
            request.user_agent,
            request.accept,
            request.tail);
    }
    // no custom headers added to client request
    else {
        const char http_request_format[] =
        "%s %s %s\r\n"  // method, path, protocol
        "Host: %s\r\n"
        "User-Agent: %s\r\n"
        "Accept: %s\r\n"
        "Connection: close\r\n"
        "\r\n";
    
        sprintf(buffer, http_request_format, 
            request.method, request.path, request.protocol,
            request.host,
            request.user_agent,
            request.accept);
    }
}

/**
 * Checks to see if custom header was added to the tail of HTTP request.
 */ 
void parse_request(const char *buffer, int bytes_read, struct http_header *request) {
    // copy full request
    memcpy(request->http_request, buffer, bytes_read);
    request->http_request[bytes_read] = '\0';

    // extract request line from HTTP request
    char request_line[REQUEST_LINE_LEN];    // magic numbers: MUST CHANGE
    extract_line(buffer, request_line, REQUEST_LINE_LEN);

    // extract method, url, protocol from request line
    if (sscanf(request_line, REQUEST_LINE_FORMAT, request->method, request->uri, request->protocol) != REQUEST_LINE_NUM) {
        perror("Request line extraction failed");
        exit(EXIT_FAILURE);
    }
    parse_uri(request);

    // extract user agent and accept headers
    extract_header(buffer, "User-Agent:", request->user_agent, USER_AGENT_LEN);
    extract_header(buffer, "Accept:", request->accept, ACCEPT_LEN);

    // extract tail from HTTP request
    extract_tail(buffer, request->tail);
}

/**
 * Returns the length of the response string by
 * extracting the header from the response
 */ 
int content_length(const char *response) {
    char header_str[HTTP_REQUEST_LEN];
    null_term_header(response, header_str, HTTP_REQUEST_LEN);

    const char *p = header_str;
    while ((p = strstr(p, CONTENT_LEN_HEADER)) != NULL) {
        // ensure it starts a new line
        if (p == header_str || *(p - 1) == '\n' || *(p - 1) == '\r') {
            p += strlen(CONTENT_LEN_HEADER);
            while (*p == ' ') p++;
            return atoi(p);
        }
        p += strlen(CONTENT_LEN_HEADER);
    }
    return -1;
}

/**
 * Extracts the header as defined by message 
 * from request or response and returns it in the buffer
 */ 
void extract_header(const char *message, const char *key, char *buffer, size_t buffer_size) {
    assert(message && key);

    // null terminate headers
    char header_str[HTTP_REQUEST_LEN];
    null_term_header(message, header_str, HTTP_REQUEST_LEN);

    // find the key in the headers
    const char *start = strstr(header_str, key);
    if (!start) {
        buffer[0] = '\0';
        return;
    }

    // make sure its at the start of a new line
    if (start != header_str && *(start -1) != '\n') {
        buffer[0] = '\0';
        return;
    }

    // skip past the header string and whitespace
    start += strlen(key);
    while (start[0] == ' ') start++;

    // find the end of line
    const char *end = strstr(start, "\r\n");
    if (!end) {
        buffer[0] = '\0';
        return;
    }

    size_t buffer_len = end - start;
    memcpy(buffer, start, buffer_len);
    buffer[buffer_len] = '\0';

}

/**
 * Extracts the max age from a line as described in Stage 3 & 4
 */ 
uint32_t extract_max_age(const char *line) {
    if (line == NULL) 
        return 0;

    int line_len = strlen(line);
    char copy[line_len + 1];
    copy[line_len] = '\0';
    strcpy(copy, line);
    to_lowercase(copy);
    
    char *p = strstr(copy, MAX_AGE_FIELD);

    if (p == NULL) {
        return 0;
    }

    p += strlen(MAX_AGE_FIELD);
    return atoi(p);
}

/**
 * Edits a string to be all in lowercase
 */ 
void to_lowercase(char* str) {
    while (*str) {
        *str = tolower((unsigned char)*str);
        str++;
    }
}