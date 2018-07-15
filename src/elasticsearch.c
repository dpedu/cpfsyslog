#include <string.h>
#include <curl/curl.h>


size_t write_data(char *dbit, size_t size, size_t nmemb, void *user_data) {
    // Discard curl output
    return size * nmemb;
}


int put_events(char* data, char* es_url) {  // es_url should be a string like 'http://192.168.1.120:8298'
    CURL *curl;
    CURLcode res;

    char* endpoint = "/_bulk";
    char final_url[strlen(es_url) + strlen(endpoint) + 1];
    sprintf(final_url, "%s%s", es_url, endpoint);

    curl = curl_easy_init(); // check this and all of these curl functions

    struct curl_slist *headers=NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    curl_easy_setopt(curl, CURLOPT_URL, final_url);
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

    res = curl_easy_perform(curl);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    curl_global_cleanup();

    if(res != CURLE_OK) {
        // printf("%d %s\n", res, response);
        printf("CURL returned: %d\n", res);
    }

    return res == CURLE_OK ? 0 : 1;
}
