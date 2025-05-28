// COMP30023 Project 2: Filip Madyarov & Jarin Lewis 2025
// Cache Controller for Stage 2

#ifndef CACHE_H
#define CACHE_H

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

#include "http_request.h"
#include "proxy_time.h"

// Cache Size Constants
#define CACHE_SIZE 10
#define ENTRY_MAX_SIZE 102400  // 100 KB
#define REQUEST_MAX_SIZE 2000

#define CACHE_MISS -1
#define CACHE_NOT_USED -1
#define NO_INDEX_FOUND -2

#define CACHE_ENTRY_VALID 1
#define CACHE_ENTRY_INVALID 0

#define CACHE_ENTRY_FRESH 1
#define CACHE_ENTRY_STALE 0

typedef struct {
    struct http_header *request;    // Full HTTP request string (key)
    char *response;                 // Response data (value)
    int response_len;               // Size of response data
    int valid;                      // 1 if this entry is active
    int last_accessed;              // Keep track of how long entry has been in cache
    int time_cached;                // stores the time entry was cached/recached
    uint32_t max_age;               // max age entry can be in cache before expiration
} CacheEntry;

// Add to Cache
void cacheRequestAndResponse(
    CacheEntry **cache, 
    struct http_header *request, 
    const char *response_buf, 
    int response_len
);

// insert entry at specified index
void cacheAtIndex(
    CacheEntry **cache, int insert_index,
    struct http_header *request,
    const char *response_buf,
    int response_len
);

// Initialise Cache
CacheEntry **initialiseCache();

// Check the cache for a request and return the request/response if present
int findCacheHit(CacheEntry** cache, struct http_header* incoming_request);

// checks if cache entry is stale or fresh, returns STALE or FRESH
int checkStaleCache(CacheEntry** cache, int index);


#endif
