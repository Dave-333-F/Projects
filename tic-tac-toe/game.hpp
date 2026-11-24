#pragma once
#include <array>
#include <stdexcept>

namespace ttt {
class Game {
    std::array<char, 9> cells_{};
    char turn_ = 'X';
public:
    Game() { cells_.fill(' '); }
    char at(int index) const { return cells_.at(static_cast<std::size_t>(index)); }
    char turn() const { return turn_; }
    char winner() const {
        constexpr int lines[8][3] = {{0,1,2},{3,4,5},{6,7,8},{0,3,6},{1,4,7},{2,5,8},{0,4,8},{2,4,6}};
        for (const auto& line : lines)
            if (cells_[line[0]] != ' ' && cells_[line[0]] == cells_[line[1]] && cells_[line[1]] == cells_[line[2]])
                return cells_[line[0]];
        return ' ';
    }
    bool full() const { for (char c : cells_) if (c == ' ') return false; return true; }
    bool finished() const { return winner() != ' ' || full(); }
    bool move(int index) {
        if (index < 0 || index >= 9 || cells_[index] != ' ' || finished()) return false;
        cells_[index] = turn_;
        turn_ = turn_ == 'X' ? 'O' : 'X';
        return true;
    }
};

// Minimax explores the tiny game tree. O maximises; X minimises.
inline int minimax(const Game& game, int depth = 0) {
    if (game.winner() == 'O') return 10 - depth;
    if (game.winner() == 'X') return depth - 10;
    if (game.full()) return 0;
    int best = game.turn() == 'O' ? -100 : 100;
    for (int i : {4, 0, 2, 6, 8, 1, 3, 5, 7}) {
        Game next = game;
        if (!next.move(i)) continue;
        int score = minimax(next, depth + 1);
        best = game.turn() == 'O' ? (score > best ? score : best) : (score < best ? score : best);
    }
    return best;
}

inline int computerMove(const Game& game) {
    if (game.finished() || game.turn() != 'O') return -1;
    int best = -100, choice = -1;
    for (int i : {4, 0, 2, 6, 8, 1, 3, 5, 7}) {
        Game next = game;
        if (!next.move(i)) continue;
        int score = minimax(next, 1);
        if (score > best) { best = score; choice = i; }
    }
    return choice;
}
} // namespace ttt
