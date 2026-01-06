#!/bin/bash

set -e

echo "Moostream - Setup Script"
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

if ! pkg-config --exists libcurl; then
    echo "ERROR: libcurl not found"
    exit 1
else
    echo "✓ libcurl found"
fi

if ! pkg-config --exists nlohmann_json; then
    echo "ERROR: nlohmann_json not found"
    exit 1
else
    echo "✓ nlohmann_json found"
fi

echo ""
echo "Creating config directory..."
mkdir -p ~/.config/moostream

# Create basic config file if it doesn't exist
if [ ! -f ~/.config/moostream/config ]; then
    cat > ~/.config/moostream/config << 'EOF'
# Moostream Configuration
volume=0.7
shuffle=false
repeat=false

# YouTube OAuth Setup Required:
# 1. Go to https://console.cloud.google.com/
# 2. Create OAuth 2.0 'TVs and Limited Input devices' credentials
# 3. Replace YOUR_CLIENT_ID_HERE with your actual client ID
youtube_client_id=YOUR_CLIENT_ID_HERE

# These will be auto-generated after first authentication:
# youtube_access_token=
# youtube_refresh_token=
EOF
    echo "✓ Config template created at ~/.config/moostream/config"
else
    echo "✓ Config directory exists at ~/.config/moostream"
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
echo "Next steps:"
echo "1. Set up YouTube OAuth credentials:"
echo "   - Go to Google Cloud Console"
echo "   - Create OAuth 2.0 'TVs and Limited Input devices' credentials"
echo "   - Add client_id to ~/.config/moostream/config"
echo ""
echo "2. Run the application:"
echo "   cd build && ./moostream"
echo ""
echo "3. Optional: Install system-wide:"
echo "   cd build && sudo make install"
echo ""
