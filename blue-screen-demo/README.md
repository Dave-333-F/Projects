# Blue-screen simulation

A standalone, visibly labelled Windows drawing exercise for C++ coursework.
It paints a blue screen and a progress counter inside an ordinary window.
It is a simulation, not a virus or a real Windows crash.

Run `bin/blue-screen-demo.exe` from the repository root. Close with **Esc**,
**Enter**, **Alt+F4**, the title-bar X, or the **Close simulation** button.
It automatically closes after 30 seconds. The normal desktop remains accessible.

There are no keyboard hooks, startup entries, file modifications, network
requests, hidden payloads or forced restarts. It does not create disguised
shortcuts or package other programs. Demonstrate it with the other person's
knowledge; the simulation label is always visible.

Build from the repository root:

```powershell
.\blue-screen-demo\build.ps1
```

Or use the root `build-all.ps1`/CMake project after this folder has been added.
This project needs only a Windows C++17 compiler and the standard Windows GDI
and user-interface libraries already supplied with the toolchain.

`main.cpp` separates the state, drawing helper, window-message handler and entry
point. `WM_PAINT` draws the scene; `WM_TIMER` updates progress; close/key events
destroy the window and release fonts. No system-wide input is intercepted.

Hidden `--self-test-close` and `--self-test-escape` modes exercise its actual
message loop and exit handlers without showing a prank window on the desktop.
