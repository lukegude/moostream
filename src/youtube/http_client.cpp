#include "youtube/http_client.h"
#include "utils/logger.h"
#include <curl/curl.h>
#include <string>
#include <cstring>

namespace ytui {

HttpClient::HttpClient() : curl_(nullptr) {
    curl_ = curl_easy_init();
    if (!curl_) {
        Logger::error("Failed to initialize CURL");
    }
}

HttpClient::~HttpClient() {
    if (curl_) {
        curl_easy_cleanup(curl_);
    }
}

size_t HttpClient::write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), realsize);
    return realsize;
}

std::string HttpClient::get(const std::string& url, const std::string& auth_token) {
    if (!curl_) {
        Logger::error("CURL not initialized");
        return "";
    }

    // Reset handle to clear any previous POST state
    curl_easy_reset(curl_);

    std::string response;
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_HTTPGET, 1L);  // Explicitly set GET
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 10L);  // 10 second timeout

    // Set Authorization header if token provided
    struct curl_slist* headers = nullptr;
    if (!auth_token.empty()) {
        std::string auth_header = "Authorization: Bearer " + auth_token;
        headers = curl_slist_append(headers, auth_header.c_str());
        curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);
    }

    CURLcode res = curl_easy_perform(curl_);
    if (headers) {
        curl_slist_free_all(headers);
    }

    if (res != CURLE_OK) {
        Logger::error("CURL request failed: " + std::string(curl_easy_strerror(res)));
        return "";
    }

    return response;
}

std::string HttpClient::post(const std::string& url, const std::string& data, const std::string& auth_token) {
    if (!curl_) {
        Logger::error("CURL not initialized");
        return "";
    }

    curl_easy_reset(curl_);

    std::string response;
    curl_easy_setopt(curl_, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl_, CURLOPT_POST, 1L);
    curl_easy_setopt(curl_, CURLOPT_POSTFIELDS, data.c_str());
    curl_easy_setopt(curl_, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl_, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl_, CURLOPT_TIMEOUT, 10L);  // 10 second timeout

    // Set headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, "Content-Type: application/x-www-form-urlencoded");

    if (!auth_token.empty()) {
        std::string auth_header = "Authorization: Bearer " + auth_token;
        headers = curl_slist_append(headers, auth_header.c_str());
    }

    curl_easy_setopt(curl_, CURLOPT_HTTPHEADER, headers);

    CURLcode res = curl_easy_perform(curl_);
    if (headers) {
        curl_slist_free_all(headers);
    }

    if (res != CURLE_OK) {
        Logger::error("CURL POST request failed: " + std::string(curl_easy_strerror(res)));
        return "";
    }

    return response;
}

} // namespace ytui