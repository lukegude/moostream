# PROJECT KNOWLEDGE BASE

**Generated:** 2026-01-04  
**Commit:** (none)  
**Branch:** main  

## OVERVIEW
YouTube video search using YouTube Data API v3 with OAuth authentication. Stream URL extraction handled by libmpv.

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
Uses HttpClient for YouTube Data API calls; nlohmann/json for JSON parsing; OAuth 2.0 Device Authorization Grant for authentication; std::future for async operations  

## ANTI-PATTERNS
extract_info() and get_stream_url() not implemented (return empty/error); OAuth authentication requires manual user interaction; API rate limits may affect search performance