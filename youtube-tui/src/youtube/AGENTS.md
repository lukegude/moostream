# PROJECT KNOWLEDGE BASE

**Generated:** 2026-01-04  
**Commit:** (none)  
**Branch:** main  

## OVERVIEW
YouTube video search and extraction using yt-dlp.

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Add YouTube search | extractor.cpp | Modify search() method |
| Add YouTube info extraction | extractor.cpp | Modify extract_info() method |
| Add stream URL extraction | extractor.cpp | Modify get_stream_url() method |
| Add async support | extractor.cpp | Use std::future methods |
| Add JSON parsing | extractor.cpp | Implement parse_track_json() and parse_search_results() |

## CODE MAP
Key class: YouTubeExtractor (extractor.h/.cpp)  

## CONVENTIONS
Uses popen() for yt-dlp execution; placeholder JSON parsing awaits nlohmann/json integration  

## ANTI-PATTERNS
Placeholder implementations in parse methods; synchronous subprocess calls may block UI threads