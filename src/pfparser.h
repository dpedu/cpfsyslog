#define IFACE_LEN 8



typedef enum pf_hit_reason {
    pf_hit_match,
    pf_hit_other
} pf_hit_reason;

typedef enum pf_hit_action {
    pf_hit_block,
    pf_hit_pass
} pf_hit_action;

typedef enum pf_direction {
    pf_dir_in,
    pf_dir_out
} pf_direction;

typedef struct pf_data {
    int rulenum;
    char iface[IFACE_LEN];
    pf_hit_reason reason;
    pf_hit_action action;
    pf_direction direction;
    int ipversion;
} pf_data;



int pfparse_message(char* message, pf_data* result);
