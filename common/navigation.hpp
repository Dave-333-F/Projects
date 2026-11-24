#pragma once
#include "console.hpp"
#include <stdexcept>
#include <utility>

// Windows-specific input is kept here. Games still work with plain text pipes
// for testing, and their rule classes know nothing about keys or mouse events.
namespace ui {
enum class Key { up, down, left, right, enter, escape, character, click, resize, end };
struct Event { Key key; char character = 0; int x = 0, y = 0; };

class ConsoleInput {
#ifdef _WIN32
    HANDLE handle_ = GetStdHandle(STD_INPUT_HANDLE);
    DWORD previousMode_ = 0;
    CONSOLE_CURSOR_INFO previousCursor_{};
    bool changedCursor_ = false;
#endif
    bool active_ = false;
public:
    ConsoleInput() {
#ifdef _WIN32
        DWORD outputMode = 0;
        if (!GetConsoleMode(handle_, &previousMode_) ||
            !GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &outputMode)) return;
        DWORD mode = (previousMode_ | ENABLE_EXTENDED_FLAGS | ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT)
                   & ~(ENABLE_QUICK_EDIT_MODE | ENABLE_LINE_INPUT | ENABLE_ECHO_INPUT | ENABLE_VIRTUAL_TERMINAL_INPUT);
        if (!SetConsoleMode(handle_, mode)) throw std::runtime_error("Cannot enable console navigation.");
        active_ = true;
        changedCursor_ = GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &previousCursor_) != 0;
        if (changedCursor_) {
            auto hidden = previousCursor_;
            hidden.bVisible = FALSE;
            SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &hidden);
        }
#endif
    }
    ~ConsoleInput() {
#ifdef _WIN32
        if (active_) SetConsoleMode(handle_, previousMode_);
        if (changedCursor_) SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &previousCursor_);
#endif
    }
    ConsoleInput(const ConsoleInput&) = delete;
    ConsoleInput& operator=(const ConsoleInput&) = delete;
    bool active() const { return active_; }
    Event read() {
#ifdef _WIN32
        if (!active_) return {Key::end};
        INPUT_RECORD record{};
        DWORD received = 0;
        while (ReadConsoleInputW(handle_, &record, 1, &received) && received == 1) {
            if (record.EventType == WINDOW_BUFFER_SIZE_EVENT) return {Key::resize};
            if (record.EventType == MOUSE_EVENT) {
                const auto& mouse = record.Event.MouseEvent;
                if ((mouse.dwEventFlags == 0 || mouse.dwEventFlags == DOUBLE_CLICK) &&
                    (mouse.dwButtonState & FROM_LEFT_1ST_BUTTON_PRESSED))
                    return {Key::click, 0, mouse.dwMousePosition.X, mouse.dwMousePosition.Y};
            }
            if (record.EventType != KEY_EVENT || !record.Event.KeyEvent.bKeyDown) continue;
            const auto& key = record.Event.KeyEvent;
            switch (key.wVirtualKeyCode) {
                case VK_UP: return {Key::up};
                case VK_DOWN: case VK_TAB: return {Key::down};
                case VK_LEFT: return {Key::left};
                case VK_RIGHT: return {Key::right};
                case VK_RETURN: case VK_SPACE: return {Key::enter};
                case VK_ESCAPE: return {Key::escape};
            }
            if (key.uChar.UnicodeChar >= 33 && key.uChar.UnicodeChar <= 126)
                return {Key::character, static_cast<char>(key.uChar.UnicodeChar)};
        }
#endif
        return {Key::end};
    }
};

struct Hit { int line, column; };
inline std::optional<Hit> hitTest(int x, int y, bool sidebar = false) {
    for (const auto& area : lineAreas)
        if (area.sidebar == sidebar && y == area.y && x >= area.x && x < area.x + area.length)
            return Hit{area.sourceLine, area.sourceColumn + x - area.x};
    return std::nullopt;
}

inline int menuStep(int selected, int direction, int count) {
    return (selected + direction + count) % count;
}

// Last option is always Back/Exit. Numeric shortcuts remain available only for
// redirected text input and tests; the interactive screen shows no option numbers.
inline std::optional<int> choose(const std::string& title, const std::string& subtitle,
                                 const Lines& options, const Lines& status, int& selected,
                                 const std::string& notice = "") {
    ConsoleInput input;
    std::string message = notice;
    while (true) {
        Lines content;
        for (std::size_t i = 0; i < options.size(); ++i)
            content.push_back(std::string(static_cast<int>(i) == selected ? " > " : "   ") + options[i]);
        Lines sidebar = status;
        sidebar.insert(sidebar.end(), {"", "Up / Down  Move cursor", "Enter      Choose", "Esc        Back", "Mouse      Click an option"});
        screen(title, subtitle, content, sidebar, message);
        if (!input.active()) {
            auto text = ui::input("Choice (test input: 1..N, 0 back) > ");
            if (!text) return std::nullopt;
            auto value = number(*text);
            if (value && *value == 0) return static_cast<int>(options.size() - 1);
            if (value && *value > 0 && *value < static_cast<int>(options.size())) return *value - 1;
            message = "Choose a valid option.";
            continue;
        }
        auto event = input.read();
        if (event.key == Key::end) return std::nullopt;
        if (event.key == Key::escape) return static_cast<int>(options.size() - 1);
        if (event.key == Key::up) selected = menuStep(selected, -1, static_cast<int>(options.size()));
        if (event.key == Key::down) selected = menuStep(selected, 1, static_cast<int>(options.size()));
        if (event.key == Key::enter) return selected;
        if (event.key == Key::click) {
            auto hit = hitTest(event.x, event.y);
            if (hit && hit->line >= 0 && hit->line < static_cast<int>(options.size())) {
                selected = hit->line;
                return selected;
            }
        }
    }
}

using Action = std::pair<std::string, std::string>; // label, internal command
inline std::optional<std::string> actions(const std::string& title, const Lines& content,
                                         const Lines& status, const std::vector<Action>& options,
                                         int& selected, int tab, const std::string& notice) {
    ConsoleInput input;
    while (true) {
        Lines sidebar = status;
        sidebar.push_back("");
        int firstActionLine = static_cast<int>(sidebar.size());
        for (std::size_t i = 0; i < options.size(); ++i)
            sidebar.push_back(std::string(static_cast<int>(i) == selected ? "> " : "  ") + options[i].first);
        screen(title, "Up/Down: action  Enter: choose  Left/Right: tab  Esc: back", content, sidebar, notice);
        if (!input.active()) return ui::input();
        auto event = input.read();
        if (event.key == Key::end) return std::nullopt;
        if (event.key == Key::escape) return "0";
        if (event.key == Key::left) return std::to_string(menuStep(tab - 1, -1, 3) + 1);
        if (event.key == Key::right) return std::to_string(menuStep(tab - 1, 1, 3) + 1);
        if (event.key == Key::up) selected = menuStep(selected, -1, static_cast<int>(options.size()));
        if (event.key == Key::down) selected = menuStep(selected, 1, static_cast<int>(options.size()));
        if (event.key == Key::enter) return options[selected].second;
        if (event.key == Key::character) return std::string(1, event.character);
        if (event.key == Key::click) {
            auto hit = hitTest(event.x, event.y, true);
            if (hit) {
                int index = hit->line - firstActionLine;
                if (index >= 0 && index < static_cast<int>(options.size())) {
                    selected = index;
                    return options[selected].second;
                }
            }
        }
    }
}

inline void waitForEnter() {
    ConsoleInput input;
    if (!input.active()) { ui::input("Press Enter > "); return; }
    while (true) {
        auto event = input.read();
        if (event.key == Key::enter || event.key == Key::escape || event.key == Key::end) return;
    }
}
} // namespace ui
