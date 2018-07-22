#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>
#include "server.h"
#include "helpers.h"
#include "vendor/argparse.h"


int main(int argc, const char** argv) {
    int port = -1;
    char* url = NULL;

    struct argparse_option options[] = {
        OPT_HELP(),
        OPT_INTEGER('p', "port", &port, "server port",       .flags = OPT_REQUIRED),
        OPT_STRING ('u', "url",  &url,  "elasticsearch url", .flags = OPT_REQUIRED),
        OPT_END()
    };

    struct argparse argparse;
    argparse_init(&argparse, options, NULL, 0);
    argparse_parse(&argparse, argc, argv);

    run_server(port, url);

    exit(EXIT_SUCCESS);
}
