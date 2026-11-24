# Tic-tac-toe

Run `bin/tic-tac-toe-cursor.exe` from the coursework root, or select Tic-tac-toe in
`bin/coursework-cursor.exe`. Build using `build-all.ps1` at the root.

Choose computer mode or two-player mode with Up/Down and Enter, or a mouse click.
X starts. Empty squares are blank. Move the `[ ]` cursor with the arrow keys and
press Enter/Space, or click a square. Esc leaves the round. Leaving does not change the
score. Completed rounds count X wins, O wins and draws for this session.

## Explain the code

1. `game.hpp`: `Game` stores nine cells in `std::array<char, 9>` and the next mark.
2. `move()` checks bounds, occupation and whether the game is already over before
   changing state. An invalid move does not switch the player.
3. `winner()` checks the eight winning triples; a full board without a winner is
   a draw.
4. `minimax()` explores possible future games using copies of `Game`. An O win
   has a positive score and an X win a negative score. O maximises and X minimises.
   Depth favours a quicker win or a later loss.
5. `computerMove()` chooses the best legal O move. Testing explores every human
   strategy against this deterministic computer and verifies that it never loses.
6. `app.hpp` handles presentation, cursor position and scores. `moveCursor()`
   changes a row/column and clamps it to the board. `clickedSquare()` maps a click
   to a cell and rejects borders. `playRound()` repeats draw/read/update until the
   game finishes or the player presses Esc. `main.cpp` starts the app.

Windows keyboard/mouse events live in `common/navigation.hpp`, outside the game
rules. Input modes and cursor visibility are restored automatically. Redirected
text tests can still use 1-9 as an input protocol, but the board never displays
numbers. Tests check cursor bounds, every cell's mouse mapping and blank rendering.

The minimax algorithm is optional advanced material: begin by explaining the
two-player game. The core game does not include console or Windows dependencies.
