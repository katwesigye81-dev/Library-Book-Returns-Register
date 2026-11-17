// timing.cpp  --  Task A4: times insertion sort and quicksort (random pivot) on random BookReturn data.
// Build with optimisation and WITHOUT sanitizers:  g++ -std=c++17 -O2 timing.cpp -o timing
// Usage: ./timing [maxN]      (default maxN = 64000; sizes are 1000, 2000, 4000, ... up to maxN)
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <random>
#include <string>

#include "BookReturn.h"
#include "Sorting.h"

static std::unique_ptr<BookReturn[]> makeRandomRecords(std::size_t n, std::mt19937& g) {
    std::unique_ptr<BookReturn[]> a(new BookReturn[n]);
    for (std::size_t i = 0; i < n; ++i) {
        char date[16];
        std::snprintf(date, sizeof date, "%04d-%02d-%02d", 2000 + static_cast<int>(g() % 100), 1 + static_cast<int>(g() % 12), 1 + static_cast<int>(g() % 28));
        a[i].returnId   = static_cast<int>(i) + 1;
        a[i].accession  = "LIB-" + std::to_string(100000 + g() % 900000);
        a[i].title      = "Library book title number " + std::to_string(g() % 100000);   // longer than the small-string size
        a[i].borrowerId = std::to_string(2200000000u + g() % 300000000u);
        a[i].returnDate = date;
        a[i].daysLate   = static_cast<int>(g() % 15);
        a[i].fineUGX    = a[i].daysLate * 500;
    }
    return a;
}

int main(int argc, char** argv) {
    std::size_t maxN = (argc > 1) ? std::strtoul(argv[1], nullptr, 10) : 64000;
    const int REPS = 3;
    std::mt19937 gen(2026);              // fixed seed: same test data on every run
    sorting::seedQuickSort(42);          // fixed pivot sequence for repeatable timings
    std::ofstream csv("timing_results.csv");
    csv << "n,insertion_ms,quicksort_ms\n";
    std::printf("%8s %16s %16s\n", "n", "insertion (ms)", "quicksort (ms)");

    for (std::size_t n = 1000; n <= maxN; n *= 2) {
        double insTotal = 0, quickTotal = 0;
        for (int rep = 0; rep < REPS; ++rep) {
            std::unique_ptr<BookReturn[]> base = makeRandomRecords(n, gen);
            std::unique_ptr<BookReturn[]> a(new BookReturn[n]), b(new BookReturn[n]);
            for (std::size_t i = 0; i < n; ++i) { a[i] = base[i]; b[i] = base[i]; }   // both sorts get identical data

            auto t0 = std::chrono::steady_clock::now();
            sorting::insertionSort(a.get(), n, ByReturnDateThenId{});
            auto t1 = std::chrono::steady_clock::now();
            sorting::quickSort(b.get(), n, ByReturnDateThenId{});
            auto t2 = std::chrono::steady_clock::now();

            insTotal   += std::chrono::duration<double, std::milli>(t1 - t0).count();
            quickTotal += std::chrono::duration<double, std::milli>(t2 - t1).count();

            // correctness check, outside the timed region
            bool good = sorting::isSorted(a.get(), n, ByReturnDateThenId{});
            for (std::size_t i = 0; good && i < n; ++i) good = (a[i].returnId == b[i].returnId);
            if (!good) { std::cerr << "ERROR: sorts disagree or output not sorted at n=" << n << "\n"; return 1; }
        }
        std::printf("%8zu %16.3f %16.3f\n", n, insTotal / REPS, quickTotal / REPS);
        std::fflush(stdout);
        csv << n << ',' << insTotal / REPS << ',' << quickTotal / REPS << '\n';
        csv.flush();
    }
    return 0;
}
