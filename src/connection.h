/**
 * COMP30024 Project 2: Connection Functionality
 * Filip Madyarov & Jarin Lewis
 */

#ifndef _CONNECTION_H_
#define _CONNECTION_H_

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

#include "http_request.h"
#include "cache.h"


// CONSTANTS
#define BACKLOG 10
#define BUFFER_SIZE 8192    // buffer must be changed to be dynamic
#define RESPONSE_SIZE 130000 // response must be changed to be dynamic

// extern volatile sig_atomic_t running;

// FLAGS
#define RUNNING 1
#define FAIL -1
#define SUCCESS 0

#define HTTP_PORT "80"
#define HTTP_HEADER_END "\r\n\r\n"


/* Function Interface */

/**
 * Start Proxy Function
 * Initialises the server socket connection for the proxy
 */
void startProxy(char *port, int cache);

/**
 * Client Response Handler
 * Handles HTTP Responses & logs relevant information for Task One
 */
void handle_client(int client_socket, CacheEntry** cache);

/**
 * Connects to origin server
 * returns origin file descriptor on success, `ORIGIN_FAIL` on fail
 */
int connect_origin(struct http_header request, char *port);

/**
 * forwards the request to origin server
 * returns `ORIGIN_FAIL` or `ORIGIN_SUCCESS`
 */
int request_origin(int origin_fd, struct http_header client_request);

/**
 * receives bytes from fd into buffer
 * returns FAIL or total bytes read on success
 */
int recv_client_request(int fd, char *buffer, size_t buffer_size);

/**
 * receives response from origin server
 * returns total bytes read on success, FAIL on failure
 */
int recv_origin_response(int origin_fd, char *response, size_t response_size);

/**
 * sends message to server described by fd
 * returns FAIL or SUCCESS
 */
int send_server(int fd, char *message, int message_len);

#endif