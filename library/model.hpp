#pragma once
#include <algorithm>
#include <cctype>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>

namespace library {
struct Book { int id; std::string title; std::string author; int borrower = 0; };
struct Member { int id; std::string name; };

// One template manages two different types. T must expose a positive integer id.
template <typename T>
class Repository {
    std::map<int, T> records_;
public:
    bool insert(T item) {
        if (item.id <= 0 || records_.size() >= 10000) return false;
        return records_.emplace(item.id, std::move(item)).second;
    }
    T* find(int id) { auto it = records_.find(id); return it == records_.end() ? nullptr : &it->second; }
    const T* find(int id) const { auto it = records_.find(id); return it == records_.end() ? nullptr : &it->second; }
    bool erase(int id) { return records_.erase(id) != 0; }
    const std::map<int, T>& all() const { return records_; }
    std::size_t size() const { return records_.size(); }
    int nextId() const {
        if (records_.empty()) return 1;
        if (records_.rbegin()->first == std::numeric_limits<int>::max()) throw std::runtime_error("No more IDs available.");
        return records_.rbegin()->first + 1;
    }
};

inline void check(bool valid, const std::string& message) { if (!valid) throw std::runtime_error(message); }

inline bool validText(const std::string& text) {
    return !text.empty() && text.size() <= 120 &&
           std::any_of(text.begin(), text.end(), [](unsigned char c) { return c > 32; }) &&
           std::all_of(text.begin(), text.end(), [](unsigned char c) { return c >= 32 && c < 127; });
}

class Library {
public:
    Repository<Book> books;
    Repository<Member> members;

    int addBook(const std::string& title, const std::string& author) {
        check(validText(title) && validText(author), "Title and author must be 1-120 printable ASCII characters.");
        int id = books.nextId();
        check(books.insert(Book{id, title, author, 0}), "Cannot add book: catalogue capacity reached.");
        return id;
    }
    int addMember(const std::string& name) {
        check(validText(name), "Member name must be 1-120 printable ASCII characters.");
        int id = members.nextId();
        check(members.insert(Member{id, name}), "Cannot add member: capacity reached.");
        return id;
    }
    void borrow(int bookId, int memberId) {
        auto* book = books.find(bookId);
        check(book != nullptr, "Book ID was not found.");
        check(members.find(memberId) != nullptr, "Member ID was not found.");
        check(book->borrower == 0, "This book is already borrowed.");
        book->borrower = memberId;
    }
    void returnBook(int bookId) {
        auto* book = books.find(bookId);
        check(book != nullptr, "Book ID was not found.");
        check(book->borrower != 0, "This book is already available.");
        book->borrower = 0;
    }
    void removeBook(int id) {
        const auto* book = books.find(id);
        check(book != nullptr, "Book ID was not found.");
        check(book->borrower == 0, "Return this book before removing it.");
        books.erase(id);
    }
    void removeMember(int id) {
        check(members.find(id) != nullptr, "Member ID was not found.");
        for (const auto& entry : books.all()) check(entry.second.borrower != id, "This member still has a borrowed book.");
        members.erase(id);
    }
    std::size_t borrowedCount() const {
        std::size_t count = 0;
        for (const auto& entry : books.all()) if (entry.second.borrower) ++count;
        return count;
    }
    static Library sample() {
        Library result;
        result.addBook("C++ Programming", "Bjarne Stroustrup");
        result.addBook("Things Fall Apart", "Chinua Achebe");
        result.addBook("The River Between", "Ngugi wa Thiong'o");
        result.addBook("Algorithms in Practice", "Study Collection");
        result.addMember("Samuel");
        result.addMember("Alex");
        return result;
    }
};
} // namespace library
