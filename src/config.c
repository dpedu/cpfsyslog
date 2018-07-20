#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <json-c/json.h>
#include "helpers.h"
#include "config.h"
#include "elasticsearch.h"


char* get_strfield(json_object* obj, char* key) {
    json_object *o_value = json_object_object_get(obj, key);
    if(o_value == NULL)
        return NULL;
    const char* value = json_object_get_string(o_value);
    if(value == NULL)
        return NULL;
    char* dest = malloc(strlen(value) + 1);
    strcpy(dest, value);
    return dest;
}


int get_intfield(json_object* obj, char* key) {
    json_object *o_value = json_object_object_get(obj, key);
    if(o_value == NULL)
        return 0;
    return json_object_get_int(o_value);
}


struct config* config_load(char* conf_path) {
    int fd;
    if((fd = open(conf_path, O_RDONLY)) == -1)
        panic("Config load failed");

    struct stat fsize = {0};
    if(fstat(fd, &fsize) != 0)
        die("Conf stat failed");

    if(fsize.st_size == 0)
        die("Empty config");

    char* confdata = mmap(NULL, fsize.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if(confdata == MAP_FAILED)
        panic("Failed to mmap conf");

    json_object *j_conf;
    if((j_conf = json_tokener_parse(confdata)) == NULL)
        die("Json parse failed");

    struct config* conf = calloc(1, sizeof(struct config));

    if((conf->url = get_strfield(j_conf, "elasticsearch")) == NULL)
        die("Config missing or invalid elasticsearch url");

    if((conf->port = get_intfield(j_conf, "serverport")) == 0)
        die("Config missing or invalid serverport number");

    json_object_put(j_conf);

    if(munmap(confdata, fsize.st_size) != 0)
        panic("munmap failed");

    close(fd);
    return conf;
}


void config_free(struct config* conf) {
    free(conf->url);
    free(conf);
}
