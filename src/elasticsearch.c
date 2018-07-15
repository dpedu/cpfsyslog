#include <string.h>
#include <curl/curl.h>


size_t write_data(char *dbit, size_t size, size_t nmemb, void *user_data) {
    char *data = (char*)user_data;
    static size_t data_size = 0;
    size_t n = size * nmemb;
    memcpy(data + data_size, dbit, n);
    data_size += n;
    data[data_size] = '\0';
    return n;
}


void test_curl() {
    CURL *curl;
    CURLcode res;
    char data[50000] = "";

    curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.1.120:8298/");
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &data);

    res = curl_easy_perform(curl);

    curl_easy_cleanup(curl);
    curl_global_cleanup();
    printf("%d %s", res, data);
}



int put_events(char* data) {
    CURL *curl;
    CURLcode res;
    char response[50000] = "";

    curl = curl_easy_init(); // check this and all of these curl functions
    curl_easy_setopt(curl, CURLOPT_URL, "http://192.168.1.120:8298/_bulk");
    curl_easy_setopt(curl, CURLOPT_POST, 1);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, data);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    // curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);

    struct curl_slist *headers=NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

    res = curl_easy_perform(curl);

    curl_slist_free_all(headers);

    curl_easy_cleanup(curl);
    curl_global_cleanup();

    if(res != CURLE_OK) {
        printf("%d %s\n", res, response);
    }

    return res == CURLE_OK ? 0 : 1;
}




