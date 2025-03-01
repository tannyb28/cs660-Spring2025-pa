# Deleting tuples from a HeapFile might lead to having empty pages. How can we avoid having empty pages in a file? What is the cost of the solution you propose?

Deleting tuples from a HeapFile might lead to having empty pages. How can we avoid having empty pages in a file? What is the cost of the solution you propose?

## Solution

To avoid having empty pages in the file, we could modify deleteTuple to check if a page becomes empty after a tuple is deleted. If the page is empty, we can:

1. Set the page as free and instead of removing it from the file, we can mark it as reusable for future insertions. This will also stabilize the memory usage.
2. We can also check for any marked free pages and reuse them before appending a new page in insertTuple.

## Cost of the Solution

The cost of this solution will have some trade-offs as:

1. Slight increase due to tracking free pages will increase the storage overhead.
2. As we reuse empty pages, insertion performance will improve in time complexity and stay stable in terms of space.
3. However, deletion performance will increase slightly as we have to check for empty pages and update the free list.
4. Reading will improve because it reduces unnecessary empty pages, making sequential scans faster.

# In this assignment we have fixed size fields. How can we support variable size fields (e.g. VARCHAR)?

We can support variable-size fields (VARCHAR) by:
1. Instead of using fixed offsets for tuples, maintain a slot directory that tracks tuple offsets dynamically. The slot directory can store pairs for each tuple.
2. Store tuples contiguously from the end of the page like heap and declare a free space pointer that tracks where new tuples should be inserted.
3. Instead of writing at a fixed slot, find space at the end of the page and update the slot directory.
4. Use the slot directory to determine where the tuple starts and its length.


