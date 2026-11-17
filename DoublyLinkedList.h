// DoublyLinkedList.h  --  Task A1: hand-written doubly linked list (class template)
// No STL containers are used. T must be default-constructible and copyable, and for the
// file functions must provide:   std::string T::toLine() const   and
//                                static bool T::fromLine(const std::string&, T&)
#pragma once
#include <cstddef>
#include <fstream>
#include <string>
#include <utility>

template <typename T>
class DoublyLinkedList {
    struct Node {
        T     data;
        Node* prev;
        Node* next;
        explicit Node(const T& d) : data(d), prev(nullptr), next(nullptr) {}
    };

    Node*       head_ = nullptr;
    Node*       tail_ = nullptr;
    std::size_t size_ = 0;

    // Walk from whichever end is nearer. Returns nullptr when i is out of range.
    Node* nodeAt(std::size_t i) const {
        if (i >= size_) return nullptr;
        Node* cur;
        if (i < size_ / 2) {
            cur = head_;
            for (std::size_t k = 0; k < i; ++k) cur = cur->next;
        } else {
            cur = tail_;
            for (std::size_t k = size_ - 1; k > i; --k) cur = cur->prev;
        }
        return cur;
    }

    // Detach n from the chain and repair head_/tail_/size_. The caller deletes n.
    void unlink(Node* n) {
        if (n->prev) n->prev->next = n->next; else head_ = n->next;
        if (n->next) n->next->prev = n->prev; else tail_ = n->prev;
        --size_;
    }

    template <typename Pred>
    Node* findNode(Pred pred) const {
        for (Node* c = head_; c; c = c->next)
            if (pred(static_cast<const T&>(c->data))) return c;
        return nullptr;
    }

public:
    // ---- construction / destruction (rule of five) ----
    DoublyLinkedList() = default;

    ~DoublyLinkedList() { clear(); }

    DoublyLinkedList(const DoublyLinkedList& other) {          // deep copy
        try {
            for (Node* c = other.head_; c; c = c->next) pushBack(c->data);
        } catch (...) {                                        // e.g. std::bad_alloc: do not leak
            clear();
            throw;
        }
    }

    DoublyLinkedList& operator=(const DoublyLinkedList& other) {   // copy-and-swap
        if (this != &other) {
            DoublyLinkedList tmp(other);
            swap(tmp);
        }
        return *this;
    }

    DoublyLinkedList(DoublyLinkedList&& other) noexcept
        : head_(other.head_), tail_(other.tail_), size_(other.size_) {
        other.head_ = other.tail_ = nullptr;
        other.size_ = 0;
    }

    DoublyLinkedList& operator=(DoublyLinkedList&& other) noexcept {
        if (this != &other) {
            clear();
            head_ = other.head_; tail_ = other.tail_; size_ = other.size_;
            other.head_ = other.tail_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    void swap(DoublyLinkedList& other) noexcept {
        std::swap(head_, other.head_);
        std::swap(tail_, other.tail_);
        std::swap(size_, other.size_);
    }

    // ---- size ----
    std::size_t size()  const { return size_; }
    bool        empty() const { return size_ == 0; }

    // ---- adding records ----
    void pushFront(const T& v) {
        Node* n = new Node(v);
        n->next = head_;
        if (head_) head_->prev = n; else tail_ = n;
        head_ = n;
        ++size_;
    }

    void pushBack(const T& v) {
        Node* n = new Node(v);
        n->prev = tail_;
        if (tail_) tail_->next = n; else head_ = n;
        tail_ = n;
        ++size_;
    }

    // Insert so that the new record ends up at position idx (0 .. size()). false if idx is invalid.
    bool insertAt(std::size_t idx, const T& v) {
        if (idx > size_) return false;
        if (idx == 0)     { pushFront(v); return true; }
        if (idx == size_) { pushBack(v);  return true; }
        Node* at = nodeAt(idx);                 // idx is 1 .. size_-1, so 'at' and at->prev exist
        Node* n  = new Node(v);
        n->prev = at->prev;
        n->next = at;
        at->prev->next = n;
        at->prev = n;
        ++size_;
        return true;
    }

    // ---- removing records (each returns false instead of crashing when nothing can be removed) ----
    bool popFront(T* removed = nullptr) {
        if (!head_) return false;
        Node* n = head_;
        if (removed) *removed = n->data;
        unlink(n);
        delete n;
        return true;
    }

    bool popBack(T* removed = nullptr) {
        if (!tail_) return false;
        Node* n = tail_;
        if (removed) *removed = n->data;
        unlink(n);
        delete n;
        return true;
    }

    bool removeAt(std::size_t idx, T* removed = nullptr) {
        Node* n = nodeAt(idx);
        if (!n) return false;
        if (removed) *removed = n->data;
        unlink(n);
        delete n;
        return true;
    }

    template <typename Pred>
    bool removeFirstIf(Pred pred, T* removed = nullptr) {   // removes only the FIRST match
        Node* n = findNode(pred);
        if (!n) return false;
        if (removed) *removed = n->data;
        unlink(n);
        delete n;
        return true;
    }

    void clear() {
        Node* c = head_;
        while (c) {
            Node* next = c->next;
            delete c;
            c = next;
        }
        head_ = tail_ = nullptr;
        size_ = 0;
    }

    // ---- access (nullptr when there is no such record) ----
    T*       front()           { return head_ ? &head_->data : nullptr; }
    const T* front() const     { return head_ ? &head_->data : nullptr; }
    T*       back()            { return tail_ ? &tail_->data : nullptr; }
    const T* back() const      { return tail_ ? &tail_->data : nullptr; }
    T*       at(std::size_t i) { Node* n = nodeAt(i); return n ? &n->data : nullptr; }
    const T* at(std::size_t i) const { Node* n = nodeAt(i); return n ? &n->data : nullptr; }

    template <typename Pred>
    T* findIf(Pred pred) { Node* n = findNode(pred); return n ? &n->data : nullptr; }
    template <typename Pred>
    const T* findIf(Pred pred) const { Node* n = findNode(pred); return n ? &n->data : nullptr; }

    // ---- traversal in both directions (this is what the prev pointers are for) ----
    template <typename Fn>
    void forEach(Fn fn) const {
        for (Node* c = head_; c; c = c->next) fn(static_cast<const T&>(c->data));
    }
    template <typename Fn>
    void forEachReverse(Fn fn) const {
        for (Node* c = tail_; c; c = c->prev) fn(static_cast<const T&>(c->data));
    }

    // ---- bridge to arrays (used by the sorting and binary-search code in Task A2) ----
    // Copies up to 'cap' records into dest (front to back) and returns how many were copied.
    std::size_t copyTo(T* dest, std::size_t cap) const {
        std::size_t i = 0;
        for (Node* c = head_; c && i < cap; c = c->next) dest[i++] = c->data;
        return i;
    }

    // Replace the whole list with the n records in src (in that order).
    void assignFrom(const T* src, std::size_t n) {
        DoublyLinkedList tmp;                       // build first, swap in only if it all worked
        for (std::size_t i = 0; i < n; ++i) tmp.pushBack(src[i]);
        swap(tmp);
    }

    // ---- text-file persistence ----
    // Blank lines and lines starting with '#' are ignored; malformed lines are skipped and counted.
    // Returns false only if the file cannot be opened (the list is then left unchanged).
    bool loadFromFile(const std::string& path, std::size_t* loaded = nullptr, std::size_t* skipped = nullptr) {
        std::ifstream in(path);
        if (!in) return false;
        DoublyLinkedList tmp;
        std::size_t ok = 0, bad = 0;
        std::string line;
        while (std::getline(in, line)) {
            if (!line.empty() && line.back() == '\r') line.pop_back();   // tolerate Windows line ends
            if (line.empty() || line[0] == '#') continue;
            T rec;
            if (T::fromLine(line, rec)) { tmp.pushBack(rec); ++ok; }
            else                        { ++bad; }
        }
        swap(tmp);
        if (loaded)  *loaded  = ok;
        if (skipped) *skipped = bad;
        return true;
    }

    // Writes one record per line. Returns false if the file cannot be opened or written.
    bool saveToFile(const std::string& path, const std::string& header = "") const {
        std::ofstream out(path, std::ios::trunc);
        if (!out) return false;
        if (!header.empty()) out << "# " << header << '\n';
        for (Node* c = head_; c; c = c->next) out << c->data.toLine() << '\n';
        out.flush();
        return static_cast<bool>(out);
    }
};
