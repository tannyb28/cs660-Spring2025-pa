#include <db/Database.hpp>
#include <db/HeapPage.hpp>
#include <stdexcept>
#include <cstring>
#include <cmath>

using namespace db;

HeapPage::HeapPage(Page &page, const TupleDesc &td) : td(td) {
    // TODO pa1
    // Compute the number of slots: capacity = floor((P*8) / (td.length()*8 + 1))
    capacity = (DEFAULT_PAGE_SIZE * 8) / (td.length() * 8 + 1);
    size_t headerBytes = (capacity + 7) / 8;
    header = page.data();  // header starts at offset 0
    // Data begins after the padding: at offset = DEFAULT_PAGE_SIZE - (td.length() * capacity)
    data = page.data() + (DEFAULT_PAGE_SIZE - (td.length() * capacity));
}

size_t HeapPage::begin() const {
    // TODO pa1
    for (size_t i = 0; i < capacity; i++) {
        if (!empty(i)) {
            return i;
        }
    }
    return capacity;
}

size_t HeapPage::end() const {
    // TODO pa1
    return capacity;
}

bool HeapPage::insertTuple(const Tuple &t) {
    // TODO pa1
    for (size_t i = 0; i < capacity; i++) {
        if (empty(i)) {
            // Mark slot as used.
            header[i / 8] |= (1 << (7 - (i % 8)));
            td.serialize(data + i * td.length(), t);
            return true;
        }
    }
    return false;
}

void HeapPage::deleteTuple(size_t slot) {
    // TODO pa1
    if (slot >= capacity || empty(slot)) {
        throw std::runtime_error("Slot is empty, cannot delete tuple");
    }
    header[slot / 8] &= ~(1 << (7 - (slot % 8)));
    std::memset(data + slot * td.length(), 0, td.length());
}

Tuple HeapPage::getTuple(size_t slot) const {
    // TODO pa1
    if (slot >= capacity || empty(slot)) {
        throw std::runtime_error("Slot is empty, cannot get tuple");
    }
    return td.deserialize(data + slot * td.length());
}

void HeapPage::next(size_t &slot) const {
    // TODO pa1
    do {
        slot++;
    } while (slot < capacity && empty(slot));
}

bool HeapPage::empty(size_t slot) const {
    // TODO pa1
    if (slot >= capacity) return true;
    uint8_t byte = header[slot / 8];
    int bitPos = 7 - (slot % 8);  // Use big-endian bit ordering.
    return !(byte & (1 << bitPos));
}
