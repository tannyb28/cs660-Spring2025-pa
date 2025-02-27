#include <db/Database.hpp>
#include <db/HeapFile.hpp>
#include <db/HeapPage.hpp>
#include <stdexcept>
#include <cstring>

using namespace db;

HeapFile::HeapFile(const std::string &name, const TupleDesc &td) : DbFile(name, td) {}

void HeapFile::insertTuple(const Tuple &t) {
    // TODO pa1
    size_t lastPageId = getNumPages() - 1;
    Page page;
    readPage(page, lastPageId);
    HeapPage heapPage(page, getTupleDesc());
    if (!heapPage.insertTuple(t)) {
        // No space in last page, so create a new page.
        Page newPage = {};
        HeapPage newHeapPage(newPage, getTupleDesc());
        if (!newHeapPage.insertTuple(t)) {
            throw std::runtime_error("Failed to insert tuple in new page");
        }
        writePage(newPage, getNumPages());
        numPages++;
    } else {
        writePage(page, lastPageId);
    }
}

void HeapFile::deleteTuple(const Iterator &it) {
    // TODO pa1
    Page page;
    readPage(page, it.page);
    HeapPage heapPage(page, getTupleDesc());
    heapPage.deleteTuple(it.slot);
    writePage(page, it.page);
}

Tuple HeapFile::getTuple(const Iterator &it) const {
    // TODO pa1
    Page page;
    // (Using const_cast to call readPage in a const method)
    const_cast<HeapFile*>(this)->readPage(page, it.page);
    HeapPage heapPage(page, getTupleDesc());
    return heapPage.getTuple(it.slot);
}

void HeapFile::next(Iterator &it) const {
    // TODO pa1
    Page page;
    while (true) {
        if (it.page >= getNumPages()) {
            it.page = getNumPages();
            return;
        }
        const_cast<HeapFile*>(this)->readPage(page, it.page);
        HeapPage heapPage(page, getTupleDesc());
        if (it.slot >= heapPage.end()) {
            it.page++;
            it.slot = 0;
            continue;
        }
        size_t curr = it.slot;
        heapPage.next(it.slot);
        if (it.slot < heapPage.end()) {
            return;
        } else {
            it.page++;
            it.slot = 0;
        }
    }
}

Iterator HeapFile::begin() const {
    // TODO pa1
    Iterator it(*this, 0, 0);
    Page page;
    while (it.page < getNumPages()) {
        const_cast<HeapFile*>(this)->readPage(page, it.page);
        HeapPage heapPage(page, getTupleDesc());
        size_t first = heapPage.begin();
        if (first < heapPage.end()) {
            it.slot = first;
            return it;
        }
        it.page++;
    }
    return end();
}

Iterator HeapFile::end() const {
    // TODO pa1
    return Iterator(*this, getNumPages(), 0);
}
