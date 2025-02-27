#include <db/DbFile.hpp>
#include <stdexcept>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstring>

using namespace db;

const TupleDesc &DbFile::getTupleDesc() const { return td; }

DbFile::DbFile(const std::string &name, const TupleDesc &td) : name(name), td(td) {
    // TODO pa1: open file and initialize numPages
    fd = open(name.c_str(), O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        throw std::runtime_error("Failed to open file");
    }
    struct stat st;
    if (fstat(fd, &st) < 0) {
        close(fd);
        throw std::runtime_error("fstat failed");
    }
    size_t fileSize = st.st_size;
    if (fileSize == 0) {
        // New file: always allocate one page.
        numPages = 1;
        Page emptyPage = {};
        if (write(fd, emptyPage.data(), DEFAULT_PAGE_SIZE) != (ssize_t)DEFAULT_PAGE_SIZE) {
            close(fd);
            throw std::runtime_error("Failed to initialize new file with one page");
        }
    } else {
        numPages = fileSize / DEFAULT_PAGE_SIZE;
    }
}

DbFile::~DbFile() {
    // TODO pa1: close file
    if (fd >= 0) {
        close(fd);
    }
}

const std::string &DbFile::getName() const { return name; }

void DbFile::readPage(Page &page, const size_t id) const {
    reads.push_back(id);
    // TODO pa1: read page
    ssize_t bytesRead = pread(fd, page.data(), DEFAULT_PAGE_SIZE, id * DEFAULT_PAGE_SIZE);
    if (bytesRead != DEFAULT_PAGE_SIZE) {
        throw std::runtime_error("Failed to read page");
    }
}

void DbFile::writePage(const Page &page, const size_t id) const {
    writes.push_back(id);
    // TODO pa1: write page
    ssize_t bytesWritten = pwrite(fd, page.data(), DEFAULT_PAGE_SIZE, id * DEFAULT_PAGE_SIZE);
    if (bytesWritten != DEFAULT_PAGE_SIZE) {
        throw std::runtime_error("Failed to write page");
    }
}

const std::vector<size_t> &DbFile::getReads() const { return reads; }

const std::vector<size_t> &DbFile::getWrites() const { return writes; }

void DbFile::insertTuple(const Tuple &t) { throw std::runtime_error("Not implemented"); }

void DbFile::deleteTuple(const Iterator &it) { throw std::runtime_error("Not implemented"); }

Tuple DbFile::getTuple(const Iterator &it) const { throw std::runtime_error("Not implemented"); }

void DbFile::next(Iterator &it) const { throw std::runtime_error("Not implemented"); }

Iterator DbFile::begin() const { throw std::runtime_error("Not implemented"); }

Iterator DbFile::end() const { throw std::runtime_error("Not implemented"); }

size_t DbFile::getNumPages() const { return numPages; }
