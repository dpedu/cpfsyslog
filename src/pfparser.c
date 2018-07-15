#include <stdio.h>
#include <string.h>
#include "pfparser.h"
#include "geo.h"


int pfdata_parse(char* message, pf_data* result) {
    /*printf("pfparse: '%s'\n", message);*/

    char* token;
    int field = 0;

    /* Parse the first 9 fields
       They are: <rule-number>,<sub-rule-number>,<anchor>,<tracker>,<real-interface>,<reason>,<action>,<direction>,<ip-version>
       We only collect rule-number, real-interface, reason, action, direction, ip-version */
    while ( (token = strsep(&message, ",")) != NULL) {
        /*printf("%02d: %s\n", field, token);*/
        switch (field) {
            case 0: /* Rule number*/
                {  /*language limitation, the `char*` label (or `unsigned`) is not supported after a switch case TODO look up the underlying reason again*/
                    char* rnend;
                    long int rulenum = strtol(token, &rnend, 10);
                    if(rnend == NULL) return 1;
                    result->rulenum = (int)rulenum;
                }
                /*if(result->rulenum == NULL) return 1;*/
            break;

            case 4: /*iface*/
                if(strlen(token) > IFACE_LEN) return 1;  /*oddly log interface name, avoid buf overflow*/
                memcpy(result->iface, token, strlen(token));
            break;

            case 5: /*reason*/
                result->reason = strcmp(token, "match") ? pf_hit_other : pf_hit_match;  /*TODO error on unexpected*/
            break;

            case 6: /*action*/
                result->action = strcmp(token, "block") ? pf_hit_block : pf_hit_pass;  /*XXX*/
            break;

            case 7: /*direction*/
                result->direction = strcmp(token, "in") ? pf_dir_in : pf_dir_out;  /*XXX*/
            break;

            case 8: /*ip-version*/
                {
                    char* ipvend;
                    long int ip_ver = strtol(token, &ipvend, 10);
                    if(ipvend == NULL) return 1;
                    result->ipversion = (int)ip_ver;
                }
            break;
        }
        if(field == 8) {
            break;
        }
        field++;
    }

    if(result->ipversion == 4) {
        /*parse ipv4 fields*/
        field = 0;
        while ( (token = strsep(&message, ",")) != NULL) {
            /*printf("%02d: %s\n", field, token);*/
            switch (field) {
                case 0: /*TOS, hex as a string field starting with "0x" or empty*/
                    {
                        char* rnend;
                        int tos = strtol(token, &rnend, 0);
                        if(rnend == NULL) return 1;
                        result->ipv4_data.tos = tos;
                    }
                break;
                case 1: /*"Explicit Congestion Notification" - or empty, we will ignore*/
                break;
                case 2: /*TTL, int*/
                    {
                        char* rnend;
                        int ttl = strtol(token, &rnend, 0);
                        if(rnend == NULL) return 1;
                        result->ipv4_data.ttl = ttl;
                    }
                break;
                case 3: /*packet ID, int (seemingly useless?)*/
                break;
                case 4: /*fragment offset, int (???)*/
                break;
                case 5: /*flags ("none" or some string, DF or MF - see https://en.wikipedia.org/wiki/IPv4#Flags)*/
                break;
                case 6: /*protocol id, int*/
                    {
                        char* rnend;
                        int proto_id = strtol(token, &rnend, 0);
                        if(rnend == NULL) return 1;
                        result->ipv4_data.protocol = proto_id;
                    }
                break;
                case 7: /*protocol name, string*/
                break;
            }
            if(field == 7) {
                break;
            }
            field++;
        }
    }
    else if(result->ipversion == 6) {
        /*parse ipv6 fields*/
        field = 0;
        while ( (token = strsep(&message, ",")) != NULL) {
            /*printf("%02d: %s\n", field, token);*/
            switch (field) {
                case 0: /*class, hex as a string field starting with "0x"*/
                break;
                case 1: /*flow label, "data" ???*/
                break;
                case 2: /*hop-limit, int (like ttl)*/
                    {
                        char* rnend;
                        int ttl = strtol(token, &rnend, 0);
                        if(rnend == NULL) return 1;
                        result->ipv6_data.hoplimit = ttl;
                    }
                break;
                case 3: /*protocol name, string*/
                break;
                case 4: /*protocol id, int*/
                    {
                        char* rnend;
                        int proto_id = strtol(token, &rnend, 0);
                        if(rnend == NULL) return 1;
                        result->ipv6_data.protocol = proto_id;
                    }
                break;
            }
            if(field == 4) {
                break;
            }
            field++;
        }
    } else {
        return 1; /*unknown ip version*/
    }

    /*Parse <ip-data>*/
    /*parse ipv6 fields*/
    field = 0;
    while ( (token = strsep(&message, ",")) != NULL) {
        /*printf("%02d: %s\n", field, token);*/
        switch (field) {
            case 0: /*packet length, int*/
                {
                    char* rnend;
                    int pack_len = strtol(token, &rnend, 0);
                    if(rnend == NULL) return 1;
                    result->packet_length = pack_len;
                }
            break;
            case 1: /*source addr, string (ipv4 OR ipv6!)*/
                if(strlen(token) > IP_STR_LEN) return 1;  /*too long ip string*/
                memcpy(result->src_addr, token, strlen(token));
            break;
            case 2: /*dest addr, string (ipv4 OR ipv6!)*/
                if(strlen(token) > IP_STR_LEN) return 1;  /*too long ip string*/
                memcpy(result->dest_addr, token, strlen(token));
            break;
        }
        if(field == 2) {
            break;
        }
        field++;
    }

    /*Parse optional <protocol-specific-data>
      one of <tcp-data> | <udp-data> | <icmp-data> | <carp-data>
      ICMP and CARP are ignored*/
    if((result->ipversion == 4 && result->ipv4_data.protocol == 6) ||
       (result->ipversion == 6 && result->ipv6_data.protocol == 6)) {/* tcp */
        /*<source-port>,<destination-port>,<data-length>,<tcp-flags>,<sequence-number>,<ack-number>,<tcp-window>,<urg>,<tcp-options>*/
        /*printf("rest: %s\n", message);*/
        /*parse ipv6 fields*/
        field = 0;
        while ( (token = strsep(&message, ",")) != NULL) {
            /*printf("%02d: %s\n", field, token);*/
            switch (field) {
                case 0: /*src port, int*/
                    {
                        char* rnend;
                        int num = strtol(token, &rnend, 0);
                        if(rnend == NULL) return 1;
                        result->tcp_data.srcport = num;
                    }
                break;
                case 1: /*dest port, int*/
                    {
                        char* rnend;
                        int num = strtol(token, &rnend, 0);
                        if(rnend == NULL) return 1;
                        result->tcp_data.destport = num;
                    }
                break;
                case 2: /*packet length, int*/
                    {
                        char* rnend;
                        int num = strtol(token, &rnend, 0);
                        if(rnend == NULL) return 1;
                        result->tcp_data.length = num;
                    }
                break;
            }
            if(field == 8) {
                break;
            }
            field++;
        }
    } else if((result->ipversion == 4 && result->ipv4_data.protocol == 11) ||
              (result->ipversion == 6 && result->ipv6_data.protocol == 11)) {/* udp */
        /*<source-port>,<destination-port>,<data-length>*/
        field = 0;
        while ( (token = strsep(&message, ",")) != NULL) {
            /*printf("%02d: %s\n", field, token);*/
            switch (field) {
                case 0: /*src port, int*/
                    {
                        char* rnend;
                        int num = strtol(token, &rnend, 0);
                        if(rnend == NULL) return 1;
                        result->udp_data.srcport = num;
                    }
                break;
                case 1: /*dest port, int*/
                    {
                        char* rnend;
                        int num = strtol(token, &rnend, 0);
                        if(rnend == NULL) return 1;
                        result->udp_data.destport = num;
                    }
                break;
                case 2: /*packet length, int*/
                    {
                        char* rnend;
                        int num = strtol(token, &rnend, 0);
                        if(rnend == NULL) return 1;
                        result->udp_data.length = num;
                    }
                break;
            }
            if(field == 2) {
                break;
            }
            field++;
        }
    } else if(result->ipversion == 4 && result->ipv4_data.protocol == 1){/* icmp-v4 */

    } else if(result->ipversion == 6 && result->ipv6_data.protocol == 58){/* icmp-v6 */

    }

    return 0;
}


void pfdata_print(pf_data* data) {
    printf("Action: %s\n", pfhastr[data->action]);
    printf("IP Data:\n\tInterface: %s\n\tIP version: %d\n",
           data->iface,
           data->ipversion);
    if(data->ipversion == 4) {
        printf("IPV4 Data:\n\tTTL: %d\n\tProtocol ID: %d\n",
               data->ipv4_data.ttl,
               data->ipv4_data.protocol);
    } else if(data->ipversion == 6) {
        printf("IPV6 Data:\n\tHop Limit: %d\n\tProtocol ID: %d\n",
               data->ipv6_data.hoplimit,
               data->ipv6_data.protocol);
    }
    printf("Endpoints:\n\tsrc: %s\n\tdest: %s\n",
               data->src_addr,
               data->dest_addr);

    if (data->ipversion == 4) {
        if (data->ipv4_data.protocol == 6) {
            printf("Protocol: tcp4\n\tsrcport: %d\n\tdestport: %d\n\tsize: %d\n",
                   data->tcp_data.srcport, data->tcp_data.destport, data->tcp_data.length);
        } else if (data->ipv4_data.protocol == 11) {
            printf("Protocol: udp4\n\tsrcport: %d\n\tdestport: %d\n\tsize: %d\n",
                   data->udp_data.srcport, data->udp_data.destport, data->udp_data.length);
        }
    } else if (data->ipversion == 6) {
        if (data->ipv6_data.protocol == 6) {
            printf("Protocol: tcp6\n\tsrcport: %d\n\tdestport: %d\n\tsize: %d\n",
                   data->tcp_data.srcport, data->tcp_data.destport, data->tcp_data.length);
        } else if (data->ipv6_data.protocol == 11) {
            printf("Protocol: udp6\n\tsrcport: %d\n\tdestport: %d\n\tsize: %d\n",
                   data->udp_data.srcport, data->udp_data.destport, data->udp_data.length);
        }
    }
}


void add_intfield(json_object* obj, char* name, int value) {
    json_object *ipversion = json_object_new_int(value);
    json_object_object_add(obj, name, ipversion);
}


void add_strfield(json_object* obj, char* name, char* value) {
    json_object *ipversion = json_object_new_string(value);
    json_object_object_add(obj, name, ipversion);
}


void add_doublefield(json_object* obj, char* name, double value) {
    json_object *number = json_object_new_double(value);
    json_object_object_add(obj, name, number);
}


const char* null_unknown(const char* p){
    return p ? p : "unknown";
}


int pfdata_to_json(pf_data* data, json_object* obj) {
    /*
    Populate the passed json_object obj with data from from pf_data data.
    */
    add_strfield(obj, "interface", data->iface);
    add_intfield(obj, "ip_version", data->ipversion);

    add_strfield(obj, "action", (char*)(pfhastr[data->action]));
    add_strfield(obj, "direction", (char*)(pfdirstr[data->direction]));

    if(data->ipversion == 4) {
        add_intfield(obj, "ttl", data->ipv4_data.ttl);
        add_intfield(obj, "protocol_id", data->ipv4_data.protocol);
    } else if(data->ipversion == 6) {
        add_intfield(obj, "ttl", data->ipv6_data.hoplimit);
        add_intfield(obj, "protocol_id", data->ipv6_data.protocol);
    }

    add_strfield(obj, "src_addr", data->src_addr);
    add_strfield(obj, "dest_addr", data->dest_addr);

    if (data->ipversion == 4) {
        if (data->ipv4_data.protocol == 6) {
            add_intfield(obj, "src_port", data->tcp_data.srcport);
            add_intfield(obj, "dest_port", data->tcp_data.destport);
            add_intfield(obj, "length", data->tcp_data.length);
        } else if (data->ipv4_data.protocol == 11) {
            add_intfield(obj, "src_port", data->udp_data.srcport);
            add_intfield(obj, "dest_port", data->udp_data.destport);
            add_intfield(obj, "length", data->udp_data.length);
        }
    } else if (data->ipversion == 6) {
        if (data->ipv6_data.protocol == 6) {
            add_intfield(obj, "src_port", data->tcp_data.srcport);
            add_intfield(obj, "dest_port", data->tcp_data.destport);
            add_intfield(obj, "length", data->tcp_data.length);
        } else if (data->ipv6_data.protocol == 11) {
            add_intfield(obj, "src_port", data->udp_data.srcport);
            add_intfield(obj, "dest_port", data->udp_data.destport);
            add_intfield(obj, "length", data->udp_data.length);
        }
    }

    GeoIPRecord* ginfo = (data->ipversion == 4 ? geo_get(data->src_addr)
                                               : geo_get6(data->src_addr));
    if(ginfo != NULL) {
        json_object* srcloc = json_object_new_object();
        json_object_object_add(obj, "srcloc", srcloc);
        add_doublefield(srcloc, "lat", ginfo->latitude);
        add_doublefield(srcloc, "lon", ginfo->longitude);
        add_strfield(obj, "src_country", (char*)null_unknown(geo_country_name(ginfo)));
        add_strfield(obj, "src_country_code", (char*)null_unknown(ginfo->country_code));
        add_strfield(obj, "src_region",  (char*)null_unknown(ginfo->region));
        add_strfield(obj, "src_state",   (char*)null_unknown(GeoIP_region_name_by_code(ginfo->country_code, ginfo->region)));
        add_strfield(obj, "src_city",    (char*)null_unknown(ginfo->city));
    }

    GeoIPRecord_delete(ginfo);

    return 0;
}
