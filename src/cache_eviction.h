#ifndef CACHE_EVICTION_H
#define CACHE_EVICTION_H

#include "cache.h"
#include <time.h>

#define DO_NOT_CACHE 1 
#define DO_CACHE 0

// Evicts the least recently used cache entry based on last_modified
void evictFromCache(CacheEntry **cache);

// valiadates if cache is being used && full
int validateCacheEntryRemoval(CacheEntry **cache);

// increments the last_accessed counter in each entry
void incrementCounter(CacheEntry **cache);

// evict cache entry stored in index
void evictIndexFromCache(CacheEntry **cache, int index);

// validate whether the Cache-Control header of a response should restrict caching it 
int isValidCacheEntry(char* response);



#endif // CACHE_EVICTION_H
