#include <iostream>
#include <string>
#include "../src/youtube/extractor.h"

int main() {
    ytui::YouTubeExtractor extractor;

    // Test with a string containing unicode escapes
    std::string test_input = "Hello \\u00e9\\u00f1\\u00fc"; // éñü in unicode escapes
    std::string decoded = extractor.decode_unicode_escapes(test_input);

    std::cout << "Input: " << test_input << std::endl;
    std::cout << "Decoded: " << decoded << std::endl;

    // Check if it's properly UTF-8
    for (char c : decoded) {
        std::cout << std::hex << (int)(unsigned char)c << " ";
    }
    std::cout << std::dec << std::endl;

    return 0;
}