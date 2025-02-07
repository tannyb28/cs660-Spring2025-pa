The BufferPool class serves as a temporary memory manager for database pages by using an LRU (Least Recently Used) caching policy to manage a fixed number of pages in random memory. We made following design choices 
Fixed-size buffer with LRU eviction - The buffer pool maintains a fixed number of pages and the pages are evicted using an LRU list (lru_list_), where recently accessed pages are moved to the front, and the least-recently-used page is evicted when space is needed.
Optimised Lookup for pages - A hash table (page_table_) is used to quickly determine whether a page is in memory in O(N). An LRU map (lru_map_) tracks iterators in the LRU list for fast access.
Dirty Page Tracking - An unordered set (dirty_pages_) maintains the list of modified pages, ensuring that only modified pages are written back to disk.
Page Fetching and Eviction - If a page is requested and available in memory, it is moved to the front of the LRU list. If the page is not in memory:
If a free slot exists, it is used.
If no free slots exist, the last used page is evicted and written to the disk in case isDirty and replaced.
Interaction with Database - Pages are loaded from disk via class member Database::get(), which retrieves the corresponding DbFile and are written back to disk using flushPage() and flushFile() methods
Missing or Incomplete Elements
Since we do not have a concurrency control in place. Our implementation might not work in case of pages added at the same time or we can say case multiple threads access the BufferPool Page 
Currently, a page may be evicted even if it is actively in use . We should have a pinning mechanism to ensure that pinned pages are not evicted.
Error Handling in Disk Operations file.readPage() and file.writePage() methods work on the everytime the operation will succeed. It can be resolved by exception handling should be added to detect disk failures.

In total we spent around 20 hours on this assignment with Apoorva Gupta understanding the code base and debugging and Tanish Bhowmich writing the code. Overall problem statement was well structured and we were able to write code efficiently however the reasoning for variables and datatypes used in the helper code was not extensive. This confused us at times to answer why a particular step is done in the helper code otherwise it was an interesting problem statement.
Analytical Questions
Evicting a dirty page is more expensive than evicting a clean page. Propose a strategy to reduce the number of dirty pages that are evicted.
Evicting a dirty page is more expensive than evicting a clean page because the dirty page must be written back to disk before eviction, incurring additional I/O overhead due to writing and taking additional memory space. We can use following ways to reduce the number of evictions
We can do batch flushing in which a vector of fixed length will store the information of dirty pages instead of writing a single page to disc everytime encountered, here the I/O operations for disk writing will be reduced and multiple dirty pages from a group will be flushed together.
We can have a background flushing job that will scan the buffer pool periodically for dirty pages and flush them to disk, it prevents the writing of multiple dirty pages just before eviction reducing eviction time.
To improve the background flushing process, we can use an adaptive approach based on dirty page ratio i.e we can set a threshold for the buffer pool and if dirty page percentage let's say increases by more than 70%  we will start the flushing dirty page job before evicting them.
With these strategies, we can significantly reduce the number of dirty pages that need to be evicted from the system, thereby improving buffer pool performance and reducing disk I/O overhead.
The Database::remove flushes any dirty pages associated with the file before it is removed. What are the implications of leaving the pages in the buffer pool when a file is removed? Can you identify a possible bug?
If the pages associated with a file remain in the buffer pool after the file is removed, following issues might arise
Since the buffer pool is still containing pages from the file that no longer exist we will get dangling references. If we try to access these pages, it will lead to erroneous behaviour or crashes due to accessing invalid memory
It will lead to stale data usage in case of new file being added with same name, the buffer pool will still reference to the old pages leading to stale or corrupted data being returned
Since buffer pool has the information from non-existent files, it will reduce the number of available slots for the other files this will lead to memory leak in bufferpool and affect the database performance
Based on the points discussed above we can have a possibility of pages getting flushed from the database but does not remove them from the buffer pool. To fix this we need to explicitly discard all pages from the buffer pool that belong to the removed file after flushing the dirty pages,
When would you need to discard a page from the buffer pool without writing it back to disk?
This can be done in following cases
Transaction Rollback - If in a transaction a change is made on a page but later those changes are aborted, any of the made changes must be discarded. Since the changes were never committed, the page should be removed from the buffer pool without writing it to disk back again.
Page Deletion - When a page or a set of pages is deleted, those pages should be directly removed from memory without writing back to disk. Writing them back would be unnecessary and wasteful since they are no longer available for querying.
Corrupted or Invalid Pages -  If we detect a page in memory to be corrupt or invalid, it should be discarded without writing it to disk to prevent overwriting. This scenario might occur due to an unexpected crash, or logical inconsistencies.
Our code handled it as the existing discardPage method correctly removes a page from memory without flushing it to disk, ensuring that pages can be discarded safely whenever required.


