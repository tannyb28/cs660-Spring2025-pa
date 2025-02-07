## Design Choices

The `BufferPool` class serves as a temporary memory manager for database pages by using an **LRU (Least Recently Used) caching policy** to manage a fixed number of pages in memory. The following design choices were made:

1. **Fixed-size buffer with LRU eviction**  
   - The buffer pool maintains a fixed number of pages.  
   - Pages are evicted using an **LRU list (`lru_list_`)**, where recently accessed pages are moved to the front, and the least-recently-used page is evicted when space is needed.

2. **Optimized Lookup for Pages**  
   - A **hash table (`page_table_`)** is used to quickly determine whether a page is in memory in **O(1) time**.  
   - An **LRU map (`lru_map_`)** tracks iterators in the LRU list for fast access.

3. **Dirty Page Tracking**  
   - An **unordered set (`dirty_pages_`)** maintains the list of modified pages, ensuring that only modified pages are written back to disk.

4. **Page Fetching and Eviction**  
   - If a page is requested and available in memory, it is moved to the front of the LRU list.  
   - If the page is not in memory:
     - If a **free slot exists**, it is used.  
     - If **no free slots exist**, the least recently used (LRU) page is **evicted and written to disk** (if dirty) before being replaced.

5. **Interaction with Database**  
   - Pages are **loaded from disk** via `Database::get()`, which retrieves the corresponding `DbFile`.  
   - Pages are **written back to disk** using `flushPage()` and `flushFile()` methods.

---

## Missing or Incomplete Elements

1. **Concurrency Control**  
   - The current implementation **lacks thread safety**, meaning multiple threads accessing the buffer pool simultaneously could cause race conditions.

2. **Page Pinning Mechanism**  
   - A page may be evicted even if it is actively in use.  
   - A **pinning mechanism** should be implemented to ensure that pinned pages are not evicted.

3. **Error Handling in Disk Operations**  
   - `file.readPage()` and `file.writePage()` assume that disk operations always succeed.  
   - **Exception handling** should be added to detect disk failures.

---

## Analytical Questions

### 1. Strategy to Reduce the Number of Dirty Pages Evicted  

Evicting a dirty page is **more expensive** than evicting a clean page since it must be **written back to disk** before eviction, causing additional I/O overhead. Strategies to reduce dirty page evictions:

1. **Batch Flushing**  
   - Instead of writing a dirty page **every time** it's evicted, store them in a **vector of fixed length**.  
   - Flush multiple dirty pages together to reduce I/O operations.

2. **Background Flushing Job**  
   - Periodically **scan the buffer pool** for dirty pages and flush them before eviction.

3. **Adaptive Flushing Based on Dirty Page Ratio**  
   - Introduce a **threshold** (e.g., if more than **70%** of the buffer pool contains dirty pages, start flushing them before eviction).

These strategies help in **reducing eviction time** and **improving overall database performance**.

---

### 2. Implications of Leaving Pages in the Buffer Pool After a File is Removed

If a file is removed, but its pages remain in the buffer pool, the following issues may arise:

1. **Dangling References**  
   - The buffer pool may still contain pages from a **deleted file**, leading to crashes when accessed.

2. **Stale Data Usage**  
   - If a **new file with the same name** is added, the buffer pool might **return old data** from the deleted file.

3. **Memory Leak and Reduced Performance**  
   - The buffer pool will **hold pages of non-existent files**, reducing available slots for active pages.

**Possible Bug**  
- The `Database::remove()` function **flushes dirty pages** but **does not discard them from the buffer pool**.
- **Fix**: After flushing, explicitly **discard all pages associated with the removed file**.

---

### 3. When Should a Page Be Discarded Without Writing It Back to Disk?

A page should be discarded **without flushing** in the following scenarios:

1. **Transaction Rollback**  
   - If a transaction **modifies a page but is later aborted**, the changes should be discarded.

2. **Page Deletion**  
   - When a page is deleted, it should be removed from memory **without being written back to disk**.

3. **Corrupt or Invalid Pages**  
   - If a **corrupt page** is detected, it should be discarded to prevent **overwriting valid data**.

**Our Code Implementation**  
- The `discardPage()` method **correctly removes a page from memory without flushing it to disk**, ensuring safety in these scenarios.

---

## Time Spent on the Assignment  

- **Apoorva Gupta**: Understanding the codebase and debugging  
- **Tanish Bhowmich**: Writing the code  
- **Total time spent**: ~20 hours  

The problem statement was well-structured, and we efficiently implemented the solution. However, some **helper code lacked clear explanations for variable and datatype choices**, making it slightly confusing.
