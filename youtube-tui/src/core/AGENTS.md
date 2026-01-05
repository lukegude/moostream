# PROJECT KNOWLEDGE BASE

**Generated:** 2026-01-04  
**Commit:** (none)  
**Branch:** main  

## OVERVIEW
Core logic for YouTube TUI player state, configuration, and playlist management.

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Add config setting | config.h/.cpp | Extend Config class with new getters/setters |
| Modify queue logic | state_manager.h/.cpp | Update StateManager queue operations |
| Add playlist feature | state_manager.h/.cpp | Extend playlist CRUD operations |
| Change track metadata | include/track.h | Update Track/Playlist struct fields |
| Add playback state | state_manager.h/.cpp | Extend state tracking (shuffle/repeat) |

## CODE MAP
Config: Singleton configuration manager with file persistence (~/.config/youtube-tui/config)  
StateManager: Thread-safe queue, playback state, and playlist management with mutex protection  
Track: YouTube video metadata structure (id, title, channel, duration, etc.)  
Playlist: User-defined track collections with persistence and timestamps  

## CONVENTIONS
- Singleton pattern for Config class (thread-safe instance access)
- Mutex-protected StateManager for concurrent UI/audio access
- Headers in include/ for public interfaces (Track/Playlist structs)
- Co-located headers for private implementation details

## ANTI-PATTERNS (CORE MODULE)
None identified. Clean separation of config vs. state vs. playlist management.

## UNIQUE STYLES
- Config uses simple key=value format for persistence
- StateManager uses std::deque for efficient queue operations
- Playlist persistence uses custom JSON serialization with string escaping
- Track/Playlist structs provide is_valid() methods for data integrity

## NOTES
Requires: C++17 standard, filesystem support for config paths  
Thread-safe design supports concurrent UI updates and audio playback  
History limited to last 100 tracks, playlists persisted to ~/.config/youtube-tui/playlists.json