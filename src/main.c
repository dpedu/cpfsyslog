#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <assert.h>
#include <unistd.h>
#include <limits.h>
#include <signal.h>
#include "helpers.h"
#include "sysparser.h"

#include <json-c/json.h>

void panic(const char* s) {
    perror(s);
    exit(1);
}


/*defined here as they are used in conjunction with the shutdown signal handler*/
int running = 1;
int sock_fd;


void sig_handler(int signum) {
    printf("\nExiting on signal %s\n", strsignal(signum));
    running = 0;  /* shut down the main loop */
    shutdown(sock_fd, SHUT_RDWR);  /* break the listener socket */
    close(sock_fd);
}


/*UDP server bits mostly lifted from https://cs.nyu.edu/~mwalfish/classes/16sp/classnotes/handout01.pdf*/
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
    my_addr.sin_port = htons(port); /*host to network endianess for a short - converts a *s*hort from the *h*ost's to *n*etwork's endianness*/
    if (bind(sock_fd, (struct sockaddr*)&my_addr, sizeof(struct sockaddr_in)) < 0)
        panic("bind failed");

    socklen_t addrlen = sizeof(struct sockaddr_in);
    char msg[4096];
    while (running) {
        int size_recvd;
        if ((size_recvd = recvfrom(sock_fd,                         /* socket */
                                   msg,                             /* buffer */
                                   sizeof(msg),                     /* buffer length */
                                   0,                               /* no flags */
                                   (struct sockaddr*)&my_peer_addr, /* who’s sending */
                                   &addrlen                         /* length of buffer to receive peer info */
                                   )) < 0) {
            if (running) panic("recvfrom");
            else break; /*sock was closed by exit signal*/
        }
        assert(size_recvd < sizeof(msg));  /*messages can't be longer than our buffer. TODO if they are longer we should
        dump it and wait until the next loop. if the next buffer is some portion of a too-long message, we can expect
        the various parsing below to fail.*/

        assert(addrlen == sizeof(struct sockaddr_in));
        printf("\nGot message: %s\n", msg);

        /*TODO should we check that msg[size_recvd] == \0 ?
        printf("From host %s src port %d got message %.*s\n",
               inet_ntoa(my_peer_addr.sin_addr), ntohs(my_peer_addr.sin_port), size_recvd, msg);*/
        struct SysMessage result;
        memset(&result, 0, sizeof(result)); /* Doing this or setting result above to `= {};` makes valgrind happy */
        /*printf("\nsize: %lu\n\n", sizeof(result)); // curious how big the struct gets
        // printf("msg[size_recvd] is: %d", msg[size_recvd]);*/
        msg[size_recvd] = '\0'; /*We receive 1 full string at a time*/

        /*parse syslog message into fields*/
        if(sysmsg_parse(&result, msg) != 0) {
            printf("Failed to parse message: %s", msg);
        } else {
            printf("syslog message is valid:\n\tpriority: %d\n\tapplication: %s\n\tDate: %s %d %02d:%02d:%02d\n",
                   result.priority,
                   result.application,
                   result.date.month,
                   result.date.day,
                   result.date.hour,
                   result.date.minute,
                   result.date.second);

            /*parse MSG field into pfsense data*/
            pf_data fwdata = {0};
            //memset(&fwdata, 0, sizeof(fwdata));
            if(pfdata_parse(msg, &fwdata) != 0) {
                printf("Failed to parse pfsense data: %s\n\n", msg);
            } else {
                pfdata_print(&fwdata);

                json_object* jobj = json_object_new_object();
                json_object *jstring = json_object_new_string("bar");
                json_object_object_add(jobj,"foo", jstring);
                printf("The json object created: %s\n",json_object_to_json_string(jobj));
                json_object_put(jobj);

            }
        }
    }

    exit(EXIT_SUCCESS);
}
