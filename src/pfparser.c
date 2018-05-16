#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pfparser.h"


int pfparse_message(char* message, pf_message* result) {
    printf("pfparse: '%s'\n", message);

    char* token;
    int field = 0;

    // Parse the first X fields
    // They are: <rule-number>,<sub-rule-number>,<anchor>,<tracker>,<real-interface>,<reason>,<action>,<direction>,<ip-version>
    // We only collect rule-number, real-interface, reason, action, direction, ip-version
    while ( (token = strsep(&message, ",")) != NULL) {
        printf("%02d: %s\n", field, token);
        switch (field) {
            case 0: // Rule number
                {
                    char* rnend;
                    long int rulenum = strtol(token, &rnend, 10);
                    if(rnend == NULL) return 1;
                    result->rulenum = (int)rulenum;
                }
                // if(result->rulenum == NULL) return 1;
            break;

            case 4: // iface
                if(strlen(token) > IFACE_LEN) return 1;
                memcpy(result->iface, token, strlen(token));
            break;

            case 5: // reason
                result->reason = strcmp(token, "match") ? pf_hit_other : pf_hit_match;
            break;

            case 6: // action
                result->action = strcmp(token, "block") ? pf_hit_block : pf_hit_pass;
            break;

            case 7: // direction
                result->direction = strcmp(token, "in") ? pf_dir_in : pf_dir_out;
            break;

            case 8: // ip-version
                {
                    char* ipvend;
                    long int ip_ver = strtol(token, &ipvend, 10);
                    if(ipvend == NULL) return 1;
                    result->ipversion = (int)ip_ver;
                }
            break;
        }
        field++;
    }
    return 0;
}
