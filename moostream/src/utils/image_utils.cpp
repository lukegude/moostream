#include "image_utils.h"
#include "stb_image.h"
#include <string>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <curl/curl.h>

namespace ytui {

static size_t curl_write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    std::vector<unsigned char>* buffer = static_cast<std::vector<unsigned char>*>(userp);
    size_t realsize = size * nmemb;
    const unsigned char* data = static_cast<const unsigned char*>(contents);
    buffer->insert(buffer->end(), data, data + realsize);
    return realsize;
}

std::vector<unsigned char> download_image(const std::string& url) {
    std::vector<unsigned char> result;
    
    if (url.empty()) {
        return result;
    }
    
    CURL* curl = curl_easy_init();
    if (!curl) {
        return result;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, curl_write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &result);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
    
    CURLcode res = curl_easy_perform(curl);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        result.clear();
    }
    
    return result;
}

std::string image_to_ascii(const std::vector<unsigned char>& image_data,
                           int target_width, int target_height) {
    if (image_data.empty()) {
        return image_to_8bit_ascii(image_data);
    }
    
    int width, height, channels;
    unsigned char* img = stbi_load_from_memory(
        image_data.data(),
        static_cast<int>(image_data.size()),
        &width, &height, &channels, 0
    );
    
    if (!img) {
        return image_to_8bit_ascii(image_data);
    }
    
    const char* shades = " .:-=+*#%@";
    const int num_shades = 10;
    
    std::stringstream ss;
    
    float x_step = static_cast<float>(width) / target_width;
    float y_step = static_cast<float>(height) / target_height;
    
    for (int y = 0; y < target_height; ++y) {
        for (int x = 0; x < target_width; ++x) {
            int px = static_cast<int>(x * x_step);
            int py = static_cast<int>(y * y_step);
            
            px = std::min(px, width - 1);
            py = std::min(py, height - 1);
            
            int idx = (py * width + px) * channels;
            
            int lum;
            if (channels >= 3) {
                lum = static_cast<int>(0.299f * img[idx] + 0.587f * img[idx + 1] + 0.114f * img[idx + 2]);
            } else {
                lum = img[idx];
            }
            
            int shade_idx = (lum * (num_shades - 1)) / 255;
            shade_idx = std::max(0, std::min(shade_idx, num_shades - 1));
            
            ss << shades[shade_idx];
        }
        ss << '\n';
    }
    
    stbi_image_free(img);
    
    return ss.str();
}

ColoredAsciiArt image_to_colored_ascii(const std::vector<unsigned char>& image_data,
                                        int target_width, int target_height) {
    ColoredAsciiArt result;
    result.width = target_width;
    result.height = target_height;
    
    if (image_data.empty()) {
        return result;
    }
    
    int width, height, channels;
    unsigned char* img = stbi_load_from_memory(
        image_data.data(),
        static_cast<int>(image_data.size()),
        &width, &height, &channels, 0
    );
    
    if (!img) {
        return result;
    }
    
    const char* shades = ".:;+xX#%@";
    const int num_shades = 9;
    
    float x_step = static_cast<float>(width) / target_width;
    float y_step = static_cast<float>(height) / target_height;
    
    result.rows.resize(target_height);
    
    for (int y = 0; y < target_height; ++y) {
        result.rows[y].resize(target_width);
        
        for (int x = 0; x < target_width; ++x) {
            int px = static_cast<int>(x * x_step);
            int py = static_cast<int>(y * y_step);
            
            px = std::min(px, width - 1);
            py = std::min(py, height - 1);
            
            int idx = (py * width + px) * channels;
            
            float r, g, b;
            int lum;
            
            if (channels >= 3) {
                r = img[idx] / 255.0f;
                g = img[idx + 1] / 255.0f;
                b = img[idx + 2] / 255.0f;
                lum = static_cast<int>(0.299f * img[idx] + 0.587f * img[idx + 1] + 0.114f * img[idx + 2]);
            } else {
                r = g = b = img[idx] / 255.0f;
                lum = img[idx];
            }
            
            int shade_idx = (lum * (num_shades - 1)) / 255;
            shade_idx = std::max(0, std::min(shade_idx, num_shades - 1));
            
            result.rows[y][x] = {shades[shade_idx], r, g, b};
        }
    }
    
    stbi_image_free(img);
    
    return result;
}

std::string image_to_8bit_ascii([[maybe_unused]] const std::vector<unsigned char>& image_data) {
    std::stringstream ss;
    ss << "+--------------------+\n";
    ss << "|                    |\n";
    ss << "|   ##############   |\n";
    ss << "|   #            #   |\n";
    ss << "|   #   --  --   #   |\n";
    ss << "|   #            #   |\n";
    ss << "|   #    ----    #   |\n";
    ss << "|   #            #   |\n";
    ss << "|   ##############   |\n";
    ss << "|                    |\n";
    ss << "+--------------------+\n";
    return ss.str();
}

}
