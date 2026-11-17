// Sorting.h  --  Task A2: insertion sort, quicksort (random pivot) and iterative binary search.
// All three work on a plain array T* a of n elements. Every sort takes a comparator
// comp(x, y) that returns true when x must come BEFORE y (a strict weak ordering).
#pragma once
#include <cstddef>
#include <cstdint>
#include <random>
#include <utility>

namespace sorting {

// ---------------------------------------------------------------- insertion sort
// Stable: an element only moves left past elements that are STRICTLY greater, never past equal ones.
// Loop invariant: at the start of each pass for index i, a[0..i-1] holds the original first i
// elements rearranged into sorted order (equal elements still in their original relative order).
template <typename T, typename Compare>
void insertionSort(T* a, std::size_t n, Compare comp) {
    for (std::size_t i = 1; i < n; ++i) {
        T key = std::move(a[i]);
        std::size_t j = i;
        while (j > 0 && comp(key, a[j - 1])) {
            a[j] = std::move(a[j - 1]);      // shift the larger element one place right
            --j;
        }
        a[j] = std::move(key);
    }
}

// ---------------------------------------------------------------- quicksort, random pivot
inline std::mt19937_64& rng() {
    static std::mt19937_64 gen{std::random_device{}()};
    return gen;
}
// Optional: fix the random sequence so a run can be repeated exactly (used in tests/timing).
inline void seedQuickSort(std::uint64_t seed) { rng().seed(seed); }

namespace detail {
// Sorts a[lo..hi] (inclusive). Uses a three-way partition so that many equal keys cannot cause
// quadratic behaviour. Recurses into the SMALLER part and loops on the larger part, so the
// call stack never grows deeper than about log2(n).
template <typename T, typename Compare>
void quickSortRange(T* a, std::ptrdiff_t lo, std::ptrdiff_t hi, Compare& comp) {
    while (lo < hi) {
        std::uniform_int_distribution<std::ptrdiff_t> pick(0, hi - lo);
        T pivot = a[lo + pick(rng())];                 // random pivot (value copied out)

        // Invariant: a[lo..lt-1] < pivot,  a[lt..i-1] == pivot,  a[gt+1..hi] > pivot,  a[i..gt] unknown
        std::ptrdiff_t lt = lo, i = lo, gt = hi;
        while (i <= gt) {
            if (comp(a[i], pivot)) {
                if (lt != i) std::swap(a[lt], a[i]);
                ++lt; ++i;
            } else if (comp(pivot, a[i])) {
                if (i != gt) std::swap(a[i], a[gt]);
                --gt;
            } else {
                ++i;
            }
        }
        // a[lt..gt] are all equal to the pivot and already in their final place.
        if (lt - lo < hi - gt) { quickSortRange(a, lo, lt - 1, comp); lo = gt + 1; }
        else                   { quickSortRange(a, gt + 1, hi, comp); hi = lt - 1; }
    }
}
}  // namespace detail

template <typename T, typename Compare>
void quickSort(T* a, std::size_t n, Compare comp) {
    if (n < 2) return;
    detail::quickSortRange(a, 0, static_cast<std::ptrdiff_t>(n) - 1, comp);
}

// ---------------------------------------------------------------- iterative binary search
// a[0..n-1] must already be sorted by keyOf(...) in ascending order (using operator<).
// Returns the index of the FIRST element whose key equals 'key', or -1 if there is none.
// Invariant: every index < lo has a smaller key; every index >= hi has a key that is not smaller.
template <typename T, typename Key, typename KeyOf>
std::ptrdiff_t binarySearch(const T* a, std::size_t n, const Key& key, KeyOf keyOf) {
    std::size_t lo = 0, hi = n;                        // search window is [lo, hi)
    while (lo < hi) {
        std::size_t mid = lo + (hi - lo) / 2;
        if (keyOf(a[mid]) < key) lo = mid + 1;
        else                     hi = mid;
    }
    if (lo < n && !(key < keyOf(a[lo]))) return static_cast<std::ptrdiff_t>(lo);
    return -1;
}

// Helper for tests: true if no element is out of order according to comp.
template <typename T, typename Compare>
bool isSorted(const T* a, std::size_t n, Compare comp) {
    for (std::size_t i = 1; i < n; ++i)
        if (comp(a[i], a[i - 1])) return false;
    return true;
}

}  // namespace sorting
