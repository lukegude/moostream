# PROJECT KNOWLEDGE BASE

**Generated:** 2026-01-04  
**Commit:** (none)  
**Branch:** main  

## OVERVIEW
C++ logging utilities for the YouTube TUI application.

## STRUCTURE
```
utils/
├── logger.h      # Logger class interface
└── logger.cpp    # Logger implementation
```

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Add logging | logger.h/logger.cpp | Use Logger class methods |
| Change log level | Logger::set_level() | Static method |
| Initialize logging | Logger::init() | Call at startup |
| Add log message | Logger::debug/info/warning/error() | Static methods |

## CODE MAP
Logger class: Thread-safe logging with file and stderr output  
Key methods: init(), shutdown(), debug(), info(), warning(), error(), set_level()  

## CONVENTIONS
- Static Logger class for global access  
- Mutex-protected for thread safety  
- Log levels: Debug, Info, Warning, Error  
- Timestamp format: YYYY-MM-DD HH:MM:SS.mmm  

## ANTI-PATTERNS (THIS DIRECTORY)
None identified in utils codebase.

## UNIQUE STYLES
- Immediate logging to file if open  
- Error messages also to stderr  
- Custom timestamp with milliseconds  

## COMMANDS
```bash
# Build with logging
cd moostream/build && make -j$(nproc)
# Check logs
tail -f /tmp/moostream.log
```

## NOTES
Logs to file specified in init(), defaults to /tmp/moostream.log  
Thread-safe using std::mutex  
C++17 standard, no external dependencies