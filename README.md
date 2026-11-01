# AI_Training

A small C++ console application for entering and displaying data records about web accounts,
run locally on Windows.

> **Status: greenfield.** No source code has been written yet. This README describes the
> intended project and the verified build setup; the design decisions listed at the bottom
> are still open.

## Requirements

The toolchain below is what is installed and verified on the development machine:

| Tool | Path | Version |
|---|---|---|
| `g++` / `gcc` | `C:\MinGW\bin\` | MinGW.org GCC 6.3.0 |
| `mingw32-make` | `C:\MinGW\bin\` | — |
| `cmake` | `C:\Program Files\CMake\bin\` | 4.3.1 |

Not installed: MSVC (`cl`), `clang++`, `make`, `ninja`.

## Building

Target **C++14**, not C++17. GCC 6.3.0 accepts `-std=c++17` but ships a pre-C++17 libstdc++:
`<optional>`, `<string_view>` and `<filesystem>` are missing and fail at `#include`.
Verified working under `-std=c++14`: `<vector> <string> <fstream> <sstream> <algorithm>
<iomanip> <memory> <cstdint>`.

```bash
g++ -std=c++14 -Wall -Wextra main.cpp -o app.exe
./app.exe
```

The shell used here is Git Bash (POSIX `sh`) on Windows, not `cmd.exe` — use forward slashes
and `/dev/null`.

## Planned functionality

A menu-driven console app that stores records describing web accounts, with add / list /
delete as the baseline operations.

## Open design decisions

These are unsettled and should be agreed before code is written:

1. **Record fields** — site / username / email / notes, and whether dates, category or
   status are included.
2. **Password storage** — whether records hold a password at all. A plain console app
   cannot store passwords securely; they would sit in cleartext on disk. If passwords are
   included, the app and this README must say so plainly rather than implying the data is
   protected.
3. **Persistence** — CSV loaded at startup and saved on exit, a custom line-based text
   format, or in-memory only.
4. **Menu operations** — add / list / delete as a baseline, plus optional edit, search and
   sort.
5. **Build setup** — a `build.bat` wrapping the `g++` command above, a `CMakeLists.txt`, or
   the hand-run compile command documented here.
