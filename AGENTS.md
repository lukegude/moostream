# PROJECT KNOWLEDGE BASE

**Generated:** 2026-01-06  
**Commit:** (none)  
**Branch:** main  

## OVERVIEW
C++ terminal-based YouTube audio player using imtui and libmpv.  

## STRUCTURE
```
./
├── moostream/
│   ├── src/              # Source code
│   │   ├── main.cpp      # Entry point
│   │   ├── ui/           # UI components (imtui)
│   │   ├── core/         # Core logic (state, config)
│   │   ├── youtube/      # YouTube integration (YouTube API)
│   │   ├── audio/        # Audio playback (libmpv)
│   │   └── utils/        # Utilities (logger)
│   ├── include/          # Public headers
│   ├── external/         # Git submodules (imtui, nlohmann_json)
│   ├── CMakeLists.txt    # Build configuration
│   ├── setup.sh          # Custom build script
│   └── README.md         # Documentation
└── build/                # Build artifacts (generated)
```

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Add UI feature | moostream/src/ui/ | Modify imtui-based views |
| Add audio feature | moostream/src/audio/ | Extend mpv_player |
| Add YouTube feature | moostream/src/youtube/ | Modify YouTube API integration |
| Fix core logic | moostream/src/core/ | Update state or config |
| Add utility | moostream/src/utils/ | Extend logger or add new |
| Build project | moostream/ | Run setup.sh or make in build dir |
| Configure build | moostream/CMakeLists.txt | Modify CMake options |

## CODE MAP
Entry point: moostream/src/main.cpp  
Key classes: Application (ui), StateManager (core), MpvPlayer (audio), Extractor (youtube)  

## CONVENTIONS
- C++17 standard  
- CMake build system with pkg-config  
- Headers in include/ for public interfaces, co-located for private  
- Git submodules for external deps  
- No tests currently (BUILD_TESTS=OFF)  
- PascalCase classes, camelCase methods, snake_case members with trailing _  
- Namespace ytui  

## ANTI-PATTERNS (THIS PROJECT)
- JSON parsing is placeholder (TODO: implement proper parsing in extractor)
- Monolithic UI: All rendering logic in Application::render() instead of modular view classes
- Placeholder view files without implementation (player_view.*, queue_view.*, search_view.*)
- UI tightly coupled to business logic (Application handles both rendering and commands)

## UNIQUE STYLES
- Terminal UI with imtui (immediate mode)  
- Audio streaming via libmpv  
- Mixed header organization  
- Custom build script (setup.sh)  

## COMMANDS
```bash
cd moostream && ./setup.sh
# or
cd moostream && mkdir -p build && cd build && cmake .. && make -j$(nproc)
./moostream
```

## UTILS DIRECTORY

### OVERVIEW
C++ utilities providing logging and image processing for the YouTube TUI application.

### WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Add logging | logger.cpp/logger.h | Use Logger class static methods |
| Change log level | Logger::set_level() | Static method for global level |
| Initialize logging | Logger::init() | Call at application startup |
| Add log message | Logger::debug/info/warning/error() | Static methods with auto-timestamp |
| Add image processing | image_utils.cpp/image_utils.h | Extend image_to_8bit_ascii() or add new functions |
| Load images | stb_image.cpp | stb_image single-file library implementation |

### CONVENTIONS
- Static Logger class for global access (no instances needed)  
- Mutex-protected for thread safety  
- stb_image integration for image loading  

### ANTI-PATTERNS
- Placeholder image_to_8bit_ascii() implementation (returns static ASCII art)  
- Missing stb_image.h header file (implementation includes non-existent header)

## UI DIRECTORY

### OVERVIEW
Terminal UI components using imtui immediate-mode GUI framework with ncurses backend. Provides search, queue management, and playback controls for the YouTube audio player.

### WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Add UI component | moostream/src/ui/ | Create new view classes (follow player_view pattern) |
| Modify playback controls | moostream/src/ui/player_view.* | Update player UI and controls |
| Change queue display | moostream/src/ui/queue_view.* | Modify queue rendering and interactions |
| Update search interface | moostream/src/ui/search_view.* | Change search input and results |
| Fix main UI logic | moostream/src/ui/application.* | Update Application class event handling |
| Add keyboard shortcuts | moostream/src/ui/application.cpp | Extend handle_input() method |

### CONVENTIONS
- PascalCase classes (Application, PlayerView), camelCase methods (renderMenuBar)
- snake_case members with trailing underscore (current_volume_)
- Immediate-mode rendering with ImGui-style API calls
- Lambda callbacks for event handling
- std::unique_ptr for view resource management

### ANTI-PATTERNS
- Monolithic rendering: All UI logic centralized in Application::render()
- Placeholder view classes: Exist but not fully implemented
- UI-business coupling: Application handles both rendering and commands

## NOTES
Requires: CMake >=3.15, C++17 compiler, ncurses, libmpv, curl, nlohmann_json
Build artifacts in build/ directory
Logs to /tmp/moostream.log

## AUDIO DIRECTORY

### OVERVIEW
libmpv-based audio player implementation

### STRUCTURE
```
./
├── mpv_player.h          # MpvPlayer class declaration
├── mpv_player.cpp        # libmpv integration implementation
└── ../include/
    └── player_interface.h # IPlayer abstract interface
```

### WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Add audio feature | mpv_player.cpp/mpv_player.h | Extend MpvPlayer class methods |
| Fix playback issue | mpv_player.cpp::process_events() | Modify mpv event handling |
| Add player interface | include/player_interface.h | Update IPlayer abstract methods |
| Change audio backend | Replace MpvPlayer implementation | Keep IPlayer interface unchanged |
| Handle mpv events | mpv_player.cpp::process_events() | Add new event handling in switch |
| Configure mpv options | mpv_player.cpp::MpvPlayer() | Modify mpv_set_option_string calls |
| Add property observation | mpv_player.cpp::MpvPlayer() | Use mpv_observe_property calls |

### CODE MAP
Interface: IPlayer (include/player_interface.h)  
Implementation: MpvPlayer (mpv_player.cpp/mpv_player.h)  
Key methods: load(), play(), pause(), stop(), seek(), set_volume(), get_volume(), get_position(), get_duration(), set_end_callback(), set_error_callback(), update()  

### CONVENTIONS
- libmpv C API used directly (no C++ wrapper)  
- Volume normalized to 0.0-1.0 (converted to mpv's 0-100)  
- Event-driven updates via update() method  
- ytdl integration via mpv's built-in ytdl option  
- Audio-only playback (video=no, terminal=no)  
- Error-only logging from mpv (msg-level=all=error)  
- Buffering optimizations with cache settings (10s cache, readahead)  
- Property observation for pause and core-idle states  

### ANTI-PATTERNS (AUDIO)
None identified in audio codebase.

### UNIQUE STYLES
- Direct mpv_handle* usage in class member  
- Callback-based error/end handling  
- State tracking separate from mpv properties  

### NOTES
Requires libmpv for YouTube streaming. Audio-only playback configured via mpv options. Playback state tracked separately from mpv properties for UI responsiveness.

## CORE DIRECTORY

### OVERVIEW
Core logic for YouTube TUI player state, configuration, and playlist management.

### WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Add config setting | config.h/.cpp | Extend Config class with new getters/setters |
| Modify queue logic | state_manager.h/.cpp | Update StateManager queue operations |
| Add playlist feature | state_manager.h/.cpp | Extend playlist CRUD operations |
| Change track metadata | include/track.h | Update Track/Playlist struct fields |
| Add playback state | state_manager.h/.cpp | Extend state tracking (shuffle/repeat) |

### CONVENTIONS (if different)
- Singleton pattern for Config class (thread-safe instance access)
- Mutex-protected StateManager for concurrent UI/audio access
- Headers in include/ for public interfaces (Track/Playlist structs)
- Co-located headers for private implementation details

### ANTI-PATTERNS
None identified. Clean separation of config vs. state vs. playlist management.

## YOUTUBE DIRECTORY

### OVERVIEW
YouTube video search using YouTube Data API v3 with OAuth authentication. Stream URL extraction handled by libmpv.

### WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Add YouTube search | extractor.cpp | Modify search() method using YouTube Data API |
| Implement YouTube info extraction | extractor.cpp | Implement extract_info() method |
| Implement stream URL extraction | extractor.cpp | Implement get_stream_url() method |
| Add OAuth authentication | extractor.cpp | Modify authenticate_oauth() and refresh_access_token() methods |
| Add HTTP client features | http_client.h/.cpp | Extend HttpClient for new API endpoints |
| Improve Unicode handling | extractor.cpp | Enhance sanitize_title() and decode_unicode_escapes() |
| Add error handling | extractor.cpp/http_client.cpp | Improve exception handling in API calls |

### CODE MAP
Key classes: YouTubeExtractor (extractor.h/.cpp), HttpClient (http_client.h/.cpp)  
Data structures: Track (include/track.h), Playlist (include/track.h)  

### CONVENTIONS
Uses HttpClient for YouTube Data API calls; nlohmann/json for JSON parsing; OAuth 2.0 Device Authorization Grant for authentication; std::future for async operations; URL encoding with libcurl  

### ANTI-PATTERNS
extract_info() and get_stream_url() not implemented (return empty/error); OAuth authentication requires manual user interaction; API rate limits may affect search performance