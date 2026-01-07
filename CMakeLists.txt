cmake_minimum_required(VERSION 3.15)
project(moostream VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTS "Build tests" OFF)

# macOS: Set up Homebrew paths for ncurses before finding packages
if(APPLE)
    execute_process(
        COMMAND brew --prefix
        OUTPUT_VARIABLE HOMEBREW_PREFIX
        OUTPUT_STRIP_TRAILING_WHITESPACE
        ERROR_QUIET
    )
    if(HOMEBREW_PREFIX)
        set(ENV{PKG_CONFIG_PATH} "${HOMEBREW_PREFIX}/opt/ncurses/lib/pkgconfig:${HOMEBREW_PREFIX}/lib/pkgconfig:$ENV{PKG_CONFIG_PATH}")
        set(CMAKE_PREFIX_PATH "${HOMEBREW_PREFIX}/opt/ncurses;${HOMEBREW_PREFIX};${CMAKE_PREFIX_PATH}")
        list(APPEND CMAKE_LIBRARY_PATH "${HOMEBREW_PREFIX}/opt/ncurses/lib")
        list(APPEND CMAKE_INCLUDE_PATH "${HOMEBREW_PREFIX}/opt/ncurses/include")
    endif()
endif()

# Find required packages
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

# Find ncurses
pkg_check_modules(NCURSES REQUIRED ncurses)

# Find mpv
pkg_check_modules(MPV REQUIRED mpv)

# Find curl
pkg_check_modules(CURL REQUIRED libcurl)

# Add imtui as subdirectory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/imtui/CMakeLists.txt")
    # Help imtui find ncurses on macOS
    if(APPLE AND HOMEBREW_PREFIX)
        set(CURSES_INCLUDE_PATH "${HOMEBREW_PREFIX}/opt/ncurses/include")
        set(CURSES_LIBRARY "${HOMEBREW_PREFIX}/opt/ncurses/lib/libncurses.dylib")
        set(CURSES_NCURSES_LIBRARY "${HOMEBREW_PREFIX}/opt/ncurses/lib/libncurses.dylib")
    endif()
    add_subdirectory(external/imtui)
else()
    message(WARNING "imtui not found in external/imtui. Please run: git submodule update --init --recursive")
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/external/nlohmann_json/include
    ${NCURSES_INCLUDE_DIRS}
    ${MPV_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
)

# Source files
set(SOURCES
    src/main.cpp
    src/ui/application.cpp
    src/ui/search_view.cpp
    src/ui/queue_view.cpp
    src/ui/player_view.cpp
    src/audio/mpv_player.cpp
    src/youtube/extractor.cpp
    src/youtube/http_client.cpp
    src/core/state_manager.cpp
    src/core/config.cpp
    src/utils/logger.cpp
    src/utils/image_utils.cpp
    src/utils/stb_image.cpp
)

# Create executable
add_executable(moostream ${SOURCES})

# macOS: Add Homebrew library path
if(APPLE AND HOMEBREW_PREFIX)
    target_link_directories(moostream PRIVATE "${HOMEBREW_PREFIX}/opt/ncurses/lib" "${HOMEBREW_PREFIX}/lib")
endif()

# Link libraries
target_link_libraries(moostream
    PRIVATE
    Threads::Threads
    ${NCURSES_LIBRARIES}
    ${MPV_LIBRARIES}
    ${CURL_LIBRARIES}
    imtui-ncurses
)

# Compiler warnings
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(moostream PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()

# Install target
install(TARGETS moostream DESTINATION bin)

# Tests
if(BUILD_TESTS)
    # Test executables
    add_executable(test_unicode test/test_unicode.cpp)
    target_link_libraries(test_unicode
        PRIVATE
        Threads::Threads
        ${NCURSES_LIBRARIES}
        ${MPV_LIBRARIES}
        ${CURL_LIBRARIES}
        imtui-ncurses
    )

    add_executable(test_search test/test_search.cpp)
    target_link_libraries(test_search
        PRIVATE
        Threads::Threads
        ${NCURSES_LIBRARIES}
        ${MPV_LIBRARIES}
        ${CURL_LIBRARIES}
        imtui-ncurses
    )
endif()
