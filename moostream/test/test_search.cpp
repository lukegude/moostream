#include "../src/youtube/extractor.h"
#include "../src/utils/logger.h"
#include "../src/core/config.h"
#include <iostream>

int main() {
    ytui::Logger::init("/tmp/test.log");
    ytui::Config::instance().load();

    ytui::YouTubeExtractor extractor;
    std::cout << "Testing search..." << std::endl;

    auto results = extractor.search("lofi", 5);
    std::cout << "Found " << results.size() << " results" << std::endl;

    for (const auto& track : results) {
        std::cout << "Title: " << track.title << std::endl;
        std::cout << "Channel: " << track.channel << std::endl;
        std::cout << "URL: " << track.url << std::endl;
        std::cout << "Is Live: " << (track.is_live ? "YES" : "NO") << std::endl;
        std::cout << "---" << std::endl;
    }

    return 0;
}