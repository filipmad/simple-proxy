// COMP30023 Project 2: Filip Madyarov & Jarin Lewis 2025
// Code influenced by Beej's Guide to Network Programming
// Connection Functionality for Socket Handling & Data Retrieval

// Modules
#include <pthread.h>
#include <time.h>
#include <unistd.h>

#include "connection.h"
#include "cache.h"
#include "cache_eviction.h"


/******************** PRIVATE FUNCTIONS ********************/

/**
 * Loops through socket results and bind()s to first valid result
 */
static void bind_sock(int *server_fd, struct addrinfo *results) {
    struct addrinfo *p;
    int enable = 1;

    for (p = results; p != NULL; p = p->ai_next) {
        if ((*server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket");
            continue;
        }

        if (setsockopt(*server_fd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) == -1) {
            perror("setsockopt SO_REUSEADDR");
            exit(EXIT_FAILURE);
        }

        // socket is IPv6 preferred, disable IPv6 only flag (IPV6_V6ONLY) to allow IPv4
        if (p->ai_family == AF_INET6) {
            int disable = 0;
            if (setsockopt(*server_fd, IPPROTO_IPV6, IPV6_V6ONLY, &disable, sizeof(disable)) == -1) {
                perror("setsockopt IPV6_V6ONLY");
                // continue
            }
        }

        if (bind(*server_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(*server_fd);
            perror("server: bind");
            continue;
        }

        break;
    }

    freeaddrinfo(results); 

    if (p == NULL) {
        fprintf(stderr, "proxy: failed to bind\n");
        exit(EXIT_FAILURE);
    }
}


/**
 * Loops through socket results and connect()s to first valid result
 */
static int connect_sock(int *server_fd, struct addrinfo *results) {
    struct addrinfo *p;

    for (p = results; p != NULL; p = p->ai_next) {
        if ((*server_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("proxy: socket");
            continue;
        }

        if (connect(*server_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(*server_fd);
            perror("proxy: connect");
            continue;
        }

        break;
    }

    freeaddrinfo(results);

    if (p == NULL) {
        return FAIL;
    }

    return SUCCESS;
}


/******************** PUBLIC FUNCTIONS ********************/
/**
 * Bind sockets to local ports and begin listening to & accepting HTTP requests
 */
void startProxy(char *port_number, int cacheEnabled) {

    // initialise server structures
    int server_fd, client_fd;
    struct addrinfo hints, *results;            // socket address structures
    struct sockaddr_storage client_addr;        // client address info, can store IPv4 or IPv6
    socklen_t addrlen;
    CacheEntry **cache = NULL;

    // initialise cache
    if (cacheEnabled) {
        cache = initialiseCache();
    }
    
    // specify the criteria for the socket addresses we need
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET6;                 // AF_UNSPEC for IPv4 or IPv6 
    hints.ai_socktype = SOCK_STREAM;            // stream socket connection
    hints.ai_flags = AI_PASSIVE;                // use my IP

    // fill in addrinfo structures
    int return_val;
    if ((return_val = getaddrinfo(NULL, port_number, &hints, &results)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(return_val));
        exit(EXIT_FAILURE);
    }

    // bind the socket fd to port
    bind_sock(&server_fd, results);

    // Start listening
    if (listen(server_fd, BACKLOG) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    // main accept() loop
    while (RUNNING) {
        addrlen = sizeof(client_addr);
        client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &addrlen);
        if (client_fd < 0) {
            perror("Accept failed");
            continue;
        }

        // log
        printf("Accepted\n");
        fflush(stdout);
        
        // request received
        handle_client(client_fd, cache);
    }

}


/**
 * reads the clients request and identifies origin server,
 * check & returns if the request/response has been cached
 */
void handle_client(int client_fd, CacheEntry** cache) {
    char buffer[BUFFER_SIZE];
    int client_bytes_read = recv_client_request(client_fd, buffer, BUFFER_SIZE);
    if (client_bytes_read == FAIL) {
        fprintf(stderr, "client request was not read\n");
        close(client_fd);
        return;
    }

    // parse the clients request, store in request struct
    struct http_header request;
    parse_request(buffer, client_bytes_read, &request);

    // log tail
    printf("Request tail %s\n", request.tail);
    fflush(stdout);

    // increment every entry counter
    incrementCounter(cache);

    // check if the request/response is in the cache
    int hitIndex = findCacheHit(cache, &request);
    int entry_status = CACHE_ENTRY_FRESH;       // init as fresh
    int cache_hit = (hitIndex != CACHE_MISS);   // true is cache hit, false if cache miss

    // if cache hit, check if its stale
    if (cache_hit) {
        entry_status = checkStaleCache(cache, hitIndex);
    }

    // if cache entry stale, log
    if (entry_status == CACHE_ENTRY_STALE) {
        printf("Stale entry for %s %s\n", request.host, request.uri);
        fflush(stdout);
    }
    
    // if cache hit and the cache entry is fresh, send cached response
    if (cache_hit && (entry_status == CACHE_ENTRY_FRESH)) {

        // cache hit: return cached request/response
        char* response = cache[hitIndex]->response;
        int response_len = cache[hitIndex]->response_len;

        // reset the last accessed counter
        cache[hitIndex]->last_accessed = 0;

        // send response back to the client 
        if (send_server(client_fd, response, response_len) == FAIL) {
            fprintf(stderr, "error in sending response back to client\n");
        }

        char* host = request.host;
        char* request_uri = request.uri; 

        // log according to the output for stage 2
        printf("Serving %s %s from cache\n", host, request_uri);
        fflush(stdout);

        close(client_fd);

    }

    // in a cache miss or stale cache entry, connect to the origin server
    // and retrieve a response.
    else {

        // connect to origin server
        int origin_fd = connect_origin(request, HTTP_PORT);

        if (origin_fd == FAIL) {
            close(client_fd);
            close(origin_fd);
            return;
        }

        // send the request to origin server
        if (request_origin(origin_fd, request) == FAIL) {
            close(client_fd);
            close(origin_fd);
            return;
        }

        // evict from the cache if it is full
        int insert_index = validateCacheEntryRemoval(cache);
        if (insert_index == NO_INDEX_FOUND && !cache_hit) {
            // if cache is full && cache miss, evict the LRU entry
            evictFromCache(cache);
        }

        // log once request is sent...
        printf("%sting %s %s\n", request.method, request.host, request.uri);
        fflush(stdout);

        char response[RESPONSE_SIZE];   
        int response_len = recv_origin_response(origin_fd, response, RESPONSE_SIZE);

        // receive bytes from origin server
        if (response_len == FAIL) {
            fprintf(stderr, "Error: receving bytes from origin failed\n");
            close(client_fd);
            close(origin_fd);
            return;
        }

        // log content length
        int content_len = content_length(response);
        if (content_len != FAIL) {
            printf("Response body length %d\n", content_len);
            fflush(stdout);
        }

        // send the response to client
        if (send_server(client_fd, response, response_len) == FAIL) {
            fprintf(stderr, "error in sending response back to client\n");
        }
        
        // cache request/response
        // EXTRA NOTE: Thank you Jarin for your wonderful note on using response_len :)
        int request_len = strlen(buffer);
        int invalid_response = isValidCacheEntry(response);
        int within_size_limits = (request_len < REQUEST_MAX_SIZE) && (response_len < ENTRY_MAX_SIZE);

        // validation & print handling for cache misses or a new response for stale cache entries
        if (cache_hit) {
            if (!invalid_response && within_size_limits) {
                cacheAtIndex(cache, hitIndex, &request, response, response_len);
            }
            else if (invalid_response) {
                printf("Not caching %s %s\n", request.host, request.uri);
                fflush(stdout);
                evictIndexFromCache(cache, hitIndex);
            }
            else if (!within_size_limits) {
                evictIndexFromCache(cache, hitIndex);
            }
        }

        // entry is not in the cache
        else {
            if (!invalid_response && within_size_limits) {
                // cache the new response
                cacheRequestAndResponse(cache, &request, response, response_len);
            }
            else if (invalid_response) {
                printf("Not caching %s %s\n", request.host, request.uri);
                fflush(stdout);
            }
        }
        
        close(origin_fd);
        close(client_fd);

    }
}

/**
 * Establishes the socket connection to the origin server for a request, 
 * returns the id of the socket
 */
int connect_origin(struct http_header client_request, char *port) {
    int origin_fd;
    struct addrinfo hints, *results;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    int return_val;
    if ((return_val = getaddrinfo(client_request.host, port, &hints, &results)) != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(return_val));
        return FAIL;
    }

    // make a socket connection, if failed, discard this request
    if (connect_sock(&origin_fd, results) == FAIL) {
        printf("proxy connection with: %s failed\n", client_request.host);
        return FAIL;
    }

    return origin_fd;
}

/**
 * Sends a HTTP request to the origin server over a socket
 */
int request_origin(int origin_fd, struct http_header client_request) {
    // build the request to be sent to origin server
    char proxy_request[HTTP_REQUEST_LEN];
    build_request(proxy_request, client_request);

    if (send_server(origin_fd, proxy_request, strlen(proxy_request)) == FAIL) {
        fprintf(stderr, "Error in forwarding request to origin server\n");
        return FAIL;
    }


    return SUCCESS;
}

/**
 * Proxy functionality to receive the request from the client, and
 * stores it in the buffer. Returns the total bytes read.
 */
int recv_client_request(int fd, char *buffer, size_t buffer_size) {
    int total_bytes = 0;

    // loop until entire HTTP message has been read into buffer
    while (RUNNING) {
        int bytes_to_read = buffer_size - total_bytes - 1;
        
        // receive bytes from server, buffer+total_bytes is pointer arithmetic
        int bytes_read = recv(fd, buffer+total_bytes, bytes_to_read, 0);

        // request receive failed, close connection
        if (bytes_read < 0) {
            perror("recv");
            return FAIL;
        }

        total_bytes += bytes_read;
        buffer[total_bytes] = '\0';

        // check if "end of HTTP header" sub-string exists in buffer
        if (strstr(buffer, HTTP_HEADER_END) != NULL) {
            break;
        }

        if (total_bytes >= buffer_size-1) {
            fprintf(stderr, "Incompatible HTTP request: request too large\n");
            return FAIL;
        }

    }

    return total_bytes;
}


/**
 * Proxy functionality to receive the response from the origin server, and
 * stores it in the buffer. Returns the total bytes read.
 */
int recv_origin_response(int origin_fd, char *response, size_t response_size) {
    int bytes_read, total_bytes_read = 0;
    char buffer[response_size];

    // loop until entire origin response has been read into response
    while ((bytes_read = recv(origin_fd, buffer, response_size, 0)) > 0) {
        if (total_bytes_read + bytes_read >= response_size) {
            fprintf(stderr, "response error: response too large\n");
            return FAIL;
        }

        memcpy(response + total_bytes_read, buffer, bytes_read);
        total_bytes_read += bytes_read;
    }

    // receive failed, close connection
    if (bytes_read < 0) {
        fprintf(stderr, "recv error\n");
        return FAIL;
    }

    return total_bytes_read;
}


/**
 * Proxy functionality to send the origin server response to the client.
 * Returns the status of the program.
 */
int send_server(int fd, char *message, int message_len) {
    int total_sent = 0;

    while (total_sent < message_len) {
        int bytes_sent = send(fd, message + total_sent, message_len-total_sent, 0);

        if (bytes_sent < 0) {
            perror("send");
            return FAIL;
        }

        total_sent += bytes_sent;

        if (bytes_sent == 0) {
            break;
        }
    }

    if (total_sent < message_len) {
        fprintf(stderr, "not all bytes have been sent to server\n");
        return FAIL;
    }

    return SUCCESS;
}

