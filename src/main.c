#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "config.h"
#include "server.h"
#include "helpers.h"


int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <config.json>\n", argv[0]);
        exit(1);
    }

    /*Parse port number to integer*/
    /*char* portend;
    unsigned int portl;
    portl = strtol(argv[1], &portend, 10);
    if (portend == NULL || portend == argv[1]) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }
    assert(portl < USHRT_MAX);
    unsigned short port = (unsigned short)portl;*/

    struct config* conf = config_load(argv[1]);

    printf("url: %s\n", conf->url);
    printf("port: %d\n", conf->port);

    run_server(conf->port, conf->url);

    config_free(conf);

    exit(EXIT_SUCCESS);
}
