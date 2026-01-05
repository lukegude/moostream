# PROJECT KNOWLEDGE BASE

**Generated:** 2026-01-04  
**Commit:** (none)  
**Branch:** main  

## OVERVIEW
C++ terminal-based YouTube audio player using imtui, libmpv, and yt-dlp.  

## STRUCTURE
```
./
├── youtube-tui/
│   ├── src/              # Source code
│   │   ├── main.cpp      # Entry point
│   │   ├── ui/           # UI components (imtui)
│   │   ├── core/         # Core logic (state, config)
│   │   ├── youtube/      # YouTube integration (yt-dlp)
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
| Add YouTube feature | youtube-tui/src/youtube/ | Modify yt-dlp integration |
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
- Audio streaming via libmpv with yt-dlp  
- Mixed header organization  

## COMMANDS
```bash
cd youtube-tui && mkdir -p build && cd build && cmake .. && make -j$(nproc)
./youtube-tui
```

## NOTES
Requires: CMake >=3.15, C++17 compiler, ncurses, libmpv, yt-dlp  
Build artifacts in build/ directory  
Logs to /tmp/youtube-tui.log