#pragma once
#include <algorithm>
#include <array>
#include <cctype>
#include <stdexcept>
#include <string>

namespace hangman {
enum class Guess { correct, wrong, repeated, invalid, finished };

class Game {
    std::string word_;
    std::array<bool, 26> guessed_{};
    int mistakes_ = 0;
public:
    explicit Game(std::string word) : word_(std::move(word)) {
        if (word_.empty()) throw std::invalid_argument("Word cannot be empty");
        for (char& c : word_) {
            c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            if (c < 'A' || c > 'Z') throw std::invalid_argument("Words must contain letters A-Z only");
        }
    }
    int mistakes() const { return mistakes_; }
    int remaining() const { return 6 - mistakes_; }
    bool won() const { for (char c : word_) if (!guessed_[c - 'A']) return false; return true; }
    bool finished() const { return won() || mistakes_ == 6; }
    const std::string& answer() const { return word_; }
    std::string masked() const {
        std::string output;
        for (char c : word_) { if (!output.empty()) output += ' '; output += guessed_[c - 'A'] ? c : '_'; }
        return output;
    }
    std::string guessedLetters() const {
        std::string output;
        for (int i = 0; i < 26; ++i) if (guessed_[i]) { if (!output.empty()) output += ' '; output += static_cast<char>('A' + i); }
        return output.empty() ? "None yet" : output;
    }
    Guess guess(const std::string& text) {
        if (finished()) return Guess::finished;
        if (text.size() != 1) return Guess::invalid;
        char c = static_cast<char>(std::toupper(static_cast<unsigned char>(text[0])));
        if (c < 'A' || c > 'Z') return Guess::invalid;
        if (guessed_[c - 'A']) return Guess::repeated;
        guessed_[c - 'A'] = true;
        if (word_.find(c) != std::string::npos) return Guess::correct;
        ++mistakes_;
        return Guess::wrong;
    }
};
} // namespace hangman
