#pragma once
#include "game.hpp"
#include "../common/navigation.hpp"

namespace ttt {
// A cell is five characters wide; one separator follows each of the first two.
constexpr int BOARD_INDENT = 10;
constexpr int FIRST_BOARD_LINE = 4;

inline ui::Lines board(const Game& game, int selected = -1) {
    ui::Lines lines = {"BOARD", "", "       Three marks in a row wins", ""};
    for (int row = 0; row < 3; ++row) {
        std::string line(BOARD_INDENT, ' ');
        for (int column = 0; column < 3; ++column) {
            int square = row * 3 + column;
            char mark = game.at(square); // Empty cells stay blank, never numbered.
            line += square == selected ? std::string(" [") + mark + "] " : std::string("  ") + mark + "  ";
            if (column < 2) line += '|';
        }
        lines.push_back(line);
        if (row < 2) lines.push_back(std::string(BOARD_INDENT, ' ') + "-----+-----+-----");
    }
    return lines;
}

inline int moveCursor(int selected, ui::Key key) {
    int row = selected / 3, column = selected % 3;
    if (key == ui::Key::up) row = std::max(0, row - 1);
    if (key == ui::Key::down) row = std::min(2, row + 1);
    if (key == ui::Key::left) column = std::max(0, column - 1);
    if (key == ui::Key::right) column = std::min(2, column + 1);
    return row * 3 + column;
}

inline int clickedSquare(const ui::Hit& hit) {
    int line = hit.line - FIRST_BOARD_LINE;
    int column = hit.column - BOARD_INDENT;
    if (line < 0 || line > 4 || line % 2 != 0 || column < 0 || column >= 17 || column % 6 == 5) return -1;
    return (line / 2) * 3 + column / 6;
}

enum class RoundResult { complete, back, inputClosed };

inline RoundResult playRound(Game& game, bool computer) {
    ui::ConsoleInput controls;
    int selected = 0;
    std::string notice = "Move the [ ] cursor, then press Enter; or click a square.";
    while (!game.finished()) {
        if (computer && game.turn() == 'O') {
            game.move(computerMove(game));
            notice = "Computer placed O. Your turn.";
            continue;
        }
        ui::screen("TIC-TAC-TOE", computer ? "YOU (X) vs COMPUTER (O)" : "PLAYER X vs PLAYER O", board(game, selected),
                   {"YOUR TURN", std::string("Current player: ") + game.turn(), "", "Arrow keys  Move cursor", "Enter/Space Place mark", "Mouse       Click square", "Esc         Leave round", "", "[ ] shows the selected cell.", "Occupied cells cannot be used."}, notice);
        int square = -1;
        if (controls.active()) {
            auto event = controls.read();
            if (event.key == ui::Key::end) return RoundResult::inputClosed;
            if (event.key == ui::Key::escape) return RoundResult::back;
            selected = moveCursor(selected, event.key);
            if (event.key == ui::Key::enter) square = selected;
            if (event.key == ui::Key::click) {
                auto hit = ui::hitTest(event.x, event.y);
                if (hit) square = clickedSquare(*hit);
                if (square >= 0) selected = square;
            }
            if (square < 0) continue;
        } else {
            // Redirected text retains a simple test protocol, not the live UI.
            auto text = ui::input("Square for test input (1-9, 0 back) > ");
            if (!text) return RoundResult::inputClosed;
            if (*text == "0") return RoundResult::back;
            auto value = ui::number(*text);
            if (value && *value >= 1 && *value <= 9) square = *value - 1;
        }
        if (!game.move(square)) notice = "Invalid move. Select an empty square.";
        else notice.clear();
    }
    return RoundResult::complete;
}

inline void run() {
    int xWins = 0, oWins = 0, draws = 0, selectedMode = 0;
    std::string notice;
    while (true) {
        auto mode = ui::choose("TIC-TAC-TOE", "CHOOSE A MODE",
                              {"Play against the computer", "Two players on this keyboard", "Back"},
                              {"SESSION SCORE", "X wins: " + std::to_string(xWins), "O wins: " + std::to_string(oWins), "Draws:  " + std::to_string(draws), "", "X always starts.", "Computer plays O."}, selectedMode, notice);
        if (!mode || *mode == 2) return;
        Game game;
        auto result = playRound(game, *mode == 0);
        if (result == RoundResult::inputClosed) return;
        if (result == RoundResult::back) { notice = "Round left without changing the score."; continue; }
        std::string outcome;
        if (game.winner() == 'X') { ++xWins; outcome = "X wins!"; }
        else if (game.winner() == 'O') { ++oWins; outcome = "O wins!"; }
        else { ++draws; outcome = "Draw. Well played!"; }
        ui::screen("TIC-TAC-TOE", "ROUND COMPLETE", board(game), {"RESULT", outcome, "", "Enter  Return to modes"}, outcome);
        ui::waitForEnter();
        notice = outcome;
    }
}
} // namespace ttt
