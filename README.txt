LIBRARY BOOK RETURNS REGISTER  --  Scenario 5 (doubly linked list)
Coursework Part A: individual implementation and analysis
Sorting algorithm (registration number ends in 7, odd): insertion sort + quicksort with a random pivot
Language: C++17, standard library only. No STL containers are used inside the list class.

FILES
  BookReturn.h         record type (struct), file line format, validation, comparators
  DoublyLinkedList.h   Task A1: class template DoublyLinkedList<T> (header only)
  Sorting.h            Task A2: insertionSort, quickSort (random pivot), binarySearch (iterative)
  main.cpp             console menu application
  tests.cpp            Task A3: 47 tests + stability experiment (writes test_results.txt)
  timing.cpp           Task A4: timing experiment, 3 runs per size (writes timing_results.csv)
  data/returns.txt     15 sample records (format below)

DATA FILE FORMAT  (one record per line, '|' separated, lines starting with # are comments)
  returnId|accession|title|borrowerId|returnDate|daysLate|fineUGX
  1001|LIB-004512|Introduction to Algorithms|2400100123|2026-11-03|0|0
  Titles must not contain the '|' character. Dates must be yyyy-mm-dd.

BUILD AND RUN  (run every command from THIS folder, because the data path is relative)
  Application:
    g++ -std=c++17 -Wall -Wextra -pedantic -O1 main.cpp -o library_returns
    ./library_returns                   (or: ./library_returns path/to/other_file.txt)

  Tests, with memory checking (AddressSanitizer + UndefinedBehaviorSanitizer; also reports leaks):
    g++ -std=c++17 -Wall -Wextra -pedantic -O1 -g -fno-omit-frame-pointer -fsanitize=address,undefined tests.cpp -o tests
    ./tests

  Timing experiment (optimised, NO sanitizer; the 64,000-record insertion sort takes a while):
    g++ -std=c++17 -O2 timing.cpp -o timing
    ./timing            (or ./timing 16000 for a quicker run)

  On Windows with MinGW-w64, use the same g++ commands (AddressSanitizer is not available there; use
  WSL, Linux or macOS for the memory-check run). With MSVC: cl /std:c++17 /EHsc /O2 main.cpp

MENU OPTIONS
  1 load  2 show head->tail  3 show tail->head  4 register a return  5 remove first  6 remove last
  7 remove by return ID  8 sort (insertion sort)  9 sort (quicksort)  10 binary search by date  11 save  0 exit
  Option 10 needs the register to be sorted first (option 8 or 9), because binary search needs sorted data.
