# PROJECT KNOWLEDGE BASE

**Generated:** 2026-01-06  
**Commit:** (none)  
**Branch:** main  

## OVERVIEW
Terminal UI components using imtui for YouTube audio player.

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Add UI component | moostream/src/ui/application.cpp | Modify Application::render_* methods |
| Fix rendering logic | moostream/src/ui/application.cpp | Update render() or specific render_* functions |
| Add keyboard shortcut | moostream/src/ui/application.cpp | Extend handle_input() method |
| Change UI theme | moostream/src/ui/application.cpp | Modify setup_theme() method |
| Add toast notification | moostream/src/ui/application.cpp | Use show_toast() method |
| Implement view class | moostream/src/ui/ | Replace placeholder in player_view.*, queue_view.*, search_view.* |

## CONVENTIONS
- ImTui immediate-mode rendering with ncurses backend
- Monolithic Application class with all UI logic
- Focus area enum for navigation (Search, Queue, Player)
- Toast notifications for user feedback
- Async search with std::future and mutex protection

## ANTI-PATTERNS
- Monolithic UI: All rendering in Application::render() instead of modular views
- Placeholder view files without implementation
- UI-business coupling: Application handles rendering and commands</content>
<parameter name="filePath">moostream/src/ui/AGENTS.md