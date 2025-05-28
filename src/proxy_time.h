#ifndef _PROXY_TIME_H_
#define _PROXY_TIME_H_

#include <time.h>

extern time_t proxy_start_time;     // global variable to store the proxy start time

// initialise the start time of the process
void start_timer();

// returns seconds since process start
int get_time_elapsed();

#endif