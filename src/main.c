#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <limits.h>

// Mostly lifted from https://cs.nyu.edu/~mwalfish/classes/16sp/classnotes/handout01.pdf

void panic(const char* s) {
    perror(s);
    exit(1);
}

struct Message {
    int priority;
};

int parse_message(struct Message* result, char* message) {
    /*
        parse a message like:
            <134>May 10 03:09:59 filterlog: 5,,,1000000103,cpsw0,match,block,in,4,0x20,,239,27547,0,none,6,tcp,40,185.216.140.37,24.4.129.164,57159,11111,0,S,3919167832,,1024,,
        Format:
            <priority>VERSION ISOTIMESTAMP HOSTNAME APPLICATION PID MESSAGEID STRUCTURED-DATA MSG
        Assumes null termed string
    */
    printf("Got message: %s\n", message);

    // Must have >3 chars to form <x> priority
    if (strlen(message) < 3) return 1;
    // Must start with <
    if (message[0] != '<') return 1;

    printf("%lu\n", strlen(message));
    // const char delim = " "

    char digits[] = "\0\0\0";
    int num_digits = 0;
    int position = 1;
    //bool found_priority_end = false; // TODO

    while (position < 4) {
        printf("inspecting %c\n", message[position]);
        if(!isdigit(message[position])) return 1; // priority must be numeric
        digits[num_digits] = message[position];
        num_digits++;
        position++;
        if (message[position] == '>') {
            break;
        }
        // break;
    }
    if (num_digits == 0) return 1; // empty priority >< ?

    printf("found digits %d, aka %d\n", atoi(digits), atoi(digits)/2);
    // result = Message{atoi(digits)};
    result->priority = atoi(digits);

    // assert(0);
    // *strtok(char *str, const char *delim)
}

int main(int argc, char** argv) {
    int sock_fd;
    struct sockaddr_in my_addr, my_peer_addr;
    char* endptr;
    unsigned int portl;
    unsigned short port;
    int size_recvd;
    char msg[1024];
    socklen_t addrlen;

    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    /* convert from string to int */
    portl = strtol(argv[1], &endptr, 10);
    if (endptr == NULL || portl == 0)
    panic("strtol");

    assert(portl < USHRT_MAX);
    port = (unsigned short)portl;

    /*
    * Note! This code is not the same as what you need to do in lab1.
    */
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        panic("socket");
    int one = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(1));

    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(port);

    if (bind(sock_fd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in)) < 0)
        panic("bind failed");

    while (1) {
        addrlen = sizeof(struct sockaddr_in);

        if ((size_recvd = recvfrom(sock_fd, /* socket */
                                     msg, /* buffer */
                                     sizeof(msg), /* size of buffer */
                                     0, /* flags = 0 */
                                     (struct sockaddr*)&my_peer_addr, /* who’s sending */
                                     &addrlen /* length of buffer to receive peer info */
                                    )) < 0)
            panic("recvfrom");

        assert(addrlen == sizeof(struct sockaddr_in));

        // TODO should we check that msg[size_recvd] == \0 ?
        // printf("From host %s src port %d got message %.*s\n",
        //        inet_ntoa(my_peer_addr.sin_addr), ntohs(my_peer_addr.sin_port), size_recvd, msg);
        struct Message result;
        // printf("XXXsize: %lu", sizeof(result)); // curious how big the struct gets
        if(parse_message(&result, msg)) {
            printf("message is valid, priority %d\n", result.priority);
        }
    }

    exit(EXIT_SUCCESS);
}
