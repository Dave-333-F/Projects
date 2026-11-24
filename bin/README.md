# Ready-to-run Windows applications

Only the five current coursework executables are included:

- `coursework-cursor.exe`: launcher for the three console applications.
- `tic-tac-toe-cursor.exe`.
- `hangman-cursor.exe`.
- `library-cursor.exe`.
- `blue-screen-demo.exe`: clearly labelled, easily closed drawing simulation.

These are 64-bit Windows builds from the included source. A compiler is not
needed to run them. Python is needed only for development tests.

`SHA256SUMS.txt` records the packaged binaries' hashes. A hash is an integrity
reference, not a code-signing certificate. If Windows blocks an unfamiliar app,
do not disable security controls; review the source or build it yourself.

The library creates `data/library.txt` locally when saving. That catalogue,
duplicate old builds, test executables and the original shortcut maker are
excluded by the repository's `.gitignore`.
