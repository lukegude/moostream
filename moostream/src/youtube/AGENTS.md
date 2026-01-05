# PROJECT KNOWLEDGE BASE

**Generated:** 2026-01-04  
**Commit:** (none)  
**Branch:** main  

## OVERVIEW
YouTube video search and extraction using yt-dlp. Supports synchronous and asynchronous operations, streaming search results, and Unicode handling.

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Add YouTube search | extractor.cpp | Modify search() or search_streaming() methods |
| Add YouTube info extraction | extractor.cpp | Modify extract_info() method |
| Add stream URL extraction | extractor.cpp | Modify get_stream_url() method |
| Add async support | extractor.cpp | Use std::future methods (already implemented) |
| Improve JSON parsing | extractor.cpp | Replace string-based parsing with nlohmann/json |
| Add Unicode support | extractor.cpp | Enhance decode_unicode_escapes() for full Unicode |
| Add error handling | extractor.cpp | Improve exception handling in execute_ytdlp() |

## CODE MAP
Key class: YouTubeExtractor (extractor.h/.cpp)  
Data structures: Track (include/track.h), Playlist (include/track.h)  

## CONVENTIONS
Uses popen() for yt-dlp subprocess execution; string-based JSON parsing for metadata extraction; Unicode escape decoding for titles; cookies file for authentication; std::future for async operations  

## ANTI-PATTERNS
Synchronous subprocess calls may block UI threads; string-based JSON parsing is fragile and incomplete; hardcoded cookies path; no proper error propagation from yt-dlp