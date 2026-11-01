# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project status

**Implemented.** A small C++ console application, run locally on this Windows machine, for entering and
displaying data records about web accounts.

The design is specified in Jira **KAN-4** —
<https://poparazvan1000.atlassian.net/browse/KAN-4>. The sections below mirror that ticket; it is the
source of truth if the two ever disagree.

### Files

- `main.cpp` — the whole application, one translation unit, everything in an anonymous namespace.
  Four layers: CSV primitives (`csvEscape`, `csvParseLine`) → persistence (`loadRecords`,
  `saveRecords`) → input helpers (`promptLine`, `promptRequired`, `promptInt`) → menu actions
  (`doAdd`, `doList`, `doDelete`).
- `build.bat` — wraps the `g++` command below; falls back to `C:\MinGW\bin\g++.exe` if `g++` is not
  on `PATH`.
- `README.md` — usage, build commands, data format, and the cleartext-password warning.
- `accounts.csv` — runtime data, created on first save. Gitignored; see below.

### Decisions made during implementation, beyond the ticket

- **List shows passwords in cleartext.** There is no reveal operation in scope, so masking them
  would make stored passwords unreadable and the tool pointless.
- **Newlines inside fields are not supported.** Each value is read with one `std::getline`, so a
  newline cannot enter a record through the UI. Commas and double quotes are fully supported, which
  is what the acceptance criteria require. This keeps the reader a line-at-a-time loop.
- **CSV has a `site,username,password` header row**, skipped on read; a headerless file still loads.
- **Add requires a non-empty site and username; an empty password is accepted** — it is odd but it
  is the user's own data.
- **Every prompt returns false at EOF**, and EOF saves and exits. Without this, a re-prompt loop
  against a closed stdin would spin forever.

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
  `Makefile`, editor/OS files, and runtime data (`*.log`, `data/`, `*.csv`). `accounts.csv` holds
  cleartext passwords and must never be committed — do not add a `!` exception for it.

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

## Acceptance criteria (KAN-4) — all verified against the built binary

- ✅ Builds clean under `-std=c++14 -Wall -Wextra` with no warnings.
- ✅ Add, list and delete all work from the menu.
- ✅ Records survive an exit and restart (CSV round trip).
- ✅ A password containing a comma and a double quote round-trips unchanged —
  `hu,nter"2` is stored as `"hu,nter""2"` and reads back identical.
- ✅ First run with no data file starts empty instead of erroring.
- ✅ Invalid menu input and out-of-range delete indices re-prompt rather than crash.
- ✅ The cleartext-password warning is visible in the app and in the README.

To re-verify, pipe a script of menu choices into the binary:

```bash
rm -f accounts.csv
printf '1\ngithub.com\nrazvan\nhu,nter"2\n4\n' | ./app.exe   # add, save
cat accounts.csv                                              # check the quoting
printf '2\n4\n' | ./app.exe                                   # reload and list
```
