#include "sysparser.h"
#include <stdio.h>
#include "helpers.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>


int parse_priority(char* message, int* priority, int* position) {
    /*
        Given a string that begins with a message something like: <123>foo
        Parse out the number (123) and place it in the passed `priority` int pointer
        The position after the final `>` will be placed in the passed `position` int pointer
        Returns 0 on success or something else on failure
    */
    /*Must have >3 chars to form <x> priority*/
    if (strlen(message) < 3) return 1;
    /*Must start with <*/
    if (message[0] != '<') return 1;
    char digits[4];
    memset(&digits, '\0', sizeof(digits));
    int num_digits = 0;
    int pos = 1;
    int found_end = 0;
    /*bool found_priority_end = false; // TODO*/
    while (pos < 4) {
        if(!isdigit(message[pos])) return 1; /*priority must be numeric*/
        digits[num_digits] = message[pos];
        num_digits++;
        pos++;
        if (message[pos] == '>') {
            found_end = 1;
            break;
        }
    }
    if (found_end == 0) return 1;
    if (num_digits == 0) return 1; /*empty priority <> ?*/
    *priority = atoi(digits);
    *position = pos;
    return 0;
}


int parse_datefield(char* message, struct Datefields* date, int* position) {
    /*
        Given a message+position pointers, where message + position in a string like:
            May 10 03:09:59 filterlog: 5,,,....
        Parse out the date and place the fields in the passed datefields struct pointer
        Position will be advanced to the character after the parsed data
    */
    /*char month[8];
    memset(&month, '\0', sizeof(month));  makes valgrind happy as the above char contains uninitialized memory*/
    int date_length;
    if(sscanf(message + *position, "%"STR(DF_MONTH_LEN)"s %d %d:%d:%d%n",
              date->month, &(date->day), &(date->hour), &(date->minute), &(date->second), &date_length) != 5) {
        return 1;  /*Failed to parse all desired fields*/
    }
    *position += date_length;
    return 0;
}


int parse_application(char* message, char* application, int* position) {
    int app_length;
    if(sscanf(message + *position, "%"STR(MSG_APP_LEN)"s%n", application, &app_length) != 1) {  /*%n not counted in returned field count*/
        return 1;  /*Failed to parse all desired fields*/
    }
    if(app_length - 1 > MSG_APP_LEN) return 1;
    if(strlen(application) < 2) return 1;  /*Expect at least chars*/
    application[app_length-1] = '\0';  /*Remove the trailing :*/
    *position += app_length;
    return 0;
}


int sysmsg_parse(struct SysMessage* result, char* message) {
    /*
        parse a message like:
            <134>May 10 03:09:59 filterlog: 5,,,1000000103,cpsw0,match,block,in,4,0x20,,239,27547,0,none,6,tcp,40,185.216.140.37,24.4.129.164,57159,11111,0,S,3919167832,,1024,,
        Format:
            <priority>VERSION ISOTIMESTAMP HOSTNAME APPLICATION PID MESSAGEID STRUCTURED-DATA MSG
        Assumes null termed string
        Param message will be transformed to the MSG field
    */
    int priority = 0;
    int position = 0;
    if(parse_priority(message, &priority, &position) != 0) return 1;
    result->priority = priority;
    position++; /*Now sits on the first character of the ISOTIMESTAMP*/

    /*Parse ISOTIMESTAMP
    Note: does not parse a full iso timestamp, only the format above*/
    struct Datefields date;
    if(parse_datefield(message, &date, &position) != 0) {
        return 1;
    }
    result->date = date;
    if(message[position] != ' ') return 1; // Something other than a space after the date
    position++;  /*position now at beginning of HOSTNAME field*/

    /*Parse APPLICATION
    filterlog: 5,,,1000000103,cpsw0,match....*/
    char application[MSG_APP_LEN];
    if(parse_application(message, application, &position) != 0) return 1;
    memcpy(result->application, application, sizeof(application));
    if(message[position] != ' ') return 1; // Something other than a space after the app name
    position += 1; /*pass over the space*/

    /*printf("remaining: '%s'\n", message + position);*/

    /*trim original message to only the CSV portion*/
    int msglen = strlen(message);
    int datalen = msglen - position;
    memmove(message, &message[position], datalen);
    /*zero the rest of the message*/
    memset(&message[datalen], 0, msglen - datalen);

    /*pf_message result_msg;*/
    // if(pfparse_message(message, &(result->data)) != 0) return 1;

    /* put message in result */
    // memcpy(&(result->message), message, strlen(message));
    // result->message = message + position;


    /*char msg_remaining[4096];
    memset(&msg_remaining, '\0', sizeof(msg_remaining));
    memcpy(msg_remaining, &message[position], strlen(message) - position);
    printf("'%s'\n", msg_remaining);
    or
    memmove(message, &message[position], strlen(message) - position);
    printf("'%s'\n", message);*/

    return 0;
}


int month2num(char* month) {
    for(int i=1; i<=12; i++) {
        if(strcmp(month, month2nummap[i]) == 0) {
            return i;
        }
    }
    return -1;
}


#ifdef AFL

int main() {
    char *buffer;
    size_t bufsize = 4096;
    size_t characters;

    buffer = (char *)malloc(bufsize * sizeof(char));
    if (buffer == NULL)
        die("Unable to allocate buffer");

    characters = getline(&buffer,&bufsize,stdin);

    struct SysMessage data = {0};
    int result = sysmsg_parse(&data, buffer);
    printf("sysparser result: %d\n", result);

    if(result == 0) {
        pf_data fwdata = {0};
        result = pfdata_parse(buffer, &fwdata);
        printf("pfparser result: %d\n", result);
    }

    return result;
}

#endif

