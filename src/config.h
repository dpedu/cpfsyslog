struct config {
    char* url;
    int port;
};


struct config* config_load(char* conf_path);

void config_free(struct config* conf);
