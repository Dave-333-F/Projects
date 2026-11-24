#pragma once
#include <filesystem>
#include <stdexcept>
#include <string>
#include <vector>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace coursework {
inline std::filesystem::path executableFolder() {
#ifdef _WIN32
    std::vector<wchar_t> buffer(32768);
    DWORD size = GetModuleFileNameW(nullptr, buffer.data(), static_cast<DWORD>(buffer.size()));
    if (size == 0 || size >= buffer.size()) throw std::runtime_error("Cannot determine application folder.");
    return std::filesystem::path(std::wstring(buffer.data(), size)).parent_path();
#else
    return std::filesystem::current_path();
#endif
}

inline std::filesystem::path dataPath(int argc, char* argv[]) {
    if (argc == 1) return executableFolder() / "data" / "library.txt";
    if (argc == 3 && std::string(argv[1]) == "--data") return std::filesystem::u8path(argv[2]);
    throw std::runtime_error("Usage: application.exe [--data path-to-library-file]");
}
} // namespace coursework
