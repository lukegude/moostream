#!/bin/bash

set -e

echo "YouTube TUI - Setup Script"
echo "=========================="
echo ""

# Check for required dependencies
echo "Checking dependencies..."

check_dependency() {
    if ! command -v "$1" &> /dev/null; then
        echo "ERROR: $1 is not installed"
        echo "Please install it using your package manager"
        exit 1
    else
        echo "✓ $1 found"
    fi
}

check_dependency cmake
check_dependency pkg-config
check_dependency yt-dlp

# Check for libraries
echo ""
echo "Checking for required libraries..."

if ! pkg-config --exists ncurses; then
    echo "ERROR: ncurses not found"
    exit 1
else
    echo "✓ ncurses found"
fi

if ! pkg-config --exists mpv; then
    echo "ERROR: libmpv not found"
    exit 1
else
    echo "✓ libmpv found"
fi

echo ""
echo "Initializing git submodules..."
if [ ! -d "external/imtui/.git" ]; then
    git submodule add https://github.com/ggerganov/imtui.git external/imtui 2>/dev/null || true
    git submodule update --init --recursive
    echo "✓ imtui submodule initialized"
else
    echo "✓ imtui already initialized"
fi

echo ""
echo "Creating build directory..."
mkdir -p build
cd build

echo "Running CMake..."
cmake ..

echo ""
echo "Building project..."
make -j$(nproc)

echo ""
echo "=========================="
echo "Build completed successfully!"
echo ""
echo "To run the application:"
echo "  cd build && ./youtube-tui"
echo ""
echo "To install system-wide:"
echo "  cd build && sudo make install"
echo ""
