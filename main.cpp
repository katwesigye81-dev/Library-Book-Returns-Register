// main.cpp  --  console application for the Library Book Returns Register (Scenario 5)
#include <iomanip>
#include <iostream>
#include <memory>
#include <string>

#include "BookReturn.h"
#include "DoublyLinkedList.h"
#include "Sorting.h"

using Register = DoublyLinkedList<BookReturn>;

namespace {

const int   FINE_PER_DAY_UGX = 500;
const char* const FILE_HEADER = "returnId|accession|title|borrowerId|returnDate|daysLate|fineUGX";

bool ask(const std::string& prompt, std::string& out) {
    std::cout << prompt << std::flush;
    return static_cast<bool>(std::getline(std::cin, out));
}

void printHeader() {
    std::cout << std::left << std::setw(7) << "ID" << std::setw(12) << "Accession" << std::setw(34) << "Title"
              << std::setw(12) << "Borrower" << std::setw(12) << "Date" << std::setw(6) << "Late"
              << "Fine(UGX)\n" << std::string(92, '-') << '\n';
}

void printRow(const BookReturn& r) {
    std::string t = r.title.size() > 32 ? r.title.substr(0, 31) + "~" : r.title;
    std::cout << std::left << std::setw(7) << r.returnId << std::setw(12) << r.accession << std::setw(34) << t
              << std::setw(12) << r.borrowerId << std::setw(12) << r.returnDate << std::setw(6) << r.daysLate
              << r.fineUGX << '\n';
}

void showForward(const Register& reg) {
    if (reg.empty()) { std::cout << "The register is empty.\n"; return; }
    printHeader();
    reg.forEach(printRow);
    std::cout << reg.size() << " record(s), head to tail.\n";
}

void showBackward(const Register& reg) {
    if (reg.empty()) { std::cout << "The register is empty.\n"; return; }
    printHeader();
    reg.forEachReverse(printRow);
    std::cout << reg.size() << " record(s), tail to head.\n";
}

int nextId(const Register& reg) {
    int maxId = 0;
    reg.forEach([&](const BookReturn& r) { if (r.returnId > maxId) maxId = r.returnId; });
    return maxId + 1;
}

// Copy the list into an array, sort the array, and rebuild the list from it.
void sortRegister(Register& reg, bool useQuick) {
    std::size_t n = reg.size();
    if (n < 2) { std::cout << "Nothing to sort.\n"; return; }
    std::unique_ptr<BookReturn[]> buf(new BookReturn[n]);
    reg.copyTo(buf.get(), n);
    if (useQuick) sorting::quickSort(buf.get(), n, ByReturnDateThenId{});
    else          sorting::insertionSort(buf.get(), n, ByReturnDateThenId{});
    reg.assignFrom(buf.get(), n);
    std::cout << "Sorted " << n << " record(s) by return date using "
              << (useQuick ? "quicksort (random pivot)" : "insertion sort") << ".\n";
}

void searchByDate(const Register& reg) {
    std::string date;
    if (!ask("Return date to find (yyyy-mm-dd): ", date)) return;
    if (!BookReturn::validDate(date)) { std::cout << "That is not a valid date.\n"; return; }
    std::size_t n = reg.size();
    std::unique_ptr<BookReturn[]> buf(new BookReturn[n ? n : 1]);
    reg.copyTo(buf.get(), n);
    auto keyOf = [](const BookReturn& r) -> const std::string& { return r.returnDate; };
    std::ptrdiff_t idx = sorting::binarySearch(buf.get(), n, date, keyOf);
    if (idx < 0) { std::cout << "No book was returned on " << date << ".\n"; return; }
    printHeader();
    for (std::size_t i = static_cast<std::size_t>(idx); i < n && buf[i].returnDate == date; ++i) printRow(buf[i]);
}

void registerReturn(Register& reg, bool& sorted) {
    std::string acc, title, borrower, date, late;
    if (!ask("Accession no.: ", acc) || !ask("Title: ", title) || !ask("Borrower ID: ", borrower) ||
        !ask("Return date (yyyy-mm-dd): ", date) || !ask("Days late: ", late))
        return;
    int days = 0;
    if (!BookReturn::parseNonNegInt(late, days)) { std::cout << "Rejected: days late must be a whole number.\n"; return; }
    std::string line = std::to_string(nextId(reg)) + '|' + acc + '|' + title + '|' + borrower + '|' + date + '|' +
                       late + '|' + std::to_string(days * FINE_PER_DAY_UGX);
    BookReturn rec;
    if (!BookReturn::fromLine(line, rec)) { std::cout << "Rejected: a field is empty or invalid (no '|' allowed).\n"; return; }
    reg.pushBack(rec);
    sorted = false;
    std::cout << "Recorded return #" << rec.returnId << ", fine " << rec.fineUGX << " UGX.\n";
}

void removeById(Register& reg) {
    std::string s;
    int id = 0;
    if (!ask("Return ID to remove: ", s)) return;
    if (!BookReturn::parseNonNegInt(s, id)) { std::cout << "Rejected: not a number.\n"; return; }
    BookReturn gone;
    if (reg.removeFirstIf([id](const BookReturn& r) { return r.returnId == id; }, &gone))
        std::cout << "Removed #" << gone.returnId << " \"" << gone.title << "\".\n";
    else
        std::cout << "No record with return ID " << id << ".\n";
}

void menu() {
    std::cout << "\n===== Library Book Returns Register =====\n"
                 " 1 Load from file          2 Show (head to tail)    3 Show (tail to head)\n"
                 " 4 Register a return       5 Remove first record    6 Remove last record\n"
                 " 7 Remove by return ID     8 Sort: insertion sort   9 Sort: quicksort\n"
                 "10 Search by return date  11 Save to file           0 Exit\n";
}

}  // namespace

int main(int argc, char** argv) {
    const std::string path = (argc > 1) ? argv[1] : "data/returns.txt";
    Register reg;
    bool sorted = false;   // true only while the list is known to be in date order

    std::size_t ok = 0, bad = 0;
    if (reg.loadFromFile(path, &ok, &bad)) std::cout << "Loaded " << ok << " record(s) from " << path << " (" << bad << " skipped).\n";
    else                                   std::cout << "Could not open " << path << "; starting with an empty register.\n";

    std::string choice;
    for (;;) {
        menu();
        if (!ask("Choice: ", choice)) break;           // end of input
        if (choice == "0") break;
        else if (choice == "1") {
            if (reg.loadFromFile(path, &ok, &bad)) { std::cout << "Loaded " << ok << " record(s), " << bad << " skipped.\n"; sorted = false; }
            else std::cout << "Could not open " << path << ".\n";
        }
        else if (choice == "2") showForward(reg);
        else if (choice == "3") showBackward(reg);
        else if (choice == "4") registerReturn(reg, sorted);
        else if (choice == "5") { BookReturn r; if (reg.popFront(&r)) std::cout << "Removed #" << r.returnId << ".\n"; else std::cout << "The register is empty.\n"; }
        else if (choice == "6") { BookReturn r; if (reg.popBack(&r))  std::cout << "Removed #" << r.returnId << ".\n"; else std::cout << "The register is empty.\n"; }
        else if (choice == "7") removeById(reg);
        else if (choice == "8") { sortRegister(reg, false); sorted = !reg.empty(); }
        else if (choice == "9") { sortRegister(reg, true);  sorted = !reg.empty(); }
        else if (choice == "10") { if (sorted) searchByDate(reg); else std::cout << "Sort the register by date first (option 8 or 9).\n"; }
        else if (choice == "11") std::cout << (reg.saveToFile(path, FILE_HEADER) ? "Saved to " + path + ".\n" : "Could not write " + path + ".\n");
        else std::cout << "Unknown choice.\n";
    }
    std::cout << "Goodbye.\n";
    return 0;
}
