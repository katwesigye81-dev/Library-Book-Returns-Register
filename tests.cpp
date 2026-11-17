// tests.cpp  --  Task A3: runs every test, prints a table, writes test_results.txt
// Expected values are typed in by hand; they are NOT computed by the code being tested.
#include <algorithm>
#include <chrono>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "BookReturn.h"
#include "DoublyLinkedList.h"
#include "Sorting.h"

using Register = DoublyLinkedList<BookReturn>;

struct Row { std::string id, op, expected, actual; bool pass; };
static std::vector<Row> rows;

static void check(const std::string& id, const std::string& op, const std::string& expected, const std::string& actual) {
    rows.push_back({id, op, expected, actual, expected == actual});
}

static BookReturn mk(int id, const std::string& date = "2026-01-01", const std::string& title = "Book") {
    BookReturn b;
    b.returnId = id; b.accession = "LIB-" + std::to_string(id); b.title = title;
    b.borrowerId = "B" + std::to_string(id); b.returnDate = date;
    return b;
}
static std::string yn(bool b) { return b ? "true" : "false"; }

// "fwd=1,2,3 bwd=3,2,1 size=3": walking both directions checks the next AND prev links.
static std::string state(const Register& l) {
    std::string f, b;
    l.forEach([&](const BookReturn& r)        { f += (f.empty() ? "" : ",") + std::to_string(r.returnId); });
    l.forEachReverse([&](const BookReturn& r) { b += (b.empty() ? "" : ",") + std::to_string(r.returnId); });
    return "fwd=" + (f.empty() ? "-" : f) + " bwd=" + (b.empty() ? "-" : b) + " size=" + std::to_string(l.size());
}
static std::string ids(const BookReturn* a, std::size_t n) {
    std::string s;
    for (std::size_t i = 0; i < n; ++i) s += (s.empty() ? "" : ",") + std::to_string(a[i].returnId);
    return s.empty() ? "-" : s;
}
static Register listOf(std::initializer_list<int> v) { Register l; for (int x : v) l.pushBack(mk(x)); return l; }

int main() {
    sorting::seedQuickSort(12345);                       // repeatable pivots

    // ------------------------------------------------------------ A1: the list
    { Register l;
      check("A1-01", "New list: size() and empty()", "size=0 empty=true", "size=" + std::to_string(l.size()) + " empty=" + yn(l.empty()));
      BookReturn r;
      check("A1-02", "popFront() on empty list", "false, size=0", yn(l.popFront(&r)) + ", size=" + std::to_string(l.size()));
      check("A1-03", "popBack() on empty list", "false, size=0", yn(l.popBack(&r)) + ", size=" + std::to_string(l.size()));
      check("A1-04", "removeAt(0) on empty list", "false", yn(l.removeAt(0)));
      check("A1-05", "front(), back(), at(0) on empty list", "all nullptr", (!l.front() && !l.back() && !l.at(0)) ? "all nullptr" : "not null");
      check("A1-06", "findIf() on empty list", "nullptr", l.findIf([](const BookReturn&) { return true; }) ? "found" : "nullptr");
    }
    { Register l; l.pushBack(mk(1));
      check("A1-07", "pushBack one record", "fwd=1 bwd=1 size=1", state(l));
      check("A1-08", "one record: front() and back() are the same record", "true", yn(l.front() == l.back() && l.front() != nullptr));
      BookReturn r; bool ok = l.popFront(&r);
      check("A1-09", "popFront on the only record", "true, removed 1, fwd=- bwd=- size=0", yn(ok) + ", removed " + std::to_string(r.returnId) + ", " + state(l));
      check("A1-10", "after emptying: front()/back() are nullptr (head and tail reset)", "true", yn(!l.front() && !l.back()));
      l.pushBack(mk(9));
      check("A1-11", "list is reusable after becoming empty", "fwd=9 bwd=9 size=1", state(l));
    }
    { Register l; l.pushBack(mk(2)); l.pushBack(mk(3)); l.pushFront(mk(1));
      check("A1-12", "pushBack 2,3 then pushFront 1", "fwd=1,2,3 bwd=3,2,1 size=3", state(l)); }
    { Register l = listOf({1, 2, 3, 4, 5}); BookReturn r; bool ok = l.popFront(&r);
      check("A1-13", "remove FIRST record of 5", "true, removed 1, fwd=2,3,4,5 bwd=5,4,3,2 size=4", yn(ok) + ", removed " + std::to_string(r.returnId) + ", " + state(l)); }
    { Register l = listOf({1, 2, 3, 4, 5}); BookReturn r; bool ok = l.popBack(&r);
      check("A1-14", "remove LAST record of 5", "true, removed 5, fwd=1,2,3,4 bwd=4,3,2,1 size=4", yn(ok) + ", removed " + std::to_string(r.returnId) + ", " + state(l)); }
    { Register l = listOf({1, 2, 3, 4, 5}); BookReturn r; bool ok = l.removeAt(2, &r);
      check("A1-15", "removeAt(2) (middle) of 5", "true, removed 3, fwd=1,2,4,5 bwd=5,4,2,1 size=4", yn(ok) + ", removed " + std::to_string(r.returnId) + ", " + state(l)); }
    { Register l = listOf({1, 2, 3});
      bool a = l.removeAt(3), b = l.removeAt(1000);
      check("A1-16", "removeAt(size) and removeAt(1000): invalid index", "false, false, fwd=1,2,3 bwd=3,2,1 size=3", yn(a) + ", " + yn(b) + ", " + state(l)); }
    { Register l = listOf({1, 2, 3});
      bool a = l.insertAt(0, mk(10)), b = l.insertAt(2, mk(11)), c = l.insertAt(5, mk(12)), d = l.insertAt(7, mk(13));   // after 3 inserts size is 6, so 7 is size+1
      check("A1-17", "insertAt(0), insertAt(2), insertAt(5 = size), then insertAt(7 = size+1)", "true,true,true,false fwd=10,1,11,2,3,12 bwd=12,3,2,11,1,10 size=6",
            yn(a) + "," + yn(b) + "," + yn(c) + "," + yn(d) + " " + state(l)); }
    { Register l = listOf({1, 2, 3, 4});
      check("A1-18", "at(0), at(3) and at(4) (just past the end)", "1, 4, nullptr",
            std::to_string(l.at(0)->returnId) + ", " + std::to_string(l.at(3)->returnId) + ", " + (l.at(4) ? "found" : "nullptr")); }
    { Register l;                                           // duplicate key: two records with returnId 7
      l.pushBack(mk(7, "2026-01-01", "First copy")); l.pushBack(mk(8)); l.pushBack(mk(7, "2026-01-02", "Second copy"));
      const BookReturn* f = l.findIf([](const BookReturn& r) { return r.returnId == 7; });
      std::string found = f ? f->title : "none";
      BookReturn r; l.removeFirstIf([](const BookReturn& x) { return x.returnId == 7; }, &r);
      check("A1-19", "duplicate returnId 7: findIf returns first; removeFirstIf removes only the first", "First copy, removed First copy, fwd=8,7 bwd=7,8 size=2",
            found + ", removed " + r.title + ", " + state(l)); }
    { Register l = listOf({1, 2, 3}); l.clear();
      check("A1-20", "clear()", "fwd=- bwd=- size=0", state(l)); }
    { Register a = listOf({1, 2, 3}); Register b(a);          // copy constructor
      b.pushBack(mk(4)); b.front()->title = "CHANGED";
      check("A1-21", "copy constructor is deep (change the copy, original untouched)", "orig: fwd=1,2,3 bwd=3,2,1 size=3 title=Book; copy size=4",
            "orig: " + state(a) + " title=" + a.front()->title + "; copy size=" + std::to_string(b.size())); }
    { Register a = listOf({1, 2, 3}); Register b = listOf({9}); b = a; a.popFront();   // copy assignment
      Register& alias = b; b = alias;                                                  // self-assignment
      check("A1-22", "copy assignment is deep; self-assignment is safe", "b: fwd=1,2,3 bwd=3,2,1 size=3; a size=2",
            "b: " + state(b) + "; a size=" + std::to_string(a.size())); }
    { Register a = listOf({1, 2, 3}); Register b(std::move(a));
      check("A1-23", "move constructor: source left empty", "src size=0; dst fwd=1,2,3 bwd=3,2,1 size=3", "src size=" + std::to_string(a.size()) + "; dst " + state(b)); }
    { Register a = listOf({1, 2, 3, 4, 5, 6, 7, 8});
      a.assignFrom(nullptr, 0);
      check("A1-24", "assignFrom(nullptr, 0) empties the list", "fwd=- bwd=- size=0", state(a)); }

    // ------------------------------------------------------------ A1: files
    { Register l; std::size_t ok = 0, bad = 0;
      bool opened = l.loadFromFile("data/returns.txt", &ok, &bad);
      check("A1-25", "load data/returns.txt (sample data)", "true, loaded=15, skipped=0", yn(opened) + ", loaded=" + std::to_string(ok) + ", skipped=" + std::to_string(bad));
      check("A1-26", "after load: first and last record", "1001 .. 1015", std::to_string(l.front()->returnId) + " .. " + std::to_string(l.back()->returnId)); }
    { std::ofstream f("test_bad.txt");
      f << "# comment line\n"
           "1|LIB-1|Book A|B1|2026-01-01|0|0\n"
           "this is not a record\n"
           "2|LIB-2|Book B|B2|2026-13-45|0|0\n"
           "3|LIB-3|Book C|B3|2026-01-03|2|1000\r\n"
           "\n"
           "abc|LIB-4|Book D|B4|2026-01-04|0|0\n";
      f.close();
      Register l; std::size_t ok = 0, bad = 0; bool opened = l.loadFromFile("test_bad.txt", &ok, &bad);
      check("A1-27", "load file with comment, blank line, 3 malformed lines, 1 CRLF line", "true, loaded=2, skipped=3, fwd=1,3 bwd=3,1 size=2",
            yn(opened) + ", loaded=" + std::to_string(ok) + ", skipped=" + std::to_string(bad) + ", " + state(l)); }
    { Register l = listOf({1, 2}); bool opened = l.loadFromFile("no_such_file.txt");
      check("A1-28", "load a file that does not exist (list must be left unchanged)", "false, fwd=1,2 bwd=2,1 size=2", yn(opened) + ", " + state(l)); }
    { Register l = listOf({1, 2}); bool saved = l.saveToFile("/no_such_dir/out.txt");
      check("A1-29", "save to a path that cannot be written", "false", yn(saved)); }
    { Register l; l.loadFromFile("data/returns.txt");
      l.popFront(); l.popBack();                                           // remove first and last, then save
      bool saved = l.saveToFile("test_saved.txt", "returnId|accession|title|borrowerId|returnDate|daysLate|fineUGX");
      Register m; std::size_t ok = 0, bad = 0; m.loadFromFile("test_saved.txt", &ok, &bad);
      bool same = (m.size() == l.size());
      for (std::size_t i = 0; same && i < l.size(); ++i) same = (l.at(i)->toLine() == m.at(i)->toLine());
      check("A1-30", "remove first+last, save, reload: file round trip", "true, loaded=13, skipped=0, identical=true, first=1002 last=1014",
            yn(saved) + ", loaded=" + std::to_string(ok) + ", skipped=" + std::to_string(bad) + ", identical=" + yn(same) +
            ", first=" + std::to_string(m.front()->returnId) + " last=" + std::to_string(m.back()->returnId)); }

    // ------------------------------------------------------------ A2: sorting
    { BookReturn* none = nullptr;
      sorting::insertionSort(none, 0, ByReturnDate{}); sorting::quickSort(none, 0, ByReturnDate{});
      check("A2-01", "both sorts on an EMPTY array (n = 0)", "no crash", "no crash"); }
    { BookReturn a[1] = {mk(1)}, b[1] = {mk(1)};
      sorting::insertionSort(a, 1, ByReturnDate{}); sorting::quickSort(b, 1, ByReturnDate{});
      check("A2-02", "both sorts on ONE record", "1 | 1", ids(a, 1) + " | " + ids(b, 1)); }
    { BookReturn a[5], b[5];
      for (int i = 0; i < 5; ++i) a[i] = b[i] = mk(i + 1, "2026-01-0" + std::to_string(i + 1));
      sorting::insertionSort(a, 5, ByReturnDate{}); sorting::quickSort(b, 5, ByReturnDate{});
      check("A2-03", "ALREADY SORTED input (dates ascending)", "1,2,3,4,5 | 1,2,3,4,5", ids(a, 5) + " | " + ids(b, 5)); }
    { BookReturn a[5], b[5];
      for (int i = 0; i < 5; ++i) a[i] = b[i] = mk(i + 1, "2026-01-0" + std::to_string(5 - i));
      sorting::insertionSort(a, 5, ByReturnDate{}); sorting::quickSort(b, 5, ByReturnDate{});
      check("A2-04", "REVERSED input (dates descending)", "5,4,3,2,1 | 5,4,3,2,1", ids(a, 5) + " | " + ids(b, 5)); }
    { const char* d[5] = {"2026-01-02", "2026-01-01", "2026-01-02", "2026-01-01", "2026-01-02"};
      BookReturn a[5], b[5];
      for (int i = 0; i < 5; ++i) a[i] = b[i] = mk(i + 1, d[i]);
      sorting::insertionSort(a, 5, ByReturnDate{}); sorting::quickSort(b, 5, ByReturnDateThenId{});
      check("A2-05", "DUPLICATE dates: insertion sort by date only keeps entry order (stable)", "2,4,1,3,5", ids(a, 5));
      check("A2-06", "DUPLICATE dates: quicksort with (date, returnId) comparator gives the same order", "2,4,1,3,5", ids(b, 5)); }
    { const std::size_t N = 20000; std::unique_ptr<BookReturn[]> a(new BookReturn[N]);
      for (std::size_t i = 0; i < N; ++i) a[i] = mk(static_cast<int>(i) + 1, "2026-05-05");
      sorting::quickSort(a.get(), N, ByReturnDate{});
      check("A2-07", "quicksort, 20,000 records all with the SAME date (worst case for naive partitioning)", "sorted=true, order unchanged",
            "sorted=" + yn(sorting::isSorted(a.get(), N, ByReturnDate{})) + ", order " + (a[0].returnId == 1 && a[N - 1].returnId == static_cast<int>(N) ? "unchanged" : "changed")); }
    { const std::size_t N = 2000; std::mt19937 g(7);
      std::unique_ptr<BookReturn[]> a(new BookReturn[N]), b(new BookReturn[N]); std::vector<BookReturn> c;
      for (std::size_t i = 0; i < N; ++i) {
          char buf[16]; std::snprintf(buf, sizeof buf, "2026-%02d-%02d", 1 + static_cast<int>(g() % 12), 1 + static_cast<int>(g() % 28));
          a[i] = b[i] = mk(static_cast<int>(i) + 1, buf); c.push_back(a[i]);
      }
      sorting::insertionSort(a.get(), N, ByReturnDateThenId{}); sorting::quickSort(b.get(), N, ByReturnDateThenId{});
      std::stable_sort(c.begin(), c.end(), ByReturnDateThenId{});          // std::stable_sort used only as a reference answer
      bool sameA = true, sameB = true;
      for (std::size_t i = 0; i < N; ++i) { sameA = sameA && a[i].returnId == c[i].returnId; sameB = sameB && b[i].returnId == c[i].returnId; }
      check("A2-08", "2,000 random records: both sorts agree with std::stable_sort (reference)", "insertion=true, quick=true", "insertion=" + yn(sameA) + ", quick=" + yn(sameB)); }
    { Register l = listOf({1, 2, 3}); l.at(0)->returnDate = "2026-03-01"; l.at(1)->returnDate = "2026-01-01"; l.at(2)->returnDate = "2026-02-01";
      std::unique_ptr<BookReturn[]> buf(new BookReturn[3]); l.copyTo(buf.get(), 3);
      sorting::quickSort(buf.get(), 3, ByReturnDate{}); l.assignFrom(buf.get(), 3);
      check("A2-09", "list -> array -> quicksort -> list (links rebuilt)", "fwd=2,3,1 bwd=1,3,2 size=3", state(l)); }
    { Register l; l.loadFromFile("data/returns.txt");
      const std::size_t n = l.size();
      std::unique_ptr<BookReturn[]> a(new BookReturn[n]), b(new BookReturn[n]);
      l.copyTo(a.get(), n); l.copyTo(b.get(), n);
      auto byFineDesc = [](const BookReturn& x, const BookReturn& y) { if (x.fineUGX != y.fineUGX) return x.fineUGX > y.fineUGX; return x.returnId < y.returnId; };
      sorting::insertionSort(a.get(), n, byFineDesc); sorting::quickSort(b.get(), n, byFineDesc);
      const std::string want = "1009,1004,1012,1007,1002,1014,1006,1015,1001,1003,1005,1008,1010,1011,1013";
      auto summary = [&](const BookReturn* r) { return "first=" + std::to_string(r[0].returnId) + " last=" + std::to_string(r[n - 1].returnId) + ", full order correct=" + yn(ids(r, n) == want); };
      const std::string good = "first=1009 last=1013, full order correct=true";
      check("A2-10", "15 sample records sorted by a DIFFERENT field (fineUGX, highest first; ties by returnId) using a lambda comparator: insertion | quick", good + " | " + good, summary(a.get()) + " | " + summary(b.get())); }

    // ------------------------------------------------------------ A2: binary search
    { const char* d[6] = {"2026-02-01", "2026-02-03", "2026-02-03", "2026-02-03", "2026-02-07", "2026-02-09"};
      BookReturn a[6]; for (int i = 0; i < 6; ++i) a[i] = mk(i + 1, d[i]);
      auto keyOf = [](const BookReturn& r) -> const std::string& { return r.returnDate; };
      auto find = [&](const char* k) { return std::to_string(sorting::binarySearch(a, 6, std::string(k), keyOf)); };
      check("A2-11", "binary search, key present once (middle-right)", "4", find("2026-02-07"));
      check("A2-12", "binary search, DUPLICATE key 2026-02-03 (3 copies) -> index of first", "1", find("2026-02-03"));
      check("A2-13", "binary search, first element / last element", "0, 5", find("2026-02-01") + ", " + find("2026-02-09"));
      check("A2-14", "binary search, ABSENT key between two keys", "-1", find("2026-02-05"));
      check("A2-15", "binary search, ABSENT key smaller than all / larger than all", "-1, -1", find("2026-01-01") + ", " + find("2026-03-01"));
      BookReturn one[1] = {mk(1, "2026-02-03")}; BookReturn* none = nullptr;
      check("A2-16", "binary search on ONE record: present / absent", "0, -1",
            std::to_string(sorting::binarySearch(one, 1, std::string("2026-02-03"), keyOf)) + ", " + std::to_string(sorting::binarySearch(one, 1, std::string("2026-02-04"), keyOf)));
      check("A2-17", "binary search on EMPTY array", "-1", std::to_string(sorting::binarySearch(none, 0, std::string("2026-02-03"), keyOf))); }

    // ------------------------------------------------------------ print the table
    int passed = 0;
    std::ofstream out("test_results.txt");
    for (const Row& r : rows) {
        passed += r.pass;
        std::cout << (r.pass ? "[PASS] " : "[FAIL] ") << r.id << "  " << r.op << "\n         expected: " << r.expected << "\n         actual:   " << r.actual << "\n";
        out << r.id << '\t' << r.op << '\t' << r.expected << '\t' << r.actual << '\t' << (r.pass ? "PASS" : "FAIL") << '\n';
    }
    std::cout << "\n" << passed << " of " << rows.size() << " tests passed.\n";

    // ------------------------------------------------------------ stability experiment (information only)
    {
        Register l; l.loadFromFile("data/returns.txt");
        const std::size_t n = l.size();
        std::unique_ptr<BookReturn[]> base(new BookReturn[n]), ref(new BookReturn[n]), t(new BookReturn[n]);
        l.copyTo(base.get(), n); l.copyTo(ref.get(), n);
        sorting::insertionSort(ref.get(), n, ByReturnDate{});               // the stable answer
        int changedDateOnly = 0, changedWithId = 0, runs = 100;
        for (int seed = 1; seed <= runs; ++seed) {
            sorting::seedQuickSort(static_cast<std::uint64_t>(seed));
            std::copy(base.get(), base.get() + n, t.get()); sorting::quickSort(t.get(), n, ByReturnDate{});
            if (ids(t.get(), n) != ids(ref.get(), n)) ++changedDateOnly;
            std::copy(base.get(), base.get() + n, t.get()); sorting::quickSort(t.get(), n, ByReturnDateThenId{});
            if (ids(t.get(), n) != ids(ref.get(), n)) ++changedWithId;
        }
        std::cout << "\nSTABILITY EXPERIMENT (15 sample records, " << runs << " quicksort runs with different pivot seeds)\n"
                  << "  stable order (insertion sort, date only): " << ids(ref.get(), n) << "\n"
                  << "  quicksort, date only            : tie order changed in " << changedDateOnly << " of " << runs << " runs\n"
                  << "  quicksort, (date, returnId)     : tie order changed in " << changedWithId << " of " << runs << " runs\n";
        std::ofstream s("stability_results.txt");
        s << ids(ref.get(), n) << '\t' << changedDateOnly << '\t' << changedWithId << '\t' << runs << '\n';
    }
    return passed == static_cast<int>(rows.size()) ? 0 : 1;
}
