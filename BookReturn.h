// BookReturn.h  --  record type for Scenario 5: Library Book Returns Register
#pragma once
#include <cstddef>
#include <string>

// One line in the register = one book handed back at a returns desk.
struct BookReturn {
    int         returnId   = 0;  // entry sequence number given when the record is keyed in (1,2,3...)
    std::string accession;       // book copy number, e.g. "LIB-004512"
    std::string title;           // must not contain the '|' character (it is the file separator)
    std::string borrowerId;      // student / staff number
    std::string returnDate;      // ISO text yyyy-mm-dd, so text order == date order
    int         daysLate   = 0;  // 0 if returned on time
    int         fineUGX    = 0;  // fine charged in Uganda shillings

    // File format (one record per line):
    //   returnId|accession|title|borrowerId|returnDate|daysLate|fineUGX
    std::string toLine() const {
        return std::to_string(returnId) + '|' + accession + '|' + title + '|' + borrowerId + '|' +
               returnDate + '|' + std::to_string(daysLate) + '|' + std::to_string(fineUGX);
    }

    // Splits "a|b|c|d|e|f|g" into exactly 7 fields. Returns false if there are fewer or more than 7.
    static bool splitSeven(const std::string& s, std::string (&f)[7]) {
        std::size_t start = 0;
        for (int i = 0; i < 7; ++i) {
            std::size_t bar = s.find('|', start);
            if (i < 6) {
                if (bar == std::string::npos) return false;      // too few fields
                f[i] = s.substr(start, bar - start);
                start = bar + 1;
            } else {
                if (bar != std::string::npos) return false;      // too many fields
                f[i] = s.substr(start);
            }
        }
        return true;
    }

    // Accepts only plain digits (no sign, no spaces); at most 9 digits so it fits in an int.
    static bool parseNonNegInt(const std::string& s, int& out) {
        if (s.empty() || s.size() > 9) return false;
        int v = 0;
        for (char c : s) {
            if (c < '0' || c > '9') return false;
            v = v * 10 + (c - '0');
        }
        out = v;
        return true;
    }

    // Checks the shape yyyy-mm-dd with month 01-12 and day 01-31.
    static bool validDate(const std::string& d) {
        if (d.size() != 10 || d[4] != '-' || d[7] != '-') return false;
        const int digitPos[8] = {0, 1, 2, 3, 5, 6, 8, 9};
        for (int p : digitPos)
            if (d[p] < '0' || d[p] > '9') return false;
        int month = (d[5] - '0') * 10 + (d[6] - '0');
        int day   = (d[8] - '0') * 10 + (d[9] - '0');
        return month >= 1 && month <= 12 && day >= 1 && day <= 31;
    }

    // Parses and validates one line. Returns false (and leaves 'out' untouched) if anything is wrong.
    static bool fromLine(const std::string& line, BookReturn& out) {
        std::string f[7];
        if (!splitSeven(line, f)) return false;
        BookReturn r;
        if (!parseNonNegInt(f[0], r.returnId) || r.returnId <= 0) return false;
        if (f[1].empty() || f[2].empty() || f[3].empty())         return false;
        if (!validDate(f[4]))                                     return false;
        if (!parseNonNegInt(f[5], r.daysLate))                    return false;
        if (!parseNonNegInt(f[6], r.fineUGX))                     return false;
        r.accession = f[1];
        r.title     = f[2];
        r.borrowerId = f[3];
        r.returnDate = f[4];
        out = r;
        return true;
    }
};

// Comparators used by the sorts (any "strict weak ordering" callable works).
struct ByReturnDate {            // date only: ties are "equal"
    bool operator()(const BookReturn& a, const BookReturn& b) const { return a.returnDate < b.returnDate; }
};
struct ByReturnDateThenId {      // date, then entry order: a total order, so no two records tie
    bool operator()(const BookReturn& a, const BookReturn& b) const {
        if (a.returnDate != b.returnDate) return a.returnDate < b.returnDate;
        return a.returnId < b.returnId;
    }
};
