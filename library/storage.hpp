#pragma once
#include "model.hpp"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <system_error>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace library {
namespace fs = std::filesystem;

inline std::string serialise(const Library& model) {
    std::ostringstream out;
    out << "STUDY_LIBRARY 1\n";
    for (const auto& item : model.members.all())
        out << "M " << item.first << ' ' << std::quoted(item.second.name) << '\n';
    for (const auto& item : model.books.all()) {
        const auto& book = item.second;
        out << "B " << book.id << ' ' << book.borrower << ' ' << std::quoted(book.title) << ' ' << std::quoted(book.author) << '\n';
    }
    return out.str();
}

inline Library parse(std::istream& input) {
    std::string line;
    check(static_cast<bool>(std::getline(input, line)) && line == "STUDY_LIBRARY 1", "Unsupported or damaged library file header.");
    Library model;
    int lineNumber = 1;
    while (std::getline(input, line)) {
        ++lineNumber;
        check(line.size() <= 1024, "Record too long at line " + std::to_string(lineNumber));
        if (line.empty()) continue;
        std::istringstream row(line);
        char kind, extra;
        check(static_cast<bool>(row >> kind), "Invalid record.");
        if (kind == 'M') {
            Member member{};
            check(static_cast<bool>(row >> member.id >> std::quoted(member.name)), "Invalid member at line " + std::to_string(lineNumber));
            check(validText(member.name) && model.members.insert(member), "Invalid or duplicate member at line " + std::to_string(lineNumber));
        } else if (kind == 'B') {
            Book book{};
            check(static_cast<bool>(row >> book.id >> book.borrower >> std::quoted(book.title) >> std::quoted(book.author)), "Invalid book at line " + std::to_string(lineNumber));
            check(book.borrower >= 0 && validText(book.title) && validText(book.author) && model.books.insert(book),
                  "Invalid or duplicate book at line " + std::to_string(lineNumber));
        } else check(false, "Unknown record type at line " + std::to_string(lineNumber));
        check(!(row >> extra), "Unexpected data at line " + std::to_string(lineNumber));
    }
    check(!input.bad(), "Could not read the whole library file.");
    for (const auto& item : model.books.all())
        check(item.second.borrower == 0 || model.members.find(item.second.borrower), "Borrowed book references a missing member.");
    return model;
}

inline Library load(const fs::path& path) {
    check(fs::file_size(path) <= 8 * 1024 * 1024, "Library file exceeds 8 MiB.");
    std::ifstream input(path, std::ios::binary);
    check(input.is_open(), "Cannot open library data file.");
    return parse(input);
}

// Write a complete temporary sibling before replacing the previous save.
// The application commits its in-memory change only after this succeeds.
inline void save(const Library& model, const fs::path& path) {
    auto absolute = fs::absolute(path);
    fs::create_directories(absolute.parent_path());
    auto data = serialise(model);
#ifdef _WIN32
    wchar_t tempName[MAX_PATH]{};
    check(GetTempFileNameW(absolute.parent_path().c_str(), L"clb", 0, tempName) != 0, "Cannot create temporary save file.");
    fs::path temporary(tempName);
    HANDLE handle = CreateFileW(tempName, GENERIC_WRITE, 0, nullptr, TRUNCATE_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (handle == INVALID_HANDLE_VALUE) {
        std::error_code error;
        fs::remove(temporary, error);
        throw std::runtime_error("Cannot open temporary save file.");
    }
    DWORD written = 0;
    bool success = WriteFile(handle, data.data(), static_cast<DWORD>(data.size()), &written, nullptr) && written == data.size();
    if (success) success = FlushFileBuffers(handle) != 0;
    if (!CloseHandle(handle)) success = false;
    if (success) success = MoveFileExW(temporary.c_str(), absolute.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) != 0;
    if (!success) {
        std::error_code error;
        fs::remove(temporary, error);
        throw std::runtime_error("Save failed; previous catalogue was not replaced. Change cancelled.");
    }
#else
    // This coursework build targets Windows; do not silently promise atomic
    // replacement on platforms that have not been validated.
    (void)data;
    throw std::runtime_error("Persistent saving currently supports Windows only.");
#endif
}
} // namespace library
