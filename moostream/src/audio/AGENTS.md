# PROJECT KNOWLEDGE BASE

**Generated:** 2026-01-04  
**Commit:** (none)  
**Branch:** main  

## OVERVIEW
libmpv-based audio player implementation

## STRUCTURE
```
./
├── mpv_player.h          # MpvPlayer class declaration
├── mpv_player.cpp        # libmpv integration implementation
└── ../include/
    └── player_interface.h # IPlayer abstract interface
```

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Add audio feature | mpv_player.cpp/mpv_player.h | Extend MpvPlayer class methods |
| Fix playback issue | mpv_player.cpp::process_events() | Modify mpv event handling |
| Add player interface | include/player_interface.h | Update IPlayer abstract methods |
| Change audio backend | Replace MpvPlayer implementation | Keep IPlayer interface unchanged |
| Handle mpv events | mpv_player.cpp::process_events() | Add new event handling in switch |
| Configure mpv options | mpv_player.cpp::MpvPlayer() | Modify mpv_set_option_string calls |
| Add property observation | mpv_player.cpp::MpvPlayer() | Use mpv_observe_property calls |

## CODE MAP
Interface: IPlayer (include/player_interface.h)  
Implementation: MpvPlayer (mpv_player.cpp/mpv_player.h)  
Key methods: load(), play(), pause(), stop(), seek(), set_volume(), get_volume(), get_position(), get_duration(), set_end_callback(), set_error_callback(), update()  

## CONVENTIONS
- libmpv C API used directly (no C++ wrapper)  
- Volume normalized to 0.0-1.0 (converted to mpv's 0-100)  
- Event-driven updates via update() method  
- ytdl integration via mpv's built-in ytdl option  
- Audio-only playback (video=no, terminal=no)  
- Error-only logging from mpv (msg-level=all=error)  

## ANTI-PATTERNS (THIS PROJECT)
None identified in audio codebase.

## UNIQUE STYLES
- Direct mpv_handle* usage in class member  
- Callback-based error/end handling  
- State tracking separate from mpv properties  

## NOTES
Requires libmpv for YouTube streaming. Audio-only playback configured via mpv options. Playback state tracked separately from mpv properties for UI responsiveness.</content>
<parameter name="filePath">moostream/src/audio/AGENTS.md