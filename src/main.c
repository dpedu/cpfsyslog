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
    char digits[] = "\0\0\0";
    int num_digits = 0;
    int position = 1;
    //bool found_priority_end = false; // TODO
    while (position < 4) {
        if(!isdigit(message[position])) return 1; // priority must be numeric
        digits[num_digits] = message[position];
        num_digits++;
        position++;
        if (message[position] == '>') {
            position++; // Now sits on the first character of the ISOTIMESTAMP
            break;
        }
        // break;
    }
    if (num_digits == 0) return 1; // empty priority <> ?
    result->priority = atoi(digits);


    // Parse date
    char month[8];
    memset(&month, '\0', sizeof(month));  // makes valgrind happy as the above char contains uninitialized memory
    int day=0, hour=0, minute=0, second=0;
    {
        int msg_len = strlen(message);
        char message_fromdate[4096];
        memset(&message_fromdate, '\0', sizeof(message_fromdate));
        memcpy(message_fromdate, &message[position], msg_len - position);
        if(sscanf(message_fromdate, "%s %d %d:%d:%d", month, &day, &hour, &minute, &second) != 5) {
            return 1;
        }
    } // frees message_fromdate, but is this pointless?

    // Advance position by finding the length of the date
    char datestr[24];
    int date_length = sprintf(datestr, "%s %d %02d:%02d:%02d", month, day, hour, minute, second);
    assert(date_length > 0);
    position += date_length + 1;  // position now at beginning of HOSTNAME field

    char msg_remaining[4096];
    memset(&msg_remaining, '\0', sizeof(msg_remaining));
    memcpy(msg_remaining, &message[position], strlen(message) - position);
    printf("'%s'\n", msg_remaining);

    return 0;
}



int main(int argc, char** argv) {
    int sock_fd;
    struct sockaddr_in my_addr, my_peer_addr;
    char* endptr;
    unsigned int portl;
    unsigned short port;
    int size_recvd;
    char msg[4096];
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
        assert(size_recvd < sizeof(msg));  // messages can't be longer than our buffer

        assert(addrlen == sizeof(struct sockaddr_in));

        // TODO should we check that msg[size_recvd] == \0 ?
        // printf("From host %s src port %d got message %.*s\n",
        //        inet_ntoa(my_peer_addr.sin_addr), ntohs(my_peer_addr.sin_port), size_recvd, msg);
        struct Message result;
        // printf("XXXsize: %lu", sizeof(result)); // curious how big the struct gets
        //printf("msg[size_recvd] is: %d", msg[size_recvd]);
        msg[size_recvd] = '\0'; // We receive 1 full string at a time
        if(parse_message(&result, msg) != 1) {
            printf("message is valid, priority %d\n", result.priority);
        }
    }

    exit(EXIT_SUCCESS);
}
