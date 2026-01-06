#include "image_utils.h"
#include <string>
#include <sstream>

namespace ytui {

std::string image_to_8bit_ascii(const std::vector<unsigned char>& image_data) {
    if (image_data.empty()) {
        return "[No image data]";
    }

    // Simple 8bit style ASCII art placeholder
    // In a full implementation, this would decode the image and convert to ASCII
    // For now, return a retro-style placeholder
    std::stringstream ss;
    ss << "  8BIT COVER  \n";
    ss << "██████████████\n";
    ss << "█░░░░░░░░░░░░█\n";
    ss << "█░████████░░░█\n";
    ss << "█░█░░░░░░░█░░█\n";
    ss << "█░████████░░░█\n";
    ss << "█░░░░░░░░░░░░█\n";
    ss << "██████████████\n";
    return ss.str();
}

}