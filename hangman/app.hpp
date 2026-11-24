#pragma once
#include "game.hpp"
#include "../common/navigation.hpp"
#include <random>

namespace hangman {
inline ui::Lines drawing(const Game& game) {
    int n = game.mistakes();
    return {"HANGMAN", "", "       +-------+", "       |       |",
            std::string("       |       ") + (n >= 1 ? "O" : " "),
            std::string("       |      ") + (n >= 3 ? "/" : " ") + (n >= 2 ? "|" : " ") + (n >= 4 ? "\\" : " "),
            std::string("       |      ") + (n >= 5 ? "/" : " ") + " " + (n >= 6 ? "\\" : " "),
            "       |", "     __|________", "", "WORD: " + game.masked(), "", "GUESSED: " + game.guessedLetters()};
}

inline void run() {
    const std::array<std::string, 3> categories = {"Programming", "Animals", "Everyday objects"};
    const std::array<std::vector<std::string>, 3> words = {{
        {"TEMPLATE", "COMPILER", "FUNCTION", "POINTER", "VARIABLE", "OBJECT", "VECTOR", "ALGORITHM"},
        {"ELEPHANT", "GIRAFFE", "PENGUIN", "DOLPHIN", "CHEETAH", "LEOPARD", "CROCODILE", "ZEBRA"},
        {"KEYBOARD", "BICYCLE", "UMBRELLA", "NOTEBOOK", "TELEPHONE", "BACKPACK", "CALENDAR", "SCISSORS"}
    }};
    std::mt19937 random(std::random_device{}());
    int won = 0, lost = 0, selectedCategory = 0;
    std::string notice;
    while (true) {
        auto choice = ui::choose("HANGMAN", "CHOOSE A CATEGORY",
                                 {"Programming", "Animals", "Everyday objects", "Back"},
                                 {"SESSION SCORE", "Won:  " + std::to_string(won), "Lost: " + std::to_string(lost), "", "Six wrong letters end a round.", "Repeated guesses cost no life."}, selectedCategory, notice);
        if (!choice || *choice == 3) return;
        auto category = static_cast<std::size_t>(*choice);
        std::uniform_int_distribution<std::size_t> distribution(0, words[category].size() - 1);
        Game game(words[category][distribution(random)]);
        notice = "Enter one letter. All occurrences of a correct letter are revealed.";
        bool abandoned = false;
        while (!game.finished()) {
            ui::screen("HANGMAN", categories[category], drawing(game),
                       {"ROUND STATUS", "Lives left: " + std::to_string(game.remaining()), "", "A-Z  Guess one letter", "0    Leave this round", "", "Guessing ignores letter case.", "Punctuation is rejected."}, notice);
            auto input = ui::input("Letter > ");
            if (!input) return;
            if (*input == "0") { abandoned = true; break; }
            switch (game.guess(*input)) {
                case Guess::correct: notice = "Correct letter!"; break;
                case Guess::wrong: notice = "That letter is not in the word."; break;
                case Guess::repeated: notice = "Already guessed. No life lost."; break;
                default: notice = "Enter exactly one letter from A to Z."; break;
            }
        }
        if (abandoned) { notice = "Round left without changing the score."; continue; }
        if (game.won()) ++won; else ++lost;
        notice = std::string(game.won() ? "You won! The word was " : "Out of lives. The word was ") + game.answer() + ".";
        ui::screen("HANGMAN", "ROUND COMPLETE", drawing(game), {"RESULT", game.won() ? "Word solved!" : "Try another word.", "", "Enter  Return to categories"}, notice);
        ui::waitForEnter();
    }
}
} // namespace hangman
