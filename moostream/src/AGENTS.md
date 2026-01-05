# PROJECT KNOWLEDGE BASE

**Generated:** 2026-01-04  
**Commit:** (none)  
**Branch:** main  

## OVERVIEW
C++ source code for terminal-based YouTube audio player using imtui, libmpv, and yt-dlp.

## STRUCTURE
```
./
├── main.cpp              # Entry point
├── ui/                   # UI components (imtui)
│   ├── application.cpp   # Main app logic and event loop
│   ├── application.h
│   ├── player_view.cpp   # Player controls view
│   ├── player_view.h
│   ├── queue_view.cpp    # Queue management view
│   ├── queue_view.h
│   ├── search_view.cpp   # YouTube search view
│   └── search_view.h
├── core/                 # Core logic and state
│   ├── config.cpp        # Configuration management
│   ├── config.h
│   ├── state_manager.cpp # Queue and playback state
│   └── state_manager.h
├── youtube/              # YouTube integration
│   ├── extractor.cpp     # yt-dlp wrapper for search/extraction
│   └── extractor.h
├── audio/                # Audio playback
│   ├── mpv_player.cpp    # libmpv wrapper
│   └── mpv_player.h
└── utils/                # Utilities
    ├── logger.cpp        # Logging utility
    └── logger.h
```

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Add UI feature | ui/ | Modify imtui views or application logic |
| Add audio feature | audio/ | Extend MpvPlayer interface |
| Add YouTube feature | youtube/ | Modify Extractor for new yt-dlp features |
| Fix core logic | core/ | Update StateManager or Config |
| Add utility | utils/ | Extend Logger or add new utilities |
| Update main loop | main.cpp + ui/application.cpp | Application initialization and event handling |

## CODE MAP
Entry point: main.cpp  
Key classes: Application (ui), StateManager (core), MpvPlayer (audio), Extractor (youtube)  

## CONVENTIONS
- Headers co-located with sources in module directories  
- Public interfaces in include/ directory (Track, Playlist, IPlayer)  
- Namespace: ytui  
- Naming: PascalCase classes, camelCase methods, snake_case members with trailing _  
- C++17 features (std::filesystem not used, preferring manual string handling)  
- RAII for resource management (unique_ptr for major components)  
- Thread safety: Mutex-protected classes (StateManager, Logger)  
- Async operations use std::future for YouTube operations  
- Enum classes for state types (PlaybackState, LogLevel)  

## ANTI-PATTERNS (THIS PROJECT)
- JSON parsing is placeholder (TODO: implement proper parsing in extractor)
- Monolithic UI: All rendering logic in Application::render() instead of modular view classes
- Placeholder view files without implementation (player_view.*, queue_view.*, search_view.*)
- UI tightly coupled to business logic (Application handles both rendering and commands)
<parameter name="filePath">moostream/src/AGENTS.md