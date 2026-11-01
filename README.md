# AI_Training

A small C++ console application for entering and displaying data records about web accounts,
run locally on Windows.

## ⚠ Passwords are stored in cleartext

**This application does not protect your passwords in any way.**

Every password you enter is written, unencrypted and unobfuscated, to a plain text file
named `accounts.csv` in the working directory. Anyone who can read that file — or a backup
of it, or a synced copy of it, or the console scrollback from a session — can read every
password it holds. There is no master password, no encryption and no access control, and a
console application cannot meaningfully provide any of those.

Use this only for throwaway or low-value accounts. Do not put a password here that protects
anything you care about.

## Requirements

The toolchain below is what is installed and verified on the development machine:

| Tool | Path | Version |
|---|---|---|
| `g++` / `gcc` | `C:\MinGW\bin\` | MinGW.org GCC 6.3.0 |
| `mingw32-make` | `C:\MinGW\bin\` | — |
| `cmake` | `C:\Program Files\CMake\bin\` | 4.3.1 |

Not installed: MSVC (`cl`), `clang++`, `make`, `ninja`.

## Building

Run the build script from `cmd.exe`:

```
build.bat
```

It finds `g++` on `PATH`, or falls back to `C:\MinGW\bin\g++.exe`, and produces `app.exe`.

Or compile by hand — this is exactly what `build.bat` runs:

```bash
g++ -std=c++14 -Wall -Wextra main.cpp -o app.exe
./app.exe
```

Target **C++14, not C++17.** GCC 6.3.0 accepts `-std=c++17` but ships a pre-C++17 libstdc++:
`<optional>`, `<string_view>` and `<filesystem>` are missing and fail at `#include`.
Verified working under `-std=c++14`: `<vector> <string> <fstream> <sstream> <algorithm>
<iomanip> <memory> <cstdint>`.

## Usage

```
app.exe
```

The app loads `accounts.csv` from the working directory at startup, then shows a menu:

```
  1) Add a record
  2) List records
  3) Delete a record
  4) Save and exit
```

- **Add** — prompts for site, username and password. Site and username are required; an
  empty password is accepted.
- **List** — prints a numbered table of all records. Passwords are shown as stored, in
  cleartext.
- **Delete** — lists the records, then removes the one you select by number. Enter `0` to
  cancel.
- **Save and exit** — writes `accounts.csv` and quits.

Records are held in memory while the app runs and written only on exit, so quitting the
process another way (closing the window, Ctrl+C) loses anything added that session.
Reaching end of input — Ctrl+Z, or a script piping commands in — saves and exits normally.

Invalid input is always re-prompted: a menu choice that is not 1–4, a delete number outside
the list, or text where a number is expected.

## Data file format

`accounts.csv` is CSV with a header row and the columns `site,username,password`, quoted per
RFC 4180: a field containing a comma or a double quote is wrapped in double quotes, and an
embedded double quote is doubled. So the password `hu,nter"2` is stored as:

```csv
site,username,password
github.com,razvan,"hu,nter""2"
```

A save/load round trip returns the exact strings you entered. Newlines inside a field are
not supported and cannot occur, since each value is read from a single console line.

If `accounts.csv` does not exist the app starts with an empty list — this is normal on first
run. A malformed line is reported with its line number and skipped; the rest of the file
still loads.

`accounts.csv` is listed in `.gitignore` and will not be committed. Keep it that way.
