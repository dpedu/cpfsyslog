#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ctype.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <limits.h>
#include "helpers.h"
// #include "pfparser.h"
#include "sysparser.h"
#include <signal.h>

/*UDP server-related mostly lifted from https://cs.nyu.edu/~mwalfish/classes/16sp/classnotes/handout01.pdf*/


void panic(const char* s) {
    perror(s);
    exit(1);
}


int running = 1;
int sock_fd;


void sig_handler(int signum) {
    printf("\nExiting on signal %s\n", strsignal(signum));
    running = 0;  /* shut down the loop */
    shutdown(sock_fd, SHUT_RDWR);  /* break the listener socket */
    close(sock_fd);
}


int main(int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    signal(SIGTERM, sig_handler);
    signal(SIGINT, sig_handler);

    /*Parse port number to integer*/
    char* portend;
    unsigned int portl;
    portl = strtol(argv[1], &portend, 10);
    if (portend == NULL) panic("strtol");
    assert(portl < USHRT_MAX);
    unsigned short port = (unsigned short)portl;

    /*Create socket*/
    char msg[4096];
    if ((sock_fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        panic("socket");

    /*Set socket options*/
    int one = 1;
    setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    /*Bind socket*/
    struct sockaddr_in my_addr, my_peer_addr;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(port); /*host to network short - converts a *s*hort from the *h*ost's to *n*etwork's endianness*/
    if (bind(sock_fd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in)) < 0)
        panic("bind failed");

    socklen_t addrlen = sizeof(struct sockaddr_in);
    while (running) {
        int size_recvd;
        if ((size_recvd = recvfrom(sock_fd, /* socket */
                                   msg, /* buffer */
                                   sizeof(msg), /* size of buffer */
                                   0, /* flags = 0 */
                                   (struct sockaddr*)&my_peer_addr, /* who’s sending */
                                   &addrlen /* length of buffer to receive peer info */
                                   )) < 0) {
            if (running) panic("recvfrom");
            else break;
        }
        assert(size_recvd < sizeof(msg));  /*messages can't be longer than our buffer*/

        assert(addrlen == sizeof(struct sockaddr_in));
        printf("\nGot message: %s\n", msg);

        /*TODO should we check that msg[size_recvd] == \0 ?
        printf("From host %s src port %d got message %.*s\n",
               inet_ntoa(my_peer_addr.sin_addr), ntohs(my_peer_addr.sin_port), size_recvd, msg);*/
        struct SysMessage result;
        memset(&result, 0, sizeof(result)); /* Doing this or setting result above to `= {};` seems to make valgrind happy */
        /*printf("\nsize: %lu\n\n", sizeof(result)); // curious how big the struct gets
        // printf("msg[size_recvd] is: %d", msg[size_recvd]);*/
        msg[size_recvd] = '\0'; /*We receive 1 full string at a time*/

        if(sysmsg_parse(&result, msg) != 0) {
            printf("Failed to parse message: %s", msg);
        } else {
            printf("syslogmessage is valid:\n\tpriority: %d\n\tapplication: %s\n\tDate: %s %d %02d:%02d:%02d\n\t\n",
                   result.priority, result.application, result.date.month, result.date.day, result.date.hour,
                   result.date.minute, result.date.second);

            pf_data fwdata;
            memset(&fwdata, 0, sizeof(fwdata));

            if(pfparse_message(msg, &fwdata) != 0) {
                printf("Failed to parse pfsense data: %s", msg);
            } else {
                printf("IP Data:\n\tInterface: %s\n\tIP version: %d\n",
                       fwdata.iface, fwdata.ipversion);
            }

        }
    }

    exit(EXIT_SUCCESS);
}
