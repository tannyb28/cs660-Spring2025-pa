#include <db/Database.hpp>

using namespace db;

BufferPool &Database::getBufferPool() { return bufferPool; }

Database &db::getDatabase() {
    static Database instance;
    return instance;
}

void Database::add(std::unique_ptr<DbFile> file) {
    std::string name = file->getName();
    if (files_.count(name)) {
        throw std::logic_error("File already exists in the database");
    }
    files_[name] = std::move(file);
}

std::unique_ptr<DbFile> Database::remove(const std::string &name) {
    auto it = files_.find(name);
    if (it == files_.end()) {
        throw std::logic_error("File does not exist in the database");
    }
    // Flush any dirty pages for this file.
    bufferPool.flushFile(name);
    // Remove the file from the catalog and return it.
    std::unique_ptr<DbFile> removedFile = std::move(it->second);
    files_.erase(it);
    return removedFile;
}

DbFile &Database::get(const std::string &name) const {
    auto it = files_.find(name);
    if (it == files_.end()) {
        throw std::logic_error("File does not exist in the database");
    }
    return *(it->second);
}