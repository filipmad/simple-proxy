//COMP30023 Project 2: Filip Madyarov & Jarin Lewis 2025
// Main File

#include "connection.h"
#include "proxy_time.h"

#include <stdio.h> 
#include <stdlib.h>  
#include <string.h>  
#include <pthread.h>


/* COMMAND LINE ARGUMENT MACROS */
#define NARGS 3
#define NARGS_CACHE 4
#define PORTNUM_ARG "-p"
#define CACHEUSE_ARG "-c"

/* CONSTANTS */
#define NO_CACHE 0
#define CACHE 1


int main(int argc, char *argv[]) {

    // validate argument formatting
    if (argc != NARGS && argc != NARGS_CACHE) {
        fprintf(stderr, "Usage: htproxy -p listen-port [-c] (optional)\n");
        exit(EXIT_FAILURE);
    }

    // read command line arguments
    char *port_number = NULL;
    int use_cache = NO_CACHE;

    for (int i=1; i < argc; i++) {
        if (!strcmp(argv[i], PORTNUM_ARG)) {
            port_number = argv[i+1];
        }
        else if (!strcmp(argv[i], CACHEUSE_ARG)) {
            use_cache = CACHE;
        }
    }

    if (port_number == NULL) {
        fprintf(stderr, "Error: port number wasn't read\n");
        exit(EXIT_FAILURE);
    }

    // start proxy timer
    start_timer();
    
    // begin the proxy
    startProxy(port_number, use_cache);

    return 0;
}
