# PROJECT KNOWLEDGE BASE

**Generated:** 2026-01-04  
**Commit:** (none)  
**Branch:** main  

## OVERVIEW
C++ terminal-based YouTube audio player using imtui, libmpv, and YouTube Data API v3.  

## STRUCTURE
```
./
├── youtube-tui/
│   ├── src/              # Source code
│   │   ├── main.cpp      # Entry point
│   │   ├── ui/           # UI components (imtui)
│   │   ├── core/         # Core logic (state, config)
│   │   ├── youtube/      # YouTube integration (YouTube Data API v3)
│   │   ├── audio/        # Audio playback (libmpv)
│   │   └── utils/        # Utilities (logger)
│   ├── include/          # Public headers
│   ├── external/         # Git submodules (imtui)
│   ├── CMakeLists.txt    # Build configuration
│   ├── Makefile          # Build wrapper
│   └── README.md         # Documentation
└── build/                # Build artifacts (generated)
```

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Add UI feature | youtube-tui/src/ui/ | Modify imtui-based views |
| Add audio feature | youtube-tui/src/audio/ | Extend mpv_player |
| Add YouTube feature | youtube-tui/src/youtube/ | Modify YouTube API integration |
| Fix core logic | youtube-tui/src/core/ | Update state or config |
| Add utility | youtube-tui/src/utils/ | Extend logger or add new |
| Build project | youtube-tui/ | Run make in build dir |
| Configure build | youtube-tui/CMakeLists.txt | Modify CMake options |

## CODE MAP
Entry point: youtube-tui/src/main.cpp  
Key classes: Application (ui), StateManager (core), MpvPlayer (audio), Extractor (youtube)  

## CONVENTIONS
- C++17 standard  
- CMake build system with pkg-config  
- Headers in include/ for public interfaces, co-located for private  
- Git submodules for external deps  
- No tests currently (BUILD_TESTS=OFF)  

## ANTI-PATTERNS (THIS PROJECT)
None identified in main codebase.  

## UNIQUE STYLES
- Terminal UI with imtui (immediate mode)  
- Audio streaming via libmpv  
- Mixed header organization  

## COMMANDS
```bash
cd youtube-tui && mkdir -p build && cd build && cmake .. && make -j$(nproc)
./youtube-tui
```

## UTILS DIRECTORY

### OVERVIEW
C++ logging utilities providing thread-safe logging with file and stderr output for the YouTube TUI application.

### WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Add logging | youtube-tui/src/utils/ | Use Logger class static methods |
| Change log level | Logger::set_level() | Static method for global level |
| Initialize logging | Logger::init() | Call at application startup |
| Add log message | Logger::debug/info/warning/error() | Static methods with auto-timestamp |

### CODE MAP
Logger class: Thread-safe singleton with file/stderr output  
Key methods: init(), shutdown(), debug(), info(), warning(), error(), set_level()  

### CONVENTIONS
- Static Logger class for global access (no instances needed)  
- Mutex-protected for thread safety  
- Log levels: Debug, Info, Warning, Error  
- Timestamp format: YYYY-MM-DD HH:MM:SS.mmm with milliseconds  

### ANTI-PATTERNS (UTILS)
None identified in utils codebase.

### UNIQUE STYLES
- Immediate file logging when initialized  
- Error messages duplicated to stderr  
- Custom timestamp including milliseconds  

### COMMANDS
```bash
# Check application logs
tail -f /tmp/youtube-tui.log
```

### NOTES
Logs to file specified in init(), defaults to /tmp/youtube-tui.log
Thread-safe using std::mutex, C++17 standard, no external dependencies

## UI DIRECTORY

### OVERVIEW
Terminal UI components using imtui immediate-mode GUI framework with ncurses backend. Provides search, queue management, and playback controls for the YouTube audio player.

### WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Add UI component | youtube-tui/src/ui/ | Create new view classes (follow player_view pattern) |
| Modify playback controls | youtube-tui/src/ui/player_view.* | Update player UI and controls |
| Change queue display | youtube-tui/src/ui/queue_view.* | Modify queue rendering and interactions |
| Update search interface | youtube-tui/src/ui/search_view.* | Change search input and results |
| Fix main UI logic | youtube-tui/src/ui/application.* | Update Application class event handling |
| Add keyboard shortcuts | youtube-tui/src/ui/application.cpp | Extend handle_input() method |

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
Logs to /tmp/youtube-tui.log