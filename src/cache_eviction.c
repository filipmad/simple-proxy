// COMP30023 Project 2: Filip Madyarov & Jarin Lewis 2025
// Cache Eviction Controller for Stage 2, 3, and 4

// Modules
#define _XOPEN_SOURCE
#include <time.h>
#include "cache.h"
#include <stdio.h> 
#include <stdlib.h>  
#include <string.h>
#include <assert.h>

#include "http_request.h"
#include "cache_eviction.h"


/******************** PUBLIC FUNCTIONS ********************/
/**
 * Evict from the cache according to the
 * least recently used entry
 */
void evictFromCache(CacheEntry **cache) {
    if (cache == NULL) {
        return;
    }

    int oldest_index = 0;
    int oldest_count = cache[0]->last_accessed;

    for (int i = 1; i < CACHE_SIZE; i++) {
        assert(cache[i]->last_accessed != oldest_count);

        if (cache[i]->last_accessed > oldest_count) {
            oldest_count = cache[i]->last_accessed;
            oldest_index = i;
        }
    }

    // free dynamically allocated fields
   evictIndexFromCache(cache, oldest_index);
}

/**
 * Evict a particular index from the cache
 */
void evictIndexFromCache(CacheEntry **cache, int index) {
    // stage 2 output
    printf("Evicting %s %s from cache\n", 
        cache[index]->request->host,
        cache[index]->request->uri 
    );
    fflush(stdout);

    // free dynamically allocated fields
    free(cache[index]->response);
    free(cache[index]->request);

    // clear entry
    cache[index]->valid = CACHE_ENTRY_INVALID;
    cache[index]->response = NULL;
    cache[index]->request = NULL;
    cache[index]->last_accessed = -1;
    cache[index]->response_len = 0;

    // assert that the entry is clear
    assert(cache[index]->valid == CACHE_ENTRY_INVALID);
    assert(cache[index]->response == NULL);
    assert(cache[index]->request == NULL);
    assert(cache[index]->last_accessed == -1);
    assert(cache[index]->response_len == 0);
}

/**
 * Validate that the cache needs an eviction
 * by determing if the cache is full or not
 */
int validateCacheEntryRemoval(CacheEntry **cache) {
    if (cache == NULL) {
        return CACHE_NOT_USED;
    }

    // Assume the cache is full
    int insert_index = NO_INDEX_FOUND;
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache[i]->valid == 0) {
            // Cache is not full
            insert_index = i;
            break;
        }
    }

    return insert_index;
}

/**
 * Increment the age for each cache entry
 */
void incrementCounter(CacheEntry **cache) {
    if (cache == NULL) {
        return;
    }

    // increment each cache entry's counter
    for (int i = 0; i < CACHE_SIZE; i++) {
        if (cache[i]->valid == CACHE_ENTRY_VALID) {
            cache[i]->last_accessed++;
        }
    }
}

/**
 * Validate whether the Cache-Control header of a response should restrict the
 * caching of a request and response
 */
int isValidCacheEntry(char* response) {

    // illegal words
    const char* restricted[] = {
        "private",
        "no-store",
        "no-cache",
        "max-age=0",
        "must-revalidate",
        "proxy-revalidate"
    };

    // extract the header from the response
    char cache_control_header[1024] = {0};
    extract_header(response, "Cache-Control:", cache_control_header, sizeof(cache_control_header));

    if (strlen(cache_control_header) == 0) {
        return DO_CACHE;
    }

    // normalize case
    to_lowercase(cache_control_header);

    // ensure there are no illegal words inside the Cache Control header
    for (int i = 0; i < sizeof(restricted) / sizeof(restricted[0]); i++) {
        if (strstr(cache_control_header, restricted[i]) != NULL) {
            return DO_NOT_CACHE;
        }
    }

    return DO_CACHE;
}
