#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <assert.h>

#include <json-c/json.h>

#include "helpers.h"
#include "sysparser.h"
#include "msgbuffer.h"
#include "geo.h"
#include "elasticsearch.h"


#define BUFF_MAX 5

/*defined here as they are used in conjunction with the shutdown signal handler*/
int running = 1;
int sock_fd;


void sig_handler(int signum) {
    printf("\nExiting on signal %s\n", strsignal(signum));
    running = 0;  /* shut down the main loop */
    shutdown(sock_fd, SHUT_RDWR);  /* break the listener socket */
    close(sock_fd);
}


time_t cur_t = {0};
struct tm cur_time = {0};


int handle_message(char* msg) {
    /*TODO should we check that msg[size_recvd] == \0 ?
    printf("From host %s src port %d got message %.*s\n",
           inet_ntoa(my_peer_addr.sin_addr), ntohs(my_peer_addr.sin_port), size_recvd, msg);*/
    struct SysMessage result;
    memset(&result, 0, sizeof(result)); /* Doing this or setting result above to `= {};` makes valgrind happy */
    /*printf("\nsize: %lu\n\n", sizeof(result)); // curious how big the struct gets
    // printf("msg[size_recvd] is: %d", msg[size_recvd]);*/

    /*parse syslog message into fields*/
    if(sysmsg_parse(&result, msg) != 0) {
        printf("Failed to parse message: %s", msg);
    } else {
        /*printf("syslog message is valid:\n\tpriority: %d\n\tapplication: %s\n\tDate: %s %d %02d:%02d:%02d\n",
               result.priority,
               result.application,
               result.date.month,
               result.date.day,
               result.date.hour,
               result.date.minute,
               result.date.second);*/

        /*parse MSG field into pfsense data*/
        pf_data fwdata = {0};
        //memset(&fwdata, 0, sizeof(fwdata));
        if(pfdata_parse(msg, &fwdata) != 0) {
            printf("Failed to parse pfsense data: %s\n\n", msg);
        } else {
            // pfdata_print(&fwdata);

            cur_t = time(NULL);
            cur_time = *localtime(&cur_t);

            char date_formtted[32];
            sprintf(date_formtted, "%04d-%02d-%02dT%02d:%02d:%02dZ",
                cur_time.tm_year + 1900,
                month2num(result.date.month),
                result.date.day,
                result.date.hour,
                result.date.minute,
                result.date.second);

            char time_now[sizeof "2018-07-15T13:49:05Z"];
            strftime(time_now, sizeof time_now, "%FT%TZ", gmtime(&cur_t));

            json_object* jobj = json_object_new_object();
            add_strfield(jobj, "date", time_now);
            add_strfield(jobj, "log_date", date_formtted);
            add_strfield(jobj, "app", result.application);
            pfdata_to_json(&fwdata, jobj);
            const char* json_msg = json_object_to_json_string(jobj);
            // printf("%s\n", json_msg);
            buff_push(strdup(json_msg));  // Copy message to heap and push to buffer
            json_object_put(jobj);
        }
    }
    return 0;
}


void clear_buffer() {
    char header[72];
    sprintf(header, "{\"index\": {\"_index\": \"firewall-%04d.%02d.%02d\", \"_type\": \"event\"}}\n",
            cur_time.tm_year + 1900,
            cur_time.tm_mon + 1,
            cur_time.tm_mday);

    // Calculate how large the payload will be
    int header_size = strlen(header);
    int num_messages = buff_count();
    char* messages[num_messages];
    int message_size = 0;
    for(int i=0; i<num_messages; i++) {
        messages[i] = buff_pop();
        message_size += strlen(messages[i]) + header_size + 1; // 1 newline
    }

    // Allocate and build the message
    char* message = calloc(1, message_size + 1);
    for(int i=0; i<num_messages; i++) {
        strcat(message, header);
        strcat(message, messages[i]);
        strcat(message, "\n");
        free(messages[i]);
    }

    // Send it
    if(put_events(message, "http://192.168.1.120:8298") == 0) {
        printf("Pushed %d messages\n", num_messages);
    } else {
        printf("Failed to post messages!\n");
    }
    free(message);
}


/*UDP server bits mostly lifted from https://cs.nyu.edu/~mwalfish/classes/16sp/classnotes/handout01.pdf*/
int run_server(int port) {
    geo_init();
    signal(SIGTERM, sig_handler);
    signal(SIGINT, sig_handler);

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
        msg[size_recvd] = '\0'; /*We receive 1 full string at a time*/
        /*printf("\nGot message: %s\n", msg);*/

        handle_message(msg);

        printf(".");
        fflush(stdout);
        if(buff_count() >= BUFF_MAX) {
            printf("\n");
            clear_buffer();
        }
    }

    buff_freeall();
    geo_close();
    return 1;
}
