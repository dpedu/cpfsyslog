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
    int derp;
} pf_data_ipv4;

typedef struct pf_data_ipv6 {
    int derp;
    int derp2;
} pf_data_ipv6;


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
} pf_data;


int pfdata_parse(char* message, pf_data* result);
