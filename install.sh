#!/bin/bash

set -e

echo "Moostream - Dependency Installer"
echo "================================="
echo ""

# Detect OS
detect_os() {
    if [[ "$OSTYPE" == "darwin"* ]]; then
        echo "macos"
    elif [[ -f /etc/os-release ]]; then
        . /etc/os-release
        case "$ID" in
            ubuntu|debian|linuxmint|pop)
                echo "debian"
                ;;
            arch|manjaro|endeavouros)
                echo "arch"
                ;;
            fedora)
                echo "fedora"
                ;;
            opensuse*|suse)
                echo "suse"
                ;;
            *)
                # Check for derivative distros
                if [[ "$ID_LIKE" == *"debian"* ]]; then
                    echo "debian"
                elif [[ "$ID_LIKE" == *"arch"* ]]; then
                    echo "arch"
                elif [[ "$ID_LIKE" == *"fedora"* ]] || [[ "$ID_LIKE" == *"rhel"* ]]; then
                    echo "fedora"
                else
                    echo "unknown"
                fi
                ;;
        esac
    else
        echo "unknown"
    fi
}

OS=$(detect_os)
echo "Detected OS: $OS"
echo ""

install_macos() {
    echo "Installing dependencies for macOS..."
    
    if ! command -v brew &> /dev/null; then
        echo "Homebrew not found."
        exit 0
    fi
    
    echo "Installing packages via Homebrew..."
    brew install cmake ncurses mpv curl nlohmann-json pkg-config
    
    echo ""
    echo "macOS dependencies installed successfully!"
}

install_debian() {
    echo "Installing dependencies for Debian/Ubuntu..."
    
    sudo apt update
    sudo apt install -y \
        build-essential \
        cmake \
        pkg-config \
        libncurses-dev \
        libmpv-dev \
        libcurl4-openssl-dev \
        nlohmann-json3-dev \
        git
    
    echo ""
    echo "Debian/Ubuntu dependencies installed successfully!"
}

install_arch() {
    echo "Installing dependencies for Arch Linux..."
    
    sudo pacman -Syu --noconfirm
    sudo pacman -S --noconfirm --needed \
        base-devel \
        cmake \
        pkg-config \
        ncurses \
        mpv \
        curl \
        nlohmann-json \
        git
    
    echo ""
    echo "Arch Linux dependencies installed successfully!"
}

install_fedora() {
    echo "Installing dependencies for Fedora..."
    
    sudo dnf install -y \
        gcc-c++ \
        cmake \
        pkg-config \
        ncurses-devel \
        mpv-libs-devel \
        libcurl-devel \
        json-devel \
        git
    
    echo ""
    echo "Fedora dependencies installed successfully!"
}

install_suse() {
    echo "Installing dependencies for openSUSE..."
    
    sudo zypper install -y \
        gcc-c++ \
        cmake \
        pkg-config \
        ncurses-devel \
        mpv-devel \
        libcurl-devel \
        nlohmann_json-devel \
        git
    
    echo ""
    echo "openSUSE dependencies installed successfully!"
}

# Install based on detected OS
case "$OS" in
    macos)
        install_macos
        ;;
    debian)
        install_debian
        ;;
    arch)
        install_arch
        ;;
    fedora)
        install_fedora
        ;;
    suse)
        install_suse
        ;;
    unknown)
        echo "ERROR: Could not detect your operating system."
        echo ""
        echo "Please install the following dependencies manually:"
        echo "  - cmake (>= 3.15)"
        echo "  - C++17 compiler (gcc/clang)"
        echo "  - ncurses development files"
        echo "  - libmpv development files"
        echo "  - libcurl development files"
        echo "  - nlohmann-json development files"
        echo "  - pkg-config"
        echo "  - git"
        echo ""
        exit 1
        ;;
esac

echo ""
echo "================================="
echo "Dependencies installed!"
echo ""

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

echo "Building moostream..."
echo ""
"$SCRIPT_DIR/setup.sh"

echo ""
echo "Installing moostream system-wide..."
cd "$SCRIPT_DIR/build"
sudo make install

echo ""
echo "================================="
echo "Installation complete!"
echo ""
echo "Next steps:"
echo "  1. Run: moostream auth    (set up YouTube OAuth)"
echo "  2. Run: moostream         (start the application)"
echo ""
