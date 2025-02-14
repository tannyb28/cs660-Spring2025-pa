#include <db/Database.hpp>
#include <db/HeapPage.hpp>
#include <stdexcept>

using namespace db;

HeapPage::HeapPage(Page &page, const TupleDesc &td) : td(td) {
    // TODO pa1
    // NOTE: header and data should point to locations inside the page buffer. Do not allocate extra memory.
}

size_t HeapPage::begin() const {
    // TODO pa1
    throw std::runtime_error("not implemented");
}

size_t HeapPage::end() const {
    // TODO pa1
}

bool HeapPage::insertTuple(const Tuple &t) {
    // TODO pa1
}

void HeapPage::deleteTuple(size_t slot) {
    // TODO pa1
}

Tuple HeapPage::getTuple(size_t slot) const {
    // TODO pa1
}

void HeapPage::next(size_t &slot) const {
    // TODO pa1
}

bool HeapPage::empty(size_t slot) const {
    // TODO pa1
}
