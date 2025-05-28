// COMP30023 Project 2: Filip Madyarov & Jarin Lewis 2025
// Cache Controller for Stage 2, 3 and 4

#include "cache.h"
#include <stdio.h> 
#include <stdlib.h>  
#include <string.h>  

#define _XOPEN_SOURCE

/******************** PUBLIC FUNCTIONS ********************/
/**
 * Inserts an entry and response into the cache at 
 * the first entry that is available
 */ 
void cacheRequestAndResponse(
    CacheEntry **cache, 
    struct http_header *request, 
    const char *response_buf, 
    int response_len)
{
    if (!cache || !request || !response_buf || response_len <= 0) return;

    int insert_index = -1;

    // Find the first invalid slot
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (!cache[i]->valid) {
            insert_index = i;
            break;
        }
    }

    cacheAtIndex(cache, insert_index, request, response_buf, response_len);
}

/**
 * Caches a request and response
 * at a certain index
 */ 
void cacheAtIndex(
    CacheEntry **cache, int insert_index,
    struct http_header *request,
    const char *response_buf,
    int response_len)
{
    // Clear any existing memory in the target slot
    if (cache[insert_index]->request) {
        free(cache[insert_index]->request);
    }
    if (cache[insert_index]->response) {
        free(cache[insert_index]->response);
    }

    // Copy request
    cache[insert_index]->request = malloc(sizeof(struct http_header));
    if (!cache[insert_index]->request) {
        fprintf(stderr, "malloc failed for request copy\n");
        return;
    }
    memcpy(cache[insert_index]->request, request, sizeof(struct http_header));

    // Copy response
    cache[insert_index]->response = malloc(response_len);
    if (!cache[insert_index]->response) {
        fprintf(stderr, "malloc failed for response copy\n");
        free(cache[insert_index]->request);
        return;
    }
    memcpy(cache[insert_index]->response, response_buf, response_len);
    cache[insert_index]->response_len = response_len;

    // Set metadata
    cache[insert_index]->last_accessed = 0;
    cache[insert_index]->valid = 1;

    // store max age if applicable
    char cache_ctrl[CACHE_CONTROL_LEN];
    extract_header(response_buf, CACHE_CONTROL_HEADER, cache_ctrl, CACHE_CONTROL_LEN);

    uint32_t max_age = extract_max_age(cache_ctrl);
    cache[insert_index]->max_age = max_age;

    // store time cached
    cache[insert_index]->time_cached = get_time_elapsed();
}


/**
 * Create the cache. Returns a pointer to the start 
 * of the cache
 */ 
CacheEntry **initialiseCache() {
    CacheEntry **cache = malloc(CACHE_SIZE * sizeof(*cache));

    if (!cache) {
        perror("Failed to allocate cache");
        exit(EXIT_FAILURE);
    }

    for (int i = 0; i < CACHE_SIZE; i++) {
        cache[i] = malloc(sizeof(CacheEntry));
        assert(cache[i]);
        cache[i]->request = NULL;
        cache[i]->response = NULL;
        cache[i]->response_len = 0;
        cache[i]->valid = 0;
        cache[i]->last_accessed = -1;
        cache[i]->time_cached = -1;
        cache[i]->max_age = 0;
    }
    return cache;
}

// Find if an entry is inside the cache
int findCacheHit(CacheEntry** cache, struct http_header* incoming_request) {
    if (cache == NULL) {
        return CACHE_NOT_USED;
    }

    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache[i]->valid && cache[i]->request != NULL) {
            struct http_header* cached = cache[i]->request;

            if (strcmp(cached->method, incoming_request->method) == 0 &&
                strcmp(cached->host, incoming_request->host) == 0 &&
                strcmp(cached->uri, incoming_request->uri) == 0) {
                return i; // Cache hit
            }
        }
    }
    return CACHE_MISS; // Miss
}

/**
 * Checks if an entry to the cache has become stale
 * and returns its status
 */ 
int checkStaleCache(CacheEntry **cache, int index) {
    if (cache == NULL)
        return CACHE_NOT_USED;

    int time_cached = cache[index]->time_cached;
    uint32_t max_age = cache[index]->max_age;

    if (max_age == 0) {
        // implies expiration functionality not being used
        // return fresh to indicate to proxy to treat entry as fresh
        return CACHE_ENTRY_FRESH;
    }

    int entry_age = get_time_elapsed() - time_cached;

    if (entry_age > max_age) {
        return CACHE_ENTRY_STALE;
    }

    return CACHE_ENTRY_FRESH;
}

