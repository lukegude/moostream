# PROJECT KNOWLEDGE BASE

**Generated:** 2026-01-04  
**Commit:** (none)  
**Branch:** main  

## OVERVIEW
libmpv-based audio player implementation

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Add audio feature | mpv_player.cpp/mpv_player.h | Extend MpvPlayer class |
| Fix playback issue | mpv_player.cpp | Modify mpv command/property calls |
| Add player interface | include/player_interface.h | Update IPlayer abstract class |
| Change audio backend | Replace MpvPlayer implementation | Keep IPlayer interface |
| Handle mpv events | mpv_player.cpp::process_events() | Add event handling logic |

## CODE MAP
Interface: IPlayer (include/player_interface.h)  
Implementation: MpvPlayer (mpv_player.cpp/mpv_player.h)  
Key methods: load(), play(), pause(), get_position(), update()  

## CONVENTIONS
- libmpv C API used directly (no C++ wrapper)  
- Volume normalized to 0.0-1.0 (converted to mpv's 0-100)  
- Event-driven updates via update() method  
- yt-dlp integration via mpv's built-in ytdl option  

## ANTI-PATTERNS (THIS PROJECT)
None identified in audio codebase.

## UNIQUE STYLES
- Direct mpv_handle* usage in class member  
- Callback-based error/end handling  
- State tracking separate from mpv properties  

## NOTES
Requires libmpv with yt-dlp support for YouTube streaming. Audio-only playback configured via mpv options.</content>
<parameter name="filePath">youtube-tui/src/audio/AGENTS.md