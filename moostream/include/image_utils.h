#pragma once

#include <string>
#include <vector>

namespace ytui {

struct ColoredChar {
    char ch;
    float r, g, b;
};

struct ColoredAsciiArt {
    std::vector<std::vector<ColoredChar>> rows;
    int width;
    int height;
    
    bool empty() const { return rows.empty(); }
};

std::vector<unsigned char> download_image(const std::string& url);

std::string image_to_ascii(const std::vector<unsigned char>& image_data,
                           int target_width = 24, int target_height = 12);

ColoredAsciiArt image_to_colored_ascii(const std::vector<unsigned char>& image_data,
                                        int target_width = 24, int target_height = 12);

std::string image_to_8bit_ascii(const std::vector<unsigned char>& image_data);

}
