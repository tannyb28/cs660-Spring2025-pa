#include <db/IndexPage.hpp>
#include <iostream>
#include <stdexcept>


//This class is responsible for handling all activities related to an index page.
//The constructor create an IndexPage node and then calculates it's capcaity.
//This class also deals with inserting new index pages into the tree and the scenario
//when the page is full and a split is needed.
using namespace db;

IndexPage::IndexPage(Page &page) {
    // TODO pa2
    header = reinterpret_cast<IndexPageHeader *>(page.data());
    keys = reinterpret_cast<int *>(page.data() + sizeof(IndexPageHeader));
    size_t page_size = DEFAULT_PAGE_SIZE;
    size_t available_space = page_size - sizeof(IndexPageHeader) - 1;
    capacity = available_space / (sizeof(int) + sizeof(size_t));
    children = reinterpret_cast<size_t *>(page.data() + sizeof(IndexPageHeader) + capacity * sizeof(int));
    if (header->size == 0) {
        header->size = 0;
  }
}

bool IndexPage::insert(int key, size_t child) {
    // TODO pa2
    if (header->size >= capacity) {
        return true;
    }

    auto it = std::lower_bound(keys, keys + header->size, key);
    size_t pos = std::distance(keys, it);

    if (pos < header->size) {
        std::move_backward(keys + pos, keys + header->size, keys + header->size + 1);
        std::move_backward(children + pos + 1, children + header->size + 1, children + header->size + 2);
    }

    keys[pos] = key;
    children[pos + 1] = child;

    header->size++;

    return header->size == capacity;
}

int IndexPage::split(IndexPage &new_page) {
    // TODO pa2
    size_t mid = header->size / 2;
    new_page.header->size = header->size - mid - 1;
    std::copy(keys + mid + 1, keys + header->size, new_page.keys);
    std::copy(children + mid + 1, children + header->size + 1, new_page.children);
    header->size = mid;
    new_page.children[0] = children[mid + 1];
    return keys[mid];
}