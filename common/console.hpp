#pragma once
#include <algorithm>
#include <cctype>
#include <iostream>
#include <optional>
#include <sstream>
#include <string>
#include <vector>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

// A small shared presentation layer: game rules never depend on console APIs.
namespace ui {
using Lines = std::vector<std::string>;

// Screen coordinates let the separate input layer recognise clicks on text.
struct LineArea { int x, y, length, sourceLine, sourceColumn; bool sidebar; };
inline std::vector<LineArea> lineAreas;

inline std::pair<int, int> cursorPosition() {
#ifdef _WIN32
    std::cout.flush();
    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
        return {info.dwCursorPosition.X, info.dwCursorPosition.Y};
#endif
    return {0, 0};
}

struct RenderLine { std::string text; int sourceLine, sourceColumn; };
inline std::vector<RenderLine> renderLines(const Lines& lines, std::size_t limit) {
    std::vector<RenderLine> result;
    for (std::size_t i = 0; i < lines.size(); ++i) {
        std::string text = lines[i];
        for (char& c : text) if (static_cast<unsigned char>(c) < 32 || c == 127) c = ' ';
        int column = 0;
        while (text.size() > limit) {
            auto split = text.rfind(' ', limit);
            if (split == std::string::npos || split == 0) split = limit;
            result.push_back({text.substr(0, split), static_cast<int>(i), column});
            text.erase(0, split);
            column += static_cast<int>(split);
            if (!text.empty() && text.front() == ' ') { text.erase(0, 1); ++column; }
        }
        result.push_back({text, static_cast<int>(i), column});
    }
    return result;
}

inline bool interactive() {
#ifdef _WIN32
    DWORD mode = 0;
    return GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &mode) != 0;
#else
    return false;
#endif
}

inline bool colour() {
#ifdef _WIN32
    static bool enabled = [] {
        DWORD mode = 0;
        HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);
        return GetConsoleMode(output, &mode) && SetConsoleMode(output, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
    }();
    return enabled;
#else
    return false;
#endif
}

inline int width() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &info))
        return info.srWindow.Right - info.srWindow.Left + 1;
#endif
    return 104;
}

inline std::string trim(std::string text) {
    auto whitespace = [](unsigned char c) { return std::isspace(c) != 0; };
    while (!text.empty() && whitespace(static_cast<unsigned char>(text.back()))) text.pop_back();
    auto begin = std::find_if_not(text.begin(), text.end(), whitespace);
    text.erase(text.begin(), begin);
    return text;
}

inline std::string lower(std::string text) {
    std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });
    return text;
}

inline std::optional<std::string> input(const std::string& prompt = "Your choice > ") {
    std::cout << '\n' << prompt << std::flush;
    std::string value;
    if (!std::getline(std::cin, value)) return std::nullopt;
    return trim(value);
}

inline std::optional<int> number(const std::string& text) {
    std::istringstream stream(text);
    int value;
    char extra;
    if (!(stream >> value) || (stream >> extra)) return std::nullopt;
    return value;
}

inline Lines wrap(const Lines& source, std::size_t limit) {
    Lines result;
    for (auto line : source) {
        // Prevent loaded records from injecting terminal control sequences.
        for (char& c : line) if (static_cast<unsigned char>(c) < 32 || c == 127) c = ' ';
        while (line.size() > limit) {
            auto split = line.rfind(' ', limit);
            if (split == std::string::npos || split == 0) split = limit;
            result.push_back(line.substr(0, split));
            line.erase(0, split);
            if (!line.empty() && line.front() == ' ') line.erase(0, 1);
        }
        result.push_back(line);
    }
    return result;
}

inline std::string padded(const std::string& line, std::size_t size) {
    return line + std::string(size > line.size() ? size - line.size() : 0, ' ');
}

inline void screen(const std::string& title, const std::string& subtitle,
                   const Lines& content, const Lines& sidebar, const std::string& message = "") {
    bool ansi = colour();
    lineAreas.clear();
    if (ansi && interactive()) std::cout << "\x1b[2J\x1b[H";
    if (ansi) std::cout << "\x1b[1;36m";
    std::cout << "\n  CPP / STUDY ARCADE     " << title << '\n';
    if (ansi) std::cout << "\x1b[0m";
    std::cout << "  " << subtitle << "\n\n";
    if (width() >= 100) {
        constexpr std::size_t leftWidth = 59, rightWidth = 30;
        auto left = renderLines(content, leftWidth);
        auto right = renderLines(sidebar, rightWidth);
        auto border = "+" + std::string(leftWidth + 2, '-') + "+" + std::string(rightWidth + 2, '-') + "+";
        std::cout << "  " << border << '\n';
        for (std::size_t i = 0; i < std::max(left.size(), right.size()); ++i) {
            auto position = cursorPosition();
            if (i < left.size()) lineAreas.push_back({4, position.second, static_cast<int>(leftWidth), left[i].sourceLine, left[i].sourceColumn, false});
            if (i < right.size()) lineAreas.push_back({66, position.second, static_cast<int>(rightWidth), right[i].sourceLine, right[i].sourceColumn, true});
            std::cout << "  | " << padded(i < left.size() ? left[i].text : "", leftWidth)
                      << " | " << padded(i < right.size() ? right[i].text : "", rightWidth) << " |\n";
        }
        std::cout << "  " << border << '\n';
    } else {
        auto limit = static_cast<std::size_t>(std::max(20, width() - 5));
        for (const auto& line : renderLines(content, limit)) {
            auto position = cursorPosition();
            lineAreas.push_back({2, position.second, static_cast<int>(limit), line.sourceLine, line.sourceColumn, false});
            std::cout << "  " << line.text << '\n';
        }
        std::cout << "\n  --- CONTROLS / STATUS ---\n";
        for (const auto& line : renderLines(sidebar, limit)) {
            auto position = cursorPosition();
            lineAreas.push_back({2, position.second, static_cast<int>(limit), line.sourceLine, line.sourceColumn, true});
            std::cout << "  " << line.text << '\n';
        }
    }
    if (!message.empty()) std::cout << "\n  " << message << '\n';
}
} // namespace ui
