#pragma once
#include "storage.hpp"
#include "../common/navigation.hpp"

namespace library {
inline int askId(const std::string& prompt) {
    auto text = ui::input(prompt);
    check(text.has_value(), "Input closed. Change cancelled.");
    auto id = ui::number(*text);
    check(id && *id > 0, "Enter a positive numeric ID. Change cancelled.");
    return *id;
}

inline std::string askText(const std::string& prompt) {
    auto text = ui::input(prompt);
    check(text.has_value(), "Input closed. Change cancelled.");
    check(validText(*text), "Enter 1-120 printable ASCII characters. Change cancelled.");
    return *text;
}

inline void run(const fs::path& dataFile) {
    Library model;
    std::string notice;
    try {
        if (fs::exists(dataFile)) { model = load(dataFile); notice = "Loaded saved catalogue."; }
        else { model = Library::sample(); notice = "Sample catalogue loaded. Your first change will save it."; }
    } catch (const std::exception& error) {
        ui::screen("LIBRARY", "DATA FILE ERROR", {error.what(), "", dataFile.u8string(), "", "Existing data was left untouched.", "Correct the file or choose another path with --data."}, {"Enter  Back"});
        ui::input("Press Enter > ");
        return;
    }
    int tab = 1, selectedAction = 0;
    const std::vector<ui::Action> actions = {
        {"Catalogue", "1"}, {"Members", "2"}, {"Help", "3"},
        {"Add book", "a"}, {"Add member", "m"}, {"Borrow book", "b"},
        {"Return book", "r"}, {"Remove book", "d"}, {"Remove member", "x"},
        {"Search", "s"}, {"Clear search", "n"}, {"Previous page", "["},
        {"Next page", "]"}, {"Save catalogue", "v"}, {"Back", "0"}
    };
    std::size_t page = 0;
    std::string query;
    while (true) {
        ui::Lines content;
        std::vector<ui::Lines> records;
        if (tab == 1) {
            content = {"[CATALOGUE]   Members   Help", "", query.empty() ? "All books" : "Search: " + query, ""};
            for (const auto& item : model.books.all()) {
                const auto& b = item.second;
                auto searchable = ui::lower(b.title + " " + b.author + " " + std::to_string(b.id));
                if (!query.empty() && searchable.find(ui::lower(query)) == std::string::npos) continue;
                std::string status = "Available";
                if (b.borrower) {
                    const auto* member = model.members.find(b.borrower);
                    status = "Borrowed by #" + std::to_string(b.borrower) + " " + member->name;
                }
                records.push_back({"#" + std::to_string(b.id) + "  " + b.title, "    " + b.author, "    " + status, ""});
            }
        } else if (tab == 2) {
            content = {"Catalogue   [MEMBERS]   Help", "", "Registered borrowers", ""};
            for (const auto& item : model.members.all()) {
                const auto& member = item.second;
                std::size_t count = 0;
                for (const auto& book : model.books.all()) if (book.second.borrower == member.id) ++count;
                records.push_back({"#" + std::to_string(member.id) + "  " + member.name, "    Borrowed books: " + std::to_string(count), ""});
            }
        } else {
            content = {"Catalogue   Members   [HELP]", "", "HOW TO USE", "Choose an action with Up/Down and Enter.", "You can also click an action on the right.", "Left/Right switches the active tab.", "", "Register a member before borrowing.", "Use book and member IDs when asked.", "Return outstanding books before removing records.", "Changes save automatically; Esc goes back.", "", "TEMPLATES IN THIS PROJECT", "Repository<Book> and Repository<Member> use", "the same insert/find/erase implementation.", "", "DATA FILE", dataFile.u8string(), "", "Single-user local storage; not a multi-user database."};
        }
        if (tab != 3) {
            constexpr std::size_t pageSize = 3;
            auto pages = std::max<std::size_t>(1, (records.size() + pageSize - 1) / pageSize);
            page = std::min(page, pages - 1);
            if (records.empty()) content.push_back("No matching records.");
            for (auto i = page * pageSize; i < std::min(records.size(), (page + 1) * pageSize); ++i)
                content.insert(content.end(), records[i].begin(), records[i].end());
            content.push_back("Page " + std::to_string(page + 1) + " / " + std::to_string(pages) + "    [ Previous    ] Next");
        }
        auto input = ui::actions("LIBRARY / TEMPLATES", content,
                                 {"AT A GLANCE", "Books: " + std::to_string(model.books.size()) + "  Members: " + std::to_string(model.members.size()),
                                  "On loan: " + std::to_string(model.borrowedCount())}, actions, selectedAction, tab, notice);
        if (!input || *input == "0") return;
        auto command = ui::lower(*input);
        if (command == "1" || command == "2" || command == "3") { tab = command[0] - '0'; page = 0; notice.clear(); continue; }
        if (command == "[") { if (page) --page; continue; }
        if (command == "]") { ++page; continue; }
        if (command == "n") { query.clear(); page = 0; notice = "Search cleared."; continue; }
        try {
            if (command == "s") { query = askText("Search title, author or ID > "); tab = 1; page = 0; notice.clear(); continue; }
            // Transaction: validate and save a copy before committing the change.
            Library candidate = model;
            std::string success;
            if (command == "a") {
                auto title = askText("Book title > ");
                auto author = askText("Author > ");
                auto id = candidate.addBook(title, author);
                success = "Added book #" + std::to_string(id) + ". Saved.";
                tab = 1;
            } else if (command == "m") {
                int id = candidate.addMember(askText("Member name > "));
                success = "Added member #" + std::to_string(id) + ". Saved.";
                tab = 2;
            } else if (command == "b") {
                int bookId = askId("Book ID > ");
                int memberId = askId("Member ID > ");
                candidate.borrow(bookId, memberId);
                success = "Book borrowed. Saved.";
            } else if (command == "r") {
                candidate.returnBook(askId("Book ID > "));
                success = "Book returned. Saved.";
            } else if (command == "d" || command == "x") {
                int id = askId(command == "d" ? "Book ID to remove > " : "Member ID to remove > ");
                auto approval = ui::input("Type REMOVE to confirm deletion > ");
                if (!approval || *approval != "REMOVE") { notice = "Removal cancelled."; continue; }
                if (command == "d") candidate.removeBook(id); else candidate.removeMember(id);
                success = "Record removed. Saved.";
            } else if (command == "v") success = "Catalogue saved.";
            else { notice = "Unknown command. See the right panel or Help tab."; continue; }
            save(candidate, dataFile);
            model = std::move(candidate);
            notice = success;
        } catch (const std::exception& error) { notice = error.what(); }
    }
}
} // namespace library
