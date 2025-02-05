#include <db/BufferPool.hpp>
#include <db/Database.hpp>
#include <numeric>

using namespace db;

BufferPool::BufferPool() {
    // Initialize free slots with indices 0 to DEFAULT_NUM_PAGES-1.
    for (size_t i = 0; i < DEFAULT_NUM_PAGES; ++i) {
        free_slots_.push_back(i);
    }
}

BufferPool::~BufferPool() {
    // Flush all dirty pages when the BufferPool is destroyed.
    std::vector<PageId> toFlush(dirty_pages_.begin(), dirty_pages_.end());
    for (const auto &pid : toFlush) {
        flushPage(pid);
    }
}

Page &BufferPool::getPage(const PageId &pid) {
    if (contains(pid)) {
        // If already present, update LRU (move to front).
        lru_list_.erase(lru_map_[pid]);
        lru_list_.push_front(pid);
        lru_map_[pid] = lru_list_.begin();
        return pages_[page_table_.at(pid)];
    }

    // Page is not in the buffer pool.
    size_t slot;
    if (!free_slots_.empty()) {
        slot = free_slots_.front();
        free_slots_.pop_front();
    } else {
        // Buffer full: evict the least-recently used page (back of the LRU list).
        PageId evictPid = lru_list_.back();
        lru_list_.pop_back();
        lru_map_.erase(evictPid);
        slot = page_table_.at(evictPid);
        if (isDirty(evictPid)) {
            flushPage(evictPid);
        }
        page_table_.erase(evictPid);
        dirty_pages_.erase(evictPid);
    }

    // Load the page from disk.
    Database &db = getDatabase();
    DbFile &file = db.get(pid.file);
    file.readPage(pages_[slot], pid.page);

    // Insert the new page into our tracking structures.
    page_table_[pid] = slot;
    lru_list_.push_front(pid);
    lru_map_[pid] = lru_list_.begin();
    return pages_[slot];
}

void BufferPool::markDirty(const PageId &pid) {
    if (!contains(pid)) {
        throw std::logic_error("Page not in buffer pool");
    }
    dirty_pages_.insert(pid);
}

bool BufferPool::isDirty(const PageId &pid) const {
    if (page_table_.find(pid) == page_table_.end()) {
        throw std::runtime_error("PageId is not in the buffer pool");
    }
    return dirty_pages_.find(pid) != dirty_pages_.end();
}

bool BufferPool::contains(const PageId &pid) const {
    return page_table_.find(pid) != page_table_.end();
}

void BufferPool::discardPage(const PageId &pid) {
    auto it = page_table_.find(pid);
    if (it == page_table_.end()) {
        return; // Nothing to do.
    }
    size_t slot = it->second;
    page_table_.erase(it);
    auto lruIt = lru_map_.find(pid);
    if (lruIt != lru_map_.end()) {
        lru_list_.erase(lruIt->second);
        lru_map_.erase(lruIt);
    }
    dirty_pages_.erase(pid);
    free_slots_.push_back(slot);
}

void BufferPool::flushPage(const PageId &pid) {
    if (!contains(pid)) {
        return;
    }

    if (!isDirty(pid)) {
        return;
    }

    size_t slot = page_table_.at(pid);
    Database &db = getDatabase();
    DbFile &file = db.get(pid.file);
    file.writePage(pages_[slot], pid.page);
    dirty_pages_.erase(pid);
}

void BufferPool::flushFile(const std::string &file) {
    // Note: Corrected the variable name from an undefined 'fileName' to 'file'.
    std::vector<PageId> pagesToFlush;
    for (const auto &entry : page_table_) {
        const PageId &pid = entry.first;
        if (pid.file == file && isDirty(pid)) {
            pagesToFlush.push_back(pid);
        }
    }
    for (const auto &pid : pagesToFlush) {
        flushPage(pid);
    }
}