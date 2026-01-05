#include "ui/application.h"
#include "utils/logger.h"
#include <iostream>
#include <exception>

int main(int argc, char** argv) {
    try {
        ytui::Application app;

        if (!app.initialize()) {
            std::cerr << "Failed to initialize application" << std::endl;
            return 1;
        }

        app.run();
        app.shutdown();

        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        ytui::Logger::error(std::string("Fatal error: ") + e.what());
        return 1;
    }
}
