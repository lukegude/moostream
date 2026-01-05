#pragma once

#include <string>
#include <curl/curl.h>

namespace ytui {

class HttpClient {
public:
    HttpClient();
    ~HttpClient();

    std::string get(const std::string& url, const std::string& auth_token = "");
    std::string post(const std::string& url, const std::string& data, const std::string& auth_token = "");

private:
    static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp);

    CURL* curl_;
};

} // namespace ytui