# Moostream

A terminal-based YouTube audio player built with imtui and libmpv. Stream YouTube videos as audio in your terminal - like a radio!

## Features

- Play YouTube videos (audio only)
- Search YouTube directly from the terminal
- Queue management (add, remove, reorder)
- Playback controls (play, pause, stop, next, previous)
- Volume control
- Shuffle and repeat modes
- Persistent configuration
- Support for both regular videos and livestreams

## Dependencies

### Required

- CMake >= 3.15
- C++17 compatible compiler
- ncurses
- libmpv
- yt-dlp (for YouTube stream extraction)

### Ubuntu/Debian

```bash
sudo apt update
sudo apt install build-essential cmake libncurses-dev libmpv-dev yt-dlp pkg-config
```

### Arch Linux

```bash
sudo pacman -S base-devel cmake ncurses mpv yt-dlp
```

### macOS

```bash
brew install cmake ncurses mpv yt-dlp pkg-config
```

## Building

1. Clone the repository with submodules:

```bash
git clone --recursive <your-repo-url> moostream
cd moostream
```

2. Add imtui as a git submodule:

```bash
git submodule add https://github.com/ggerganov/imtui.git external/imtui
git submodule update --init --recursive
```

3. Build the project:

```bash
mkdir build
cd build
cmake ..
make -j$(nproc)
```

4. Run the application:

```bash
./moostream
```

## Installation

After building, you can install system-wide:

```bash
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
ytdlp_path=yt-dlp
```

### Logs

Logs are written to `/tmp/moostream.log` for debugging.

## Architecture

```
moostream/
├── src/
│   ├── main.cpp              # Entry point
│   ├── ui/                   # ImTui-based UI layer
│   │   └── application.cpp   # Main application logic
│   ├── audio/                # Audio playback
│   │   └── mpv_player.cpp    # libmpv wrapper
│   ├── youtube/              # YouTube integration
│   │   └── extractor.cpp     # yt-dlp wrapper
│   ├── core/                 # Core functionality
│   │   ├── state_manager.cpp # Queue/playback state
│   │   └── config.cpp        # Configuration
│   └── utils/                # Utilities
│       └── logger.cpp        # Logging
└── external/
    └── imtui/                # imtui library (submodule)
```

## How It Works

1. **YouTube Integration**: Uses yt-dlp to search and extract stream URLs
2. **Audio Playback**: libmpv handles streaming and playback with built-in yt-dlp support
3. **UI**: imtui provides an immediate-mode GUI in the terminal using ncurses
4. **State Management**: Centralized state manager handles queue and playback logic

## TODO/Future Features

- [ ] JSON parsing for proper YouTube metadata (currently placeholder)
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

### "Failed to execute yt-dlp"

Ensure yt-dlp is in your PATH:

```bash
which yt-dlp
yt-dlp --version
```

### Build errors with imtui

Make sure you've initialized the submodules:

```bash
git submodule update --init --recursive
```

## Contributing

Contributions are welcome! Areas that need work:

1. JSON parsing implementation for YouTube metadata
2. UI improvements and additional views
3. Better error handling
4. Testing

## License

[Choose your license - MIT, GPL, etc.]

## Credits

- [imtui](https://github.com/ggerganov/imtui) - Immediate mode TUI library
- [libmpv](https://mpv.io/) - Media player library
- [yt-dlp](https://github.com/yt-dlp/yt-dlp) - YouTube downloader
