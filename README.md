# C++ Coursework Projects

Four Windows C++17 projects for learning game state, templates and event-driven
interfaces. Both collaborators can work on every project. This is a shared
baseline; contributions are recorded when each person actually makes them.

Target repository: https://github.com/Dave-333-F/Projects

| Project | What it demonstrates | Run from the repository root |
|---|---|---|
| Tic-tac-toe | Arrays, legal moves, win detection and minimax | `bin/tic-tac-toe-cursor.exe` |
| Hangman | Strings, guessed letters, lives and random word selection | `bin/hangman-cursor.exe` |
| Library manager | `Repository<T>` reused for books/members, loans and file storage | `bin/library-cursor.exe` |
| Blue-screen simulation | Windows drawing, timers and normal close events | `bin/blue-screen-demo.exe` |

The blue-screen project is a clearly labelled simulation, not a virus. It closes
with Esc, Enter, Alt+F4, its close button or the title-bar X, and automatically
after 30 seconds. It does not lock the desktop, block shortcuts, create disguised
payloads, change files, access the network or restart Windows.

## Run without compiling

Use the prebuilt **64-bit Windows** executables in `bin/`. Open
`run-coursework.cmd`, or from PowerShell in this directory:

```powershell
.\bin\coursework-cursor.exe
```

The launcher opens tic-tac-toe, hangman and the library. Run the separate graphical
simulation with:

```powershell
.\bin\blue-screen-demo.exe
```

Use Up/Down and Enter in menus; click options where the terminal supports mouse
events. Tic-tac-toe has blank cells, an arrow-key cursor and click-to-place moves.
Esc goes back. Hangman guesses and library text fields use ordinary typing.
For two side-by-side console panels, use a terminal at least 100 columns wide.

The library creates its own `bin/data/library.txt` after a successful change or
Save. No personal saved catalogue is included. Use only one library process per
save file. See each project's README for a code walkthrough and controls.

## Build from source

Use an existing Windows C++17 compiler. The PowerShell scripts look for `g++` in
PATH, then `C:\msys64\ucrt64\bin\g++.exe`. They do not install dependencies or
change PowerShell execution policy.

```powershell
.\build-all.ps1
```

This builds the current console executables with the `-cursor` suffix and the
standalone `blue-screen-demo.exe`. Close a running copy before rebuilding it.
The scripts do not forcibly terminate your applications or delete old binaries.

With CMake already installed, an alternative is:

```powershell
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

CMake executable names do not have the `-cursor` suffix and are under its own
`build/bin` configuration folder. The blue-screen application targets Windows.

## Tests

Python 3 is needed only for the integration checks, not to run the applications:

```powershell
.\build-all.ps1 -Test
```

Optional console-event tests, using hidden disposable app windows:

```powershell
python tests/check_console_navigation.py
```

Tests use synthetic data under `tests/results` and leave personal catalogue data
alone. The blue-screen tests use hidden windows and verify actual Esc/close
handlers; they do not show a prank screen on somebody else's desktop.
See BUILD-VERIFICATION.md in the packaged handoff for observed results.

## Collaborating and sharing by USB

Read **COLLABORATION.md** before the first push. Both people should have their
own clone and GitHub sign-in. Transfer source changes by USB if needed, but do
not share a `.git` directory, passwords, tokens or Git Credential Manager data.
Always pull the latest remote commits before beginning the next contribution.

Use CONTRIBUTIONS.md to record real work and commit hashes. Pushes are network
operations: one push may contain several commits. A useful change with a clear
commit message is the unit of work, rather than an empty push.

## What .gitignore keeps out

The repository root is allowlisted. Only these coursework folders, shared build
files, guides and five named application executables are intended for GitHub.
Other root folders are ignored by default. Runtime data, test output, caches,
private keys, environment files, temporary archives and extra binaries are ignored.

The cleaned collection excludes the original shortcut/portable-program builder,
malware samples and analysis evidence, received files, file-transfer/cloud service
configuration, private upload links and lecture PDFs/slides. Those local projects
are separate from this coursework handoff. A `.gitignore` does not remove files
that were already committed; inspect `git status` and the staged diff every time.

The included binary checksums identify this packaged build. Rebuilding changes
the hashes; update the checksum list when deliberately publishing a new binary.
