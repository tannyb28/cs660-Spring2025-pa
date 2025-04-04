#include <cstring>
#include <db/BTreeFile.hpp>
#include <db/Database.hpp>
#include <db/IndexPage.hpp>
#include <db/LeafPage.hpp>
#include <iostream>
#include <stdexcept>

using namespace db;

BTreeFile::BTreeFile(const std::string &name, const TupleDesc &td, size_t key_index)
        : DbFile(name, td), key_index(key_index) {}

void BTreeFile::insertTuple(const Tuple &t) {
    // TODO pa2
    int key = std::get<int>(t.get_field(key_index));
    parent_page_id = root_id;

    BufferPool &bufferPool = db::getDatabase().getBufferPool();
    PageId rootPageId = {name, root_id};
    Page &rootPage = bufferPool.getPage(rootPageId);

    IndexPage currentIndexPage(rootPage);
    Page currentPage = rootPage;
    bool isLeaf = !currentIndexPage.header->index_children;


    while (!isLeaf) {
        IndexPage indexPage(currentPage);
        int *keys = indexPage.keys;
        size_t *children = indexPage.children;

        size_t childIndex = 0;
        while (childIndex < indexPage.header->size && keys[childIndex] < key) {
            ++childIndex;
        }
        PageId childPageId = {name, children[childIndex]};
        if(current_page_id != root_id) {
          parent_page_id= current_page_id;
        }
        current_page_id= childPageId.page;
        currentPage = bufferPool.getPage(childPageId);
        currentIndexPage = IndexPage(currentPage);
        isLeaf = !currentIndexPage.header->index_children;
    }

    LeafPage leafPage(currentPage, td, key_index);
    bool needsSplit = leafPage.insertTuple(t);

    bufferPool.markDirty(PageId{name, current_page_id});

    if (needsSplit) {
        PageId newLeafPageId = {name, new_page_value};//What should the value be for the new page being created?
        new_page_value++;
        Page &newLeafPage = bufferPool.getPage(newLeafPageId);
        LeafPage newLeaf(newLeafPage, td, key_index);
        int splitKey = leafPage.split(newLeaf);

        bufferPool.markDirty(PageId{name, new_page_value});

        Page parentPage = rootPage;
        if (current_page_id == root_id && currentIndexPage.header->size == 0) {
            PageId newRootPageId = {name, new_page_value};
            new_page_value++;
            Page &newRootPage = bufferPool.getPage(newRootPageId);
            IndexPage newRoot(newRootPage);
            newRoot.header->index_children = true;
            newRoot.insert(splitKey, --new_page_value);

            newRoot.children[0] = current_page_id;
            bufferPool.markDirty(PageId{name, root_id});
        } else {
            bool parentNeedsSplit = currentIndexPage.insert(splitKey, --new_page_value);
            bufferPool.markDirty(PageId{name, parent_page_id});

            while (parentNeedsSplit) {
                PageId newIndexPageId = {name, new_page_value};
                new_page_value++;
                Page &newIndexPage = bufferPool.getPage(newIndexPageId);
                IndexPage newIndex(newIndexPage);
                int parentSplitKey = currentIndexPage.split(newIndex);

                bufferPool.markDirty({name, newIndexPageId.page});
                bufferPool.markDirty(PageId{name, current_page_id});

                if (parent_page_id == root_id) {
                    PageId newRootPageId = {name, new_page_value};
                    new_page_value++;
                    Page &newRootPage = bufferPool.getPage(newRootPageId);
                    IndexPage newRoot(newRootPage);
                    newRoot.header->index_children = true;
                    newRoot.insert(parentSplitKey, newIndexPageId.page);
                    newRoot.children[0] = parent_page_id;
                    bufferPool.markDirty({name, root_id});
                    break;
                } else {
                    PageId parentPageId = {name, current_page_id};
                    parentPage = bufferPool.getPage(parentPageId);
                    currentIndexPage = IndexPage(parentPage);
                    parentNeedsSplit = currentIndexPage.insert(parentSplitKey, new_page_value);
                    new_page_value++;
                    bufferPool.markDirty({name, parent_page_id});
                }
            }
        }
    }
}

void BTreeFile::deleteTuple(const Iterator &it) {
    // Do not implement
}

Tuple BTreeFile::getTuple(const Iterator &it) const {
    // TODO pa2
    BufferPool &bufferPool = db::getDatabase().getBufferPool();
    PageId pageId = {name, it.page};
    Page &page = bufferPool.getPage(pageId);

    LeafPage leafPage(page, td, key_index);
    if (it.slot >= leafPage.header->size) {
        throw std::out_of_range("Iterator slot out of range");
    }

    return leafPage.getTuple(it.slot);
}

void BTreeFile::next(Iterator &it) const {
    // TODO pa2
    BufferPool &bufferPool = db::getDatabase().getBufferPool();
    PageId pageId = {name, it.page};
    Page &page = bufferPool.getPage(pageId);

    LeafPage leafPage(page, td, key_index);
    ++it.slot;

    if (it.slot >= leafPage.header->size) {
        // Move to the next leaf page
        if (leafPage.header->next_leaf == 0) { // No more pages
        it.page = 0;
        it.slot = -1;
        } else {
        it.page = leafPage.header->next_leaf;
        it.slot = 0;
        }
    }
}

Iterator BTreeFile::begin() const {
    // TODO pa2
    BufferPool &bufferPool = db::getDatabase().getBufferPool();
    PageId currentPageId = {name, root_id};
    Page &currentPage = bufferPool.getPage(currentPageId);

    // Traverse to the leftmost leaf
    IndexPage currentIndexPage(currentPage);
    while (currentIndexPage.header->index_children) {
        currentPageId.page = currentIndexPage.children[0];
        currentPage = bufferPool.getPage(currentPageId);
        currentIndexPage = IndexPage(currentPage);
    }

    LeafPage leafPage(currentPage, td, key_index);
    if (leafPage.header->size > 0) {
        return Iterator(*this, currentPageId.page, 0);
    } else {
        return end();
    }
}

Iterator BTreeFile::end() const {
    // TODO pa2
    return Iterator(*this, 0, -1);
}