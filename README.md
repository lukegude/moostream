# Moostream

A terminal-based YouTube audio player built with imtui and libmpv. Stream YouTube videos as audio in your terminal - like a radio!

## Features

- Play YouTube videos (audio only)
- Search YouTube directly from the terminal (via YouTube Data API v3)
- Queue management (add, remove, reorder)
- Playback controls (play, pause, stop, next, previous)
- Volume control
- Shuffle and repeat modes
- Persistent configuration
- OAuth 2.0 authentication for YouTube API
- Support for both regular videos and livestreams

## Dependencies

### Required

- CMake >= 3.15
- C++17 compatible compiler
- ncurses
- libmpv
- curl (for YouTube API requests)
- nlohmann_json

### Ubuntu/Debian

```bash
sudo apt update
sudo apt install build-essential cmake libncurses-dev libmpv-dev libcurl4-openssl-dev nlohmann-json3-dev pkg-config
```

### Arch Linux

```bash
sudo pacman -S base-devel cmake ncurses mpv nlohmann-json
```

### macOS

```bash
brew install cmake ncurses mpv nlohmann-json pkg-config
```

## Building

### Quick Start (Recommended)

```bash
git clone --recursive <your-repo-url> moostream
cd moostream
./install.sh  # Install deps, build, and install system-wide (macOS, Debian/Ubuntu, Arch, Fedora, openSUSE)
```

### Manual Build

1. Clone the repository with submodules:

```bash
git clone --recursive <your-repo-url> moostream
cd moostream
```

2. Initialize submodules (if not cloned with --recursive):

```bash
git submodule update --init --recursive
```

3. Build the project:

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

4. Authenticate with YouTube (one-time setup):

```bash
./moostream auth
```
This will display a URL and code. Complete the authorization in your browser, and the program will automatically detect when it's done.

5. Run the application:

```bash
./moostream
```

## Command Line Options

- `moostream` - Run the main application
- `moostream auth` - Set up YouTube OAuth authentication
- `moostream auth clear` - Clear stored authentication tokens

## Installation

After building, you can install system-wide:

```bash
cd build
sudo make install
```

This installs the binary to `/usr/local/bin/moostream`.

## Usage

### Basic Controls

- **Search**: Type in the search box and press Enter or click "Search"
- **Add to Queue**: Double-click a search result or select and click "Add to Queue"
- **Play**: Double-click a queue item or select and click play controls
- **Navigate**: Use arrow keys or mouse to navigate
- **Volume**: Use the volume slider

### Keyboard Shortcuts

- `q` - Quit application
- `s` - Toggle search view

### Configuration

Configuration is stored in `~/.config/moostream/config`:

```
volume=0.7
shuffle=false
repeat=false
youtube_client_id=YOUR_OAUTH_CLIENT_ID_HERE
youtube_access_token=auto_generated
youtube_refresh_token=auto_generated
```

### YouTube OAuth Setup

To use the YouTube search functionality, you need OAuth 2.0 credentials:

1. Go to the [Google Cloud Console](https://console.cloud.google.com/)
2. Create a new project or select an existing one
3. **Enable the YouTube Data API v3** (required!)
4. Configure the OAuth consent screen (set app name, support email, etc.)
5. Create OAuth 2.0 credentials (Client ID) - choose "TVs and Limited Input devices"
6. The JSON file contains your `client_id` and `client_secret`
7. Add both `client_id` and `client_secret` to your config file as `youtube_client_id` and `youtube_client_secret`
8. Run `./moostream auth` to complete the OAuth flow

**Troubleshooting:**
- Make sure YouTube Data API v3 is enabled in your project
- Make sure the OAuth consent screen is properly configured
- The client ID must be associated with the project that has the API enabled
- Check `/tmp/moostream.log` for error messages

Note: OAuth provides better security than API keys and allows higher rate limits.

### Logs

Logs are written to `/tmp/moostream.log` for debugging.

## Architecture

```
moostream/
├── src/
│   ├── main.cpp                # Entry point, CLI handling
│   ├── ui/                     # ImTui-based UI layer
│   │   ├── application.cpp/h   # Main application logic
│   │   ├── player_view.cpp/h   # Player controls view
│   │   ├── queue_view.cpp/h    # Queue management view
│   │   └── search_view.cpp/h   # YouTube search view
│   ├── audio/                  # Audio playback
│   │   └── mpv_player.cpp/h    # libmpv wrapper
│   ├── youtube/                # YouTube integration
│   │   ├── extractor.cpp/h     # YouTube Data API v3 integration
│   │   └── http_client.cpp/h   # HTTP client for API requests
│   ├── core/                   # Core functionality
│   │   ├── state_manager.cpp/h # Queue/playback state
│   │   └── config.cpp/h        # Configuration
│   └── utils/                  # Utilities
│       ├── logger.cpp/h        # Logging
│       ├── image_utils.cpp     # Image processing utilities
│       └── stb_image.cpp       # stb_image library
├── include/                    # Public headers
│   ├── track.h                 # Track/Playlist data structures
│   ├── player_interface.h      # IPlayer interface
│   ├── image_utils.h           # Image utilities header
│   └── stb_image.h             # stb_image header
├── external/                   # Git submodules
│   ├── imtui/                  # imtui library
│   └── nlohmann_json/          # JSON library
├── test/                       # Test files
├── CMakeLists.txt              # Build configuration
├── setup.sh                    # Setup and build script
└── README.md
```

## How It Works

1. **YouTube Integration**: Uses YouTube Data API v3 with OAuth 2.0 for search; libmpv's built-in yt-dlp support for stream extraction
2. **Audio Playback**: libmpv handles streaming and playback
3. **UI**: imtui provides an immediate-mode GUI in the terminal using ncurses
4. **State Management**: Centralized state manager handles queue and playback logic

## TODO/Future Features

- [ ] Playlist support (save/load playlists)
- [ ] Visualizer (ASCII spectrum analyzer)
- [ ] History view
- [ ] Better keyboard shortcuts
- [ ] Search filters (duration, upload date, etc.)
- [ ] Cache management
- [ ] Mouse support improvements
- [ ] Theming/color schemes

## Troubleshooting

### "Failed to create MPV instance"

Make sure libmpv is properly installed:

```bash
pkg-config --modversion mpv
```

### Build errors with imtui

Make sure you've initialized the submodules:

```bash
git submodule update --init --recursive
```

### Authentication issues

Clear tokens and re-authenticate:

```bash
./moostream auth clear
./moostream auth
```

## Contributing

Contributions are welcome! Areas that need work:

1. UI improvements and additional views
2. Better error handling
3. Testing

## License

[Choose your license - MIT, GPL, etc.]

## Credits

- [imtui](https://github.com/ggerganov/imtui) - Immediate mode TUI library
- [libmpv](https://mpv.io/) - Media player library
- [nlohmann/json](https://github.com/nlohmann/json) - JSON for Modern C++
