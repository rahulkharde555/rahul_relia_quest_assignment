#include "url_encode.h"
#include <curl/curl.h>

//URL encode the employee name:
bool urlencode(const std::string& name, std::string& encoded_name) 
{
    CURL *curl = curl_easy_init();
    if(!curl) {
        return false;
    }
    
    char *output = curl_easy_escape(curl, name.c_str(), name.length());
    if(!output) {
        return false;
    }
    encoded_name = output;
    curl_free(output);
    curl_easy_cleanup(curl);
    return true;
}