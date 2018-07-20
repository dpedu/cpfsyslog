#include <stdio.h>
#include <stdlib.h>


void panic(const char* s) {
    perror(s);
    exit(1);
}

void die(const char* s) {
    fprintf(stderr, "%s\n", s);
    exit(EXIT_FAILURE);
}
