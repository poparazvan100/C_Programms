# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project status

Greenfield — **no source code exists yet**. The directory contains only `README.md`, `.gitignore` and
this file.

Intended project: a small C++ console application, run locally on this Windows machine, for entering and
displaying data records about web accounts.

The design is fully specified in Jira **KAN-4** (Story, To Do) —
<https://poparazvan1000.atlassian.net/browse/KAN-4>. The sections below mirror that ticket; it is the
source of truth if the two ever disagree.

### Deliverables

- `main.cpp` — the application (single translation unit).
- `build.bat` — wraps the verified `g++` command below.
- `README.md` — usage, build commands, and the cleartext-password warning.

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
- Git repository on branch `main`, pushed to `origin`
  (https://github.com/poparazvan100/C_Programms.git).
- `.gitignore` ignores build output (`*.exe`, `build/`, objects), CMake artifacts, a generated
  `Makefile`, editor/OS files, and runtime data (`*.log`, `data/`, `*.csv`). If the app ends up
  persisting to CSV and a sample file should be committed, add a `!` exception for it.

## Design (settled in KAN-4)

### Record fields

Exactly three, nothing else: `site`, `username`, `password`. No email, notes, dates, category or status.

### Passwords are stored in cleartext — say so

A plain console app cannot store passwords securely. They sit in **cleartext in the CSV file**, readable
by anyone with access to that file, a backup of it, or the console scrollback.

State this plainly in the app itself and in the README. Do not word anything to imply the data is
encrypted, protected or safe. The tool is for throwaway or low-value accounts only.

### Menu operations

Exactly three, plus exit — **edit, search and sort are out of scope**:

1. **Add** — prompt for site, username, password; append to the in-memory list.
2. **List** — readable table of all records, with an index per row.
3. **Delete** — select by the index shown in the list, remove it.
4. **Exit** — save to CSV and quit.

Required behaviour:

- An invalid menu choice or an out-of-range delete index re-prompts — never crash, never silently no-op.
- Non-numeric input where a number is expected is handled (clear the stream state, re-prompt).
- Listing an empty set prints a message, not an empty table.

### Persistence — CSV, loaded at startup, saved on exit

- Columns in order: `site,username,password`.
- **RFC 4180 quoting is required.** A naive `out << a << "," << b` corrupts the file the moment a
  password contains a comma, a double quote or a newline — and passwords routinely contain commas. Wrap
  a field in double quotes when it contains any of those, and escape an embedded double quote by
  doubling it (`""`).
- The reader must handle quoted fields symmetrically, so a save/load round trip returns the exact
  original strings.
- A missing data file on first run is normal — start with an empty list, do not error.
- A malformed line is reported to the user, not silently dropped and not fatal.

### Build setup — `build.bat`, not CMake

Chosen because the app is a single translation unit and the `g++` invocation below is already proven on
this machine; CMake would add a configure step for no benefit. The README documents both `build.bat` and
the raw `g++` command so the app can still be built by hand.

## Acceptance criteria (KAN-4)

- Builds clean under `-std=c++14 -Wall -Wextra` with no warnings.
- Add, list and delete all work from the menu.
- Records survive an exit and restart (CSV round trip).
- A password containing a comma and a double quote round-trips unchanged.
- First run with no data file starts empty instead of erroring.
- Invalid menu input and out-of-range delete indices re-prompt rather than crash.
- The cleartext-password warning is visible in the app and in the README.
