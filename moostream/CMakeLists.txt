cmake_minimum_required(VERSION 3.15)
project(moostream VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(BUILD_TESTS "Build tests" OFF)

# Find required packages
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)

# Find ncurses
pkg_check_modules(NCURSES REQUIRED ncurses)

# Find mpv
pkg_check_modules(MPV REQUIRED mpv)

# Add imtui as subdirectory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/external/imtui/CMakeLists.txt")
    add_subdirectory(external/imtui)
else()
    message(WARNING "imtui not found in external/imtui. Please run: git submodule update --init --recursive")
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${NCURSES_INCLUDE_DIRS}
    ${MPV_INCLUDE_DIRS}
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
    src/core/state_manager.cpp
    src/core/config.cpp
    src/utils/logger.cpp
)

# Create executable
add_executable(moostream ${SOURCES})

# Link libraries
target_link_libraries(moostream
    PRIVATE
    Threads::Threads
    ${NCURSES_LIBRARIES}
    ${MPV_LIBRARIES}
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
