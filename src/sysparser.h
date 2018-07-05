#include <stdio.h>
#include "pfparser.h"


#define DF_MONTH_LEN 9


const static char* month2nummap[] __attribute__ ((unused)) =
    {[1] = "Jan",
     [2] = "Feb",
     [3] = "Mar",
     [4] = "Apr",
     [5] = "May",
     [6] = "Jun",
     [7] = "Jul",
     [8] = "Aug",
     [9] = "Sep",
     [10] = "Oct",
     [11] = "Nov",
     [12] = "Dec"};




/*TODO numeric indicator for month?*/
struct Datefields {
    char month[DF_MONTH_LEN];
    int day;
    int hour;
    int minute;
    int second;
};

/*TODO check max app name length*/
#define MSG_APP_LEN 16

struct SysMessage {
    int priority;
    char application[MSG_APP_LEN];
    struct Datefields date;
};


int sysmsg_parse(struct SysMessage* result, char* message);

int month2num(char* month);
