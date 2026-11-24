# Hangman

Run `bin/hangman-cursor.exe` from the coursework root, or select Hangman in
`bin/coursework-cursor.exe`. Build using `build-all.ps1` at the root.

Choose programming, animals or everyday objects using Up/Down and Enter or by
clicking the category. Esc leaves the category menu. Guess one letter at a time
by typing it and pressing Enter.
Six incorrect letters end the round. Correct letters reveal every occurrence;
repeated or invalid guesses do not cost lives. Upper/lowercase are treated the
same. `0` leaves without recording a win or loss. Scores last for this session.

## Explain the code

1. `game.hpp`: `Game` owns the answer, an array of 26 guessed-letter flags and a
   count of incorrect guesses.
2. `guess()` validates input, converts to uppercase, checks for repetition, and
   records a correct or incorrect guess. It returns a descriptive `enum class`.
3. `masked()` displays known letters and underscores without revealing the answer.
4. `won()` checks that every answer letter has been guessed. `finished()` also
   checks whether all six lives were used.
5. `app.hpp` chooses a word with `std::mt19937` and a uniform distribution, renders
   the drawing, and handles menus and scores.
6. `main.cpp` starts the app. The rule class has no UI/Windows dependencies.

The word lists are built into `app.hpp` so the application works offline and
does not need an external dictionary. The tests use fixed words for reproducibility.
