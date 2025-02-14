#include <db/Database.hpp>
#include <db/HeapFile.hpp>
#include <db/HeapPage.hpp>
#include <stdexcept>

using namespace db;

HeapFile::HeapFile(const std::string &name, const TupleDesc &td) : DbFile(name, td) {}

void HeapFile::insertTuple(const Tuple &t) {
    // TODO pa1
}

void HeapFile::deleteTuple(const Iterator &it) {
    // TODO pa1
}

Tuple HeapFile::getTuple(const Iterator &it) const {
    // TODO pa1
}

void HeapFile::next(Iterator &it) const {
    // TODO pa1
}

Iterator HeapFile::begin() const {
    // TODO pa1
}

Iterator HeapFile::end() const {
    // TODO pa1
}
