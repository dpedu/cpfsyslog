#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "server.h"


int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    /*Parse port number to integer*/
    char* portend;
    unsigned int portl;
    portl = strtol(argv[1], &portend, 10);
    if (portend == NULL) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    assert(portl < USHRT_MAX);
    unsigned short port = (unsigned short)portl;

    run_server(port);
    exit(EXIT_SUCCESS);
}
