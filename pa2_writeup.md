# PA2 Writeup
Tanish Bhowmick & Apoorva Gupta

## Question 1
Maintaining an index of a file can slow down insertions and deletions as the index needs to be updated. However, it can speed up lookups. What are some strategies to minimize the impact of index maintenance on bulk insertions and deletions? What do you need to change in the implementation to support these strategies?

### Answer:
Some potentially strategies include:
1. Deferred/Bulk Updates: We could accumulate changes in a staging area or buffer and update the index in one batch. We could do this by adding a bulk mode flag, a temporary buffer, and a batch load.

2. Periodic Deletion: We could mark entries as deleted instead of immediately removing them and then implement some periodic maintenance to remove then rebuild/rebalance the tree. To implement this we could add some deletion flag to tuples or index entries and implement a cleanup routine to periodically purge entries flagged for deletion and rebalance the tree. We would also have to adjust traversal logic to skip over flagged records.

3. Buffered Writes: We could use a memtable to collect a number of operations before writing them out in a single batch, reducing the number of times we need to traverse and update tree structure. We could do this by integrating a memtable that caches incoming operations and some mechanism to merge the operations and apply them to the tree in bulk.


## Question 2
A common workload for database tables is to insert new entries with an auto-incrementing key. How can you optimize the BTreeFile for this workload?
### Answer:
Since the key is auto-incrementing, we know that every new key is larger than all existing ones. To optimize for this pattern, we can add a pointer that tracks the rightmost leaf. This would allow us to avoid traversing the tree from root for every insertion but rather point directy to rightmost leaf.

## Question 3
A common strategy employed in production systems is to maintain the internal nodes of indexes to always exist in the bufferpool (or rather, pin them to memory). Discuss why this is a good idea and if there are any implications of this strategy.

### Answer:
Pros:
- This helps with frequent access during tree traversal because internal nodes are used in every lookup, insertion, and deletion in order to direct the search path.
- Internal nodes are usually smaller than leaf pages and the number of nodes is lower that data pages meaning that pinning these should leave a smaller memory footprint.
- If internal nodes are available in memory, the number of disk accesses per query would be lower so the performance in general should be better especially under heavier load.

Considerations:
- Need to make sure the buffer pool is managed properly to ensure pinning internal nodes doesn't starve other pages of memory.
- Internal nodes are continuously in use for directing the search path so updates during splits and rebalancing must be handled carefully to update properly.
- Pinning the internal nodes would provide significant performance gains for heavier workloads but this comes with higher memory utilization so there needs to be careful management of memory to support pinned internal nodes and also workload on the leaf pages.

## Write Up

# Design Choices
The following design choices are made while writing this code.
- Insertion tracks the path in top to down approach in the B+Tree this allows splits to go up to the root.
- To manage bufferpool consistency we mark pages as dirty after every change.
- If leaf node is full it splits itself into two copying half the tuples over.
- For versatality and simplicityThe, the Iterator is used instead of a pointers.
- If the root gets filled a new root gets created at the same time and does not go for lazy splitting.

# Missing / Incomplete Implementations
- Our implementation works on the assumption that the root is small initially, missing a general case when root can grow dynamically as well.
- It does not check for errors no check for null pages or failed page fetches.
- Actual B+Trees implementations  need locks during insertion and splits which is missing in our implementations.

  ## Contributions
  - Apoorva Gupta - Code review and debugging
  - Tanish Bhowmick - Code written
  It took us ~15 Hours to complete this assignment and implementation of the B+ Tree was the hardest part overall, however assignment was decently challenging and gave us a good overall learning experience.
