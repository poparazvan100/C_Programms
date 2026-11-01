# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project status

Greenfield — **no source code exists yet**. The directory contains only `File1.txt` (empty) and this file.

Intended project: a small C++ console application, run locally on this Windows machine, for entering and
displaying data records about web accounts.

## Toolchain on this machine (verified)

| Tool | Path | Version |
|---|---|---|
| `g++` / `gcc` | `C:\MinGW\bin\` | MinGW.org GCC 6.3.0 |
| `mingw32-make` | `C:\MinGW\bin\` | — |
| `cmake` | `C:\Program Files\CMake\bin\` | 4.3.1 |

Not installed: MSVC (`cl`), `clang++`, `make`, `ninja`.

### Target C++14, not C++17

GCC 6.3.0 accepts `-std=c++17` but ships a pre-C++17 libstdc++. Headers including `<optional>`,
`<string_view>` and `<filesystem>` are **missing** and fail at `#include`. Verified working under
`-std=c++14`: `<vector> <string> <fstream> <sstream> <algorithm> <iomanip> <memory> <cstdint>`.

Compile and run (verified):

```bash
g++ -std=c++14 -Wall -Wextra main.cpp -o app.exe
./app.exe
```

## Environment notes

- Shell is Git Bash (POSIX `sh`) on Windows, not `cmd.exe` — use forward slashes and `/dev/null`.
- Not a git repository.

## Open design decisions

These were not settled before planning stopped; confirm with the user before writing code:

1. **Record fields** — site / username / email / notes, and whether dates, category or status are included.
2. **Password storage** — whether records hold a password at all. A plain console app cannot store
   passwords securely; they would sit in cleartext on disk. If the user wants this, say so plainly in the
   app and README rather than implying the data is protected.
3. **Persistence** — CSV file loaded at startup and saved on exit, a custom line-based text format, or
   in-memory only.
4. **Menu operations** — add / list / delete as a baseline, plus optional edit, search and sort.
5. **Build setup** — a `build.bat` wrapping the `g++` command above, a `CMakeLists.txt`, or a hand-run
   compile command documented in the README.
