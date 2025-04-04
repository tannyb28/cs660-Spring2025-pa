#include <cstring>
#include <db/BTreeFile.hpp>
#include <db/LeafPage.hpp>
#include <iostream>
#include <stdexcept>

// This class is responsible for all functions related to the Leaf Page which includes
//activities like inserting a tuple inside a leaf page, splitting a leaf page and getting
//a tuple from a Leaf Node in a B tree.

using namespace db;

LeafPage::LeafPage(Page &page, const TupleDesc &td, size_t key_index) : td(td), key_index(key_index) {
    // TODO pa2
    header = reinterpret_cast<LeafPageHeader *>(page.data());
    data = reinterpret_cast<uint8_t *>(header + 1);
    capacity = (DEFAULT_PAGE_SIZE - sizeof(LeafPageHeader)) / td.length();
    header->size = 0;
    header->next_leaf = -1;
}

bool LeafPage::insertTuple(const Tuple &t) {
    // TODO pa2
    int key = std::get<int>(t.get_field(key_index));
    int tuple_size = td.length();

    if (header->size >= capacity) {
        uint8_t *pos = data;
        for (int i = 0; i < header->size; ++i) {
        Tuple current_tuple = td.deserialize(pos);
        int current_key = std::get<int>(current_tuple.get_field(key_index));

        if (current_key == key) {
            td.serialize(pos, t);
            return true;
        }
        pos += tuple_size;
        }
        return true;
    }

    uint8_t *insert_pos = data;
    int i = 0;
    while (i < header->size) {
        Tuple current_tuple = td.deserialize(insert_pos);
        int current_key = std::get<int>(current_tuple.get_field(key_index));

        if (current_key == key) {
        td.serialize(insert_pos, t);
        return false;
        }

        if (current_key > key) break;

        insert_pos += tuple_size;
        ++i;
    }

    if (i < header->size) {
        memmove(insert_pos + tuple_size, insert_pos, (header->size - i) * tuple_size);
    }

    td.serialize(insert_pos, t);
    header->size++;

    return header->size == capacity;
}

int LeafPage::split(LeafPage &new_page) {
    // TODO pa2
    if (new_page.header->size != 0) {
        throw std::invalid_argument("New page must be empty for split");
    }

    int split_index = header->size / 2;

    uint8_t *split_pos = data + split_index * td.length();
    uint8_t *new_data = new_page.data;

    int num_tuples_to_move = header->size - split_index;

    std::memcpy(new_data, split_pos, num_tuples_to_move * td.length());

    new_page.header->size = num_tuples_to_move;
    new_page.header->next_leaf = header->next_leaf;

    header->size = split_index;
    header->next_leaf = new_page.header->page_ids;

    Tuple first_tuple_in_new_page = td.deserialize(new_data);
    int split_key = std::get<int>(first_tuple_in_new_page.get_field(key_index));

    return split_key;
}

Tuple LeafPage::getTuple(size_t slot) const {
    // TODO pa2
    if (slot >= header->size) {
        throw std::out_of_range("Slot index out of range");
    }

    uint8_t *tuple_pos = data + slot * td.length();
    Tuple tuple = td.deserialize(tuple_pos);

    return tuple;
}