// COMP30023 Project 2: Filip Madyarov & Jarin Lewis 2025
// Timer functionality for Stage 4

// Modules
#include "proxy_time.h"

time_t proxy_start_time;

/******************** PUBLIC FUNCTIONS ********************/
/**
 * Start the timer for stage 4
 */
void start_timer() {
    proxy_start_time = time(NULL);
}

/**
 * Retrieve the time elapsed since start_timer() was called
 */
int get_time_elapsed() {
    return (int) difftime(time(NULL), proxy_start_time);
}