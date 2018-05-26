#include "pfparser.h"

#define DF_MONTH_LEN 9

/*TODO numeric indicator for month?*/
struct Datefields {
    char month[DF_MONTH_LEN];
    int day;
    int hour;
    int minute;
    int second;
};

/*TODO check max app name length*/
#define MSG_APP_LEN 64

struct SysMessage {
    int priority;
    char application[MSG_APP_LEN];
    struct Datefields date;
    // char message;
    // pf_message data;
};


int sysmsg_parse(struct SysMessage* result, char* message);
