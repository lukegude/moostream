#include "ui/application.h"
#include "utils/logger.h"
#include "core/config.h"
#include "youtube/extractor.h"
#include <iostream>
#include <exception>
#include <memory>
#include <string>
#include <cctype>

void run_auth() {
    std::cout << "Moostream OAuth Setup" << std::endl;
    std::cout << "====================" << std::endl;
    std::cout << std::endl;

    // Initialize logger
    ytui::Logger::init("/tmp/moostream.log");

    // Load config
    ytui::Config::instance().load();

    // Check if client ID is configured
    std::string client_id = ytui::Config::instance().get_youtube_client_id();
    std::cout << "Current client ID: '" << client_id.substr(0, 30) << "...'" << std::endl;

    if (client_id.empty() || client_id == "YOUR_CLIENT_ID_HERE") {
        std::cout << "✗ YouTube client ID not configured." << std::endl;
        std::cout << "Please edit ~/.config/moostream/config and set:" << std::endl;
        std::cout << "youtube_client_id=YOUR_CLIENT_ID_FROM_GOOGLE" << std::endl;
        std::cout << std::endl;
        std::cout << "Get the client ID from:" << std::endl;
        std::cout << "https://console.cloud.google.com/ > APIs & Services > Credentials" << std::endl;
        std::cout << "Create: OAuth 2.0 Client IDs > TVs and Limited Input devices" << std::endl;
        return;
    }

    // Validate client ID format (should end with .apps.googleusercontent.com)
    if (client_id.find(".apps.googleusercontent.com") == std::string::npos) {
        std::cout << "⚠ Warning: Client ID doesn't look like a valid Google OAuth ID" << std::endl;
        std::cout << "It should end with '.apps.googleusercontent.com'" << std::endl;
        std::cout << "Double-check you copied the correct client ID from Google Cloud Console." << std::endl;
        std::cout << std::endl;
    }

    // Check if we already have tokens
    std::string existing_token = ytui::Config::instance().get_youtube_access_token();
    if (!existing_token.empty()) {
        std::cout << "✓ You are already authenticated!" << std::endl;
        std::cout << "You can use the application normally: ./moostream" << std::endl;
        return;
    }

    std::cout << "Starting OAuth authentication..." << std::endl;
    std::cout << std::endl;

    // Create extractor and run auth
    auto extractor = std::make_unique<ytui::YouTubeExtractor>();

    std::string token = extractor->get_access_token();
    if (!token.empty()) {
        std::cout << "✓ Authentication successful!" << std::endl;
        std::cout << "You can now use the application normally: ./moostream" << std::endl;
    } else {
        std::cout << "✗ Authentication failed." << std::endl;
        std::cout << "Check the logs at /tmp/moostream.log for details." << std::endl;
        std::cout << "Common issues:" << std::endl;
        std::cout << "1. Wrong client type (must be 'TVs and Limited Input devices')" << std::endl;
        std::cout << "2. YouTube Data API v3 not enabled in your project" << std::endl;
        std::cout << "3. OAuth consent screen not configured" << std::endl;
    }
}

void run_auth_clear() {
    std::cout << "Moostream OAuth Token Clear" << std::endl;
    std::cout << "==========================" << std::endl;
    std::cout << std::endl;

    // Initialize logger
    ytui::Logger::init("/tmp/moostream.log");

    // Load config
    ytui::Config::instance().load();

    // Check if we have tokens
    std::string access_token = ytui::Config::instance().get_youtube_access_token();
    std::string refresh_token = ytui::Config::instance().get_youtube_refresh_token();

    if (access_token.empty() && refresh_token.empty()) {
        std::cout << "No authentication tokens found to clear." << std::endl;
        return;
    }

    std::cout << "Current authentication status:" << std::endl;
    std::cout << "Access token: " << (access_token.empty() ? "Not set" : "Set") << std::endl;
    std::cout << "Refresh token: " << (refresh_token.empty() ? "Not set" : "Set") << std::endl;
    std::cout << std::endl;

    std::cout << "Are you sure you want to clear all YouTube authentication tokens? (y/N): ";
    char response;
    std::cin >> response;

    if (std::tolower(response) == 'y') {
        ytui::Config::instance().set_youtube_access_token("");
        ytui::Config::instance().set_youtube_refresh_token("");
        ytui::Config::instance().set_youtube_token_expiry(0);
        ytui::Config::instance().save();

        std::cout << "✓ Authentication tokens cleared successfully." << std::endl;
        std::cout << "You can re-authenticate by running: ./moostream auth" << std::endl;
    } else {
        std::cout << "Operation cancelled." << std::endl;
    }
}

int main(int argc, char** argv) {
    // Check for auth command
    if (argc >= 2 && std::string(argv[1]) == "auth") {
        if (argc >= 3 && std::string(argv[2]) == "clear") {
            run_auth_clear();
        } else {
            run_auth();
        }
        return 0;
    }

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
