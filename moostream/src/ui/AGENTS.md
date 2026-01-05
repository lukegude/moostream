# PROJECT KNOWLEDGE BASE

**Generated:** 2026-01-04  
**Commit:** (none)  
**Branch:** main  

## OVERVIEW
UI components for terminal-based YouTube audio player using imtui.

## WHERE TO LOOK
| Task | Location | Notes |
|------|----------|-------|
| Add UI feature | moostream/src/ui/application.cpp | Modify Application class rendering methods |
| Fix UI logic | moostream/src/ui/application.cpp | Update render_* functions or event handling |
| Add new view | moostream/src/ui/ | Implement placeholder view files (player_view, queue_view, search_view) |
| Change UI framework | moostream/src/ui/application.cpp | Update ImTui initialization and rendering calls |
| Add keyboard shortcuts | moostream/src/ui/application.cpp | Extend handle_input() method |

## CODE MAP
Entry point: moostream/src/main.cpp (instantiates Application)  
Key class: Application (ui/application.h) - main UI controller  
Rendering: application.cpp render() method with render_* functions  
Views: Currently monolithic; placeholders for modular views  

## CONVENTIONS
- C++17 standard with namespace ytui  
- PascalCase classes (Application), camelCase methods (render_menu_bar), snake_case members with _ (player_)  
- ImGui immediate-mode rendering with ncurses backend  
- Monolithic Application class containing all UI logic  
- Lambda callbacks for event handling  
- std::unique_ptr for resource management  

## ANTI-PATTERNS (THIS PROJECT)
- Monolithic UI: All rendering logic in Application::render() instead of modular view classes  
- Placeholder files without implementation (player_view.*, queue_view.*, search_view.*)  
- UI tightly coupled to business logic (Application handles both rendering and commands)</content>
<parameter name="filePath">moostream/src/ui/AGENTS.md