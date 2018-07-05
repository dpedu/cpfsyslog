#include <stdlib.h>
#include <json-c/json.h>


#define IFACE_LEN 8


typedef enum pf_hit_reason {
    pf_hit_match,
    pf_hit_other
} pf_hit_reason;

const static char* pfhrstr[] __attribute__ ((unused)) =
    {[pf_hit_match] = "match",
     [pf_hit_other] = "other"};


typedef enum pf_hit_action {
    pf_hit_block,
    pf_hit_pass
} pf_hit_action;

const static char* pfhastr[] __attribute__ ((unused)) =
    {[pf_hit_block] = "block",
     [pf_hit_pass] = "pass"};


typedef enum pf_direction {
    pf_dir_in,
    pf_dir_out
} pf_direction;

const static char* pfdirstr[] __attribute__ ((unused)) =
    {[pf_dir_in] = "in",
     [pf_dir_out] = "out"};


typedef struct pf_data_ipv4 {
    int ttl;
    int tos;
    int protocol;
} pf_data_ipv4;

typedef struct pf_data_ipv6 {
    int hoplimit;
    int protocol;
} pf_data_ipv6;

/*typedef struct ipv4_addr {
    u_int32_t addr;
} ipv4_addr;

typedef struct ipv6_addr {
    u_int32_t addr1;
    u_int32_t addr2;
    u_int32_t addr3;
    u_int32_t addr4;
} ipv6_addr;*/

typedef struct pf_data_tcp {
    int srcport;
    int destport;
    int length;
} pf_data_tcp;

typedef struct pf_data_udp {
    int srcport;
    int destport;
    int length;
} pf_data_udp;

#define IP_STR_LEN 41 /*40 char ipv6 address + null term*/


typedef struct pf_data {
    int rulenum;
    char iface[IFACE_LEN];
    pf_hit_reason reason;
    pf_hit_action action;
    pf_direction direction;
    int ipversion;
    union {
        pf_data_ipv4 ipv4_data;
        pf_data_ipv6 ipv6_data;
    };
    /*union {
        ipv4_addr ipv4_src;
        ipv6_addr ipv6_src;
    };
    union {
        ipv4_addr ipv4_dest;
        ipv6_addr ipv6_dest;
    };*/
    int packet_length;
    char src_addr[IP_STR_LEN];
    char dest_addr[IP_STR_LEN];
    union {
        pf_data_tcp tcp_data;
        pf_data_udp udp_data;
    };
} pf_data;


int pfdata_parse(char* message, pf_data* result);

void pfdata_print(pf_data* data);

int pfdata_to_json(pf_data* data, json_object* obj);
