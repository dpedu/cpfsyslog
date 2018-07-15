#include <stdio.h>
#include <stdlib.h>


void panic(const char* s) {
    perror(s);
    exit(1);
}
