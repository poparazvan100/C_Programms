// A console tool for keeping records of web accounts.
//
// WARNING: passwords are stored in cleartext. See printWarning() below and the
// README. This is suitable for throwaway or low-value accounts only.
//
// Build: g++ -std=c++14 -Wall -Wextra main.cpp -o app.exe

#include <algorithm>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

namespace {

const char* const kDataFile = "accounts.csv";
const char* const kHeaderLine = "site,username,password";

struct Record {
    std::string site;
    std::string username;
    std::string password;
};

// ---------------------------------------------------------------------------
// CSV field handling (RFC 4180, minus multi-line fields)
//
// A field is quoted when it contains a comma or a double quote; an embedded
// double quote is escaped by doubling it. Newlines inside fields are not
// supported and cannot occur: every field is read from a single console line.
// ---------------------------------------------------------------------------

std::string csvEscape(const std::string& field) {
    if (field.find_first_of(",\"") == std::string::npos) {
        return field;
    }
    std::string out = "\"";
    for (char c : field) {
        if (c == '"') {
            out += "\"\"";
        } else {
            out += c;
        }
    }
    out += '"';
    return out;
}

// Splits one CSV line into fields. Returns false if the line is malformed:
// an unterminated quoted field, or text after a closing quote.
bool csvParseLine(const std::string& line, std::vector<std::string>& out) {
    out.clear();
    std::string field;
    std::size_t i = 0;

    while (true) {
        field.clear();

        if (i < line.size() && line[i] == '"') {
            ++i;  // opening quote
            bool closed = false;
            while (i < line.size()) {
                if (line[i] == '"') {
                    if (i + 1 < line.size() && line[i + 1] == '"') {
                        field += '"';  // escaped quote
                        i += 2;
                    } else {
                        ++i;  // closing quote
                        closed = true;
                        break;
                    }
                } else {
                    field += line[i];
                    ++i;
                }
            }
            if (!closed) {
                return false;  // ran off the end still inside quotes
            }
            if (i < line.size() && line[i] != ',') {
                return false;  // junk between the closing quote and the comma
            }
        } else {
            while (i < line.size() && line[i] != ',') {
                field += line[i];
                ++i;
            }
        }

        out.push_back(field);

        if (i >= line.size()) {
            return true;
        }
        ++i;  // consume the comma, then read the next field
    }
}

// ---------------------------------------------------------------------------
// Persistence
// ---------------------------------------------------------------------------

// Reads records from path. A missing file is not an error: the list stays
// empty and the app starts fresh. Malformed lines are reported and skipped.
void loadRecords(const std::string& path, std::vector<Record>& records) {
    std::ifstream in(path.c_str());
    if (!in) {
        return;  // first run
    }

    std::string line;
    std::size_t lineNumber = 0;
    std::size_t skipped = 0;

    while (std::getline(in, line)) {
        ++lineNumber;

        // Tolerate CRLF files read in text mode on other platforms.
        if (!line.empty() && line[line.size() - 1] == '\r') {
            line.erase(line.size() - 1);
        }
        if (line.empty()) {
            continue;
        }
        if (lineNumber == 1 && line == kHeaderLine) {
            continue;  // header; a headerless file is read just as happily
        }

        std::vector<std::string> fields;
        if (!csvParseLine(line, fields) || fields.size() != 3) {
            std::cout << path << ":" << lineNumber
                      << ": malformed line, skipped\n";
            ++skipped;
            continue;
        }

        Record r;
        r.site = fields[0];
        r.username = fields[1];
        r.password = fields[2];
        records.push_back(r);
    }

    std::cout << "Loaded " << records.size() << " record(s) from " << path;
    if (skipped > 0) {
        std::cout << " (" << skipped << " line(s) skipped)";
    }
    std::cout << ".\n";
}

bool saveRecords(const std::string& path, const std::vector<Record>& records) {
    std::ofstream out(path.c_str(), std::ios::trunc);
    if (!out) {
        std::cout << "Error: could not open " << path << " for writing.\n";
        return false;
    }

    out << kHeaderLine << "\n";
    for (const Record& r : records) {
        out << csvEscape(r.site) << "," << csvEscape(r.username) << ","
            << csvEscape(r.password) << "\n";
    }
    out.flush();

    if (!out) {
        std::cout << "Error: failed while writing " << path
                  << "; records may be lost.\n";
        return false;
    }
    std::cout << "Saved " << records.size() << " record(s) to " << path << ".\n";
    return true;
}

// ---------------------------------------------------------------------------
// Console input
//
// Every prompt returns false on end of input (Ctrl+Z, or a pipe running dry),
// so a re-prompt loop can never spin forever against a closed stdin.
// ---------------------------------------------------------------------------

bool promptLine(const char* label, std::string& value) {
    std::cout << label;
    if (!std::getline(std::cin, value)) {
        return false;
    }
    return true;
}

// Re-prompts until a non-empty value is given.
bool promptRequired(const char* label, std::string& value) {
    while (promptLine(label, value)) {
        if (!value.empty()) {
            return true;
        }
        std::cout << "  This field cannot be empty.\n";
    }
    return false;
}

// Reads a whole line and parses it as an int, so stray text never leaves the
// stream in a failed state. Returns false only on end of input.
bool promptInt(const char* label, int& value) {
    std::string line;
    while (promptLine(label, line)) {
        std::istringstream parser(line);
        int parsed = 0;
        char trailing = 0;
        if (parser >> parsed && !(parser >> trailing)) {
            value = parsed;
            return true;
        }
        std::cout << "  Please enter a number.\n";
    }
    return false;
}

// ---------------------------------------------------------------------------
// Menu actions
// ---------------------------------------------------------------------------

void printWarning() {
    std::cout
        << "----------------------------------------------------------------\n"
        << " WARNING: passwords are stored in CLEARTEXT in " << kDataFile << ".\n"
        << " They are not encrypted and not protected in any way. Anyone who\n"
        << " can read that file, a backup of it, or this console's scrollback\n"
        << " can read every password you enter here.\n"
        << " Use this only for throwaway or low-value accounts.\n"
        << "----------------------------------------------------------------\n";
}

void doAdd(std::vector<Record>& records) {
    std::cout << "\nAdd a record (leave the site blank at any point to stop):\n";

    Record r;
    if (!promptRequired("  Site:     ", r.site)) {
        return;
    }
    if (!promptRequired("  Username: ", r.username)) {
        return;
    }
    // An empty password is allowed: unusual, but it is the user's own data.
    if (!promptLine("  Password: ", r.password)) {
        return;
    }

    records.push_back(r);
    std::cout << "Added. " << records.size() << " record(s) held.\n";
}

void doList(const std::vector<Record>& records) {
    if (records.empty()) {
        std::cout << "\nNo records yet. Use option 1 to add one.\n";
        return;
    }

    // Size each column to its widest value so the table stays readable.
    std::size_t siteWidth = 4;      // "SITE"
    std::size_t userWidth = 8;      // "USERNAME"
    for (const Record& r : records) {
        siteWidth = std::max(siteWidth, r.site.size());
        userWidth = std::max(userWidth, r.username.size());
    }

    std::cout << "\n  Passwords below are shown as stored, in cleartext.\n\n";
    std::cout << "  " << std::setw(3) << std::right << "#" << "  "
              << std::setw(static_cast<int>(siteWidth)) << std::left << "SITE"
              << "  " << std::setw(static_cast<int>(userWidth)) << std::left
              << "USERNAME" << "  " << "PASSWORD" << "\n";

    for (std::size_t i = 0; i < records.size(); ++i) {
        std::cout << "  " << std::setw(3) << std::right << (i + 1) << "  "
                  << std::setw(static_cast<int>(siteWidth)) << std::left
                  << records[i].site << "  "
                  << std::setw(static_cast<int>(userWidth)) << std::left
                  << records[i].username << "  " << records[i].password << "\n";
    }
    std::cout << "\n" << records.size() << " record(s).\n";
}

void doDelete(std::vector<Record>& records) {
    if (records.empty()) {
        std::cout << "\nNothing to delete: there are no records.\n";
        return;
    }

    doList(records);

    int choice = 0;
    while (promptInt("\nNumber to delete (0 to cancel): ", choice)) {
        if (choice == 0) {
            std::cout << "Cancelled.\n";
            return;
        }
        if (choice >= 1 && choice <= static_cast<int>(records.size())) {
            const Record& doomed = records[static_cast<std::size_t>(choice - 1)];
            std::cout << "Deleted " << doomed.site << " / " << doomed.username
                      << ".\n";
            records.erase(records.begin() + (choice - 1));
            return;
        }
        std::cout << "  Please enter a number between 1 and " << records.size()
                  << ", or 0 to cancel.\n";
    }
}

void printMenu() {
    std::cout << "\n  1) Add a record\n"
              << "  2) List records\n"
              << "  3) Delete a record\n"
              << "  4) Save and exit\n";
}

}  // namespace

int main() {
    std::cout << "Web account records\n";
    printWarning();

    std::vector<Record> records;
    loadRecords(kDataFile, records);

    int choice = 0;
    bool running = true;
    while (running) {
        printMenu();
        if (!promptInt("Choice: ", choice)) {
            // End of input: save rather than discard what was entered.
            std::cout << "\nEnd of input.\n";
            break;
        }

        switch (choice) {
            case 1: doAdd(records);    break;
            case 2: doList(records);   break;
            case 3: doDelete(records); break;
            case 4: running = false;   break;
            default:
                std::cout << "  Please choose 1, 2, 3 or 4.\n";
                break;
        }
    }

    return saveRecords(kDataFile, records) ? 0 : 1;
}
