#include <cstring>
#include <db/Tuple.hpp>
#include <stdexcept>
#include <unordered_set>

using namespace db;

Tuple::Tuple(const std::vector<field_t> &fields) : fields(fields) {}

type_t Tuple::field_type(size_t i) const {
    const field_t &field = fields.at(i);
    if (std::holds_alternative<int>(field)) {
        return type_t::INT;
    }
    if (std::holds_alternative<double>(field)) {
        return type_t::DOUBLE;
    }
    if (std::holds_alternative<std::string>(field)) {
        return type_t::CHAR;
    }
    throw std::logic_error("Unknown field type");
}

size_t Tuple::size() const { return fields.size(); }

const field_t &Tuple::get_field(size_t i) const { return fields.at(i); }

TupleDesc::TupleDesc(const std::vector<type_t> &types, const std::vector<std::string> &names) {
    if (types.size() != names.size())
        throw std::logic_error("Types and names must have the same length");

    // Ensure field names are unique.
    std::unordered_set<std::string> uniqueNames;
    for (const auto &n : names) {
        if (!uniqueNames.insert(n).second)
            throw std::logic_error("Field names must be unique");
    }

    this->types = types;
    this->names = names;
    tupleLength = 0;
    offsets.resize(types.size());
    for (size_t i = 0; i < types.size(); ++i) {
        offsets[i] = tupleLength;
        switch (types[i]) {
            case type_t::INT:
                tupleLength += INT_SIZE;
                break;
            case type_t::DOUBLE:
                tupleLength += DOUBLE_SIZE;
                break;
            case type_t::CHAR:
                tupleLength += CHAR_SIZE;
                break;
            default:
                throw std::logic_error("Unsupported type in TupleDesc");
        }
    }
}

// Returns true if the tuple's number of fields and field types match this schema.
bool TupleDesc::compatible(const Tuple &tuple) const {
    if (tuple.size() != types.size()) return false;
    for (size_t i = 0; i < types.size(); i++) {
        if (tuple.field_type(i) != types[i])
            return false;
    }
    return true;
}

// Returns the index of the field with the given name.
size_t TupleDesc::index_of(const std::string &name) const {
    for (size_t i = 0; i < names.size(); i++) {
        if (names[i] == name)
            return i;
    }
    throw std::logic_error("Field name not found in TupleDesc");
}

// Returns the byte offset of the field at the given index.
size_t TupleDesc::offset_of(const size_t &index) const {
    if (index >= offsets.size()) {
        throw std::logic_error("Index out of bounds in TupleDesc::offset_of");
    }
    return offsets[index];
}

// Returns the total length (in bytes) required to store a tuple.
size_t TupleDesc::length() const {
    return tupleLength;
}

// Returns the number of fields.
size_t TupleDesc::size() const {
    return types.size();
}

// Deserializes a tuple from the given byte buffer.
Tuple TupleDesc::deserialize(const uint8_t *data) const {
    std::vector<field_t> fields;
    size_t offset = 0;
    for (size_t i = 0; i < types.size(); i++) {
        switch (types[i]) {
            case type_t::INT: {
                int value;
                std::memcpy(&value, data + offset, INT_SIZE);
                fields.push_back(value);
                offset += INT_SIZE;
                break;
            }
            case type_t::DOUBLE: {
                double value;
                std::memcpy(&value, data + offset, DOUBLE_SIZE);
                fields.push_back(value);
                offset += DOUBLE_SIZE;
                break;
            }
            case type_t::CHAR: {
                char buffer[CHAR_SIZE + 1];
                std::memcpy(buffer, data + offset, CHAR_SIZE);
                buffer[CHAR_SIZE] = '\0';
                fields.push_back(std::string(buffer));
                offset += CHAR_SIZE;
                break;
            }
            default:
                throw std::logic_error("Unsupported type in deserialize");
        }
    }
    return Tuple(fields);
}

// Serializes the tuple into the given byte buffer.
void TupleDesc::serialize(uint8_t *data, const Tuple &t) const {
    if (t.size() != types.size()) {
        throw std::logic_error("Tuple size does not match TupleDesc");
    }
    size_t offset = 0;
    for (size_t i = 0; i < types.size(); i++) {
        switch (types[i]) {
            case type_t::INT: {
                int value = std::get<int>(t.get_field(i));
                std::memcpy(data + offset, &value, INT_SIZE);
                offset += INT_SIZE;
                break;
            }
            case type_t::DOUBLE: {
                double value = std::get<double>(t.get_field(i));
                std::memcpy(data + offset, &value, DOUBLE_SIZE);
                offset += DOUBLE_SIZE;
                break;
            }
            case type_t::CHAR: {
                std::string value = std::get<std::string>(t.get_field(i));
                std::memset(data + offset, 0, CHAR_SIZE);
                std::memcpy(data + offset, value.c_str(), std::min(value.size(), (size_t)CHAR_SIZE));
                offset += CHAR_SIZE;
                break;
            }
            default:
                throw std::logic_error("Unsupported type in serialize");
        }
    }
}

// Merges two TupleDescs into one, concatenating their field types and names.
db::TupleDesc TupleDesc::merge(const TupleDesc &td1, const TupleDesc &td2) {
    std::vector<type_t> mergedTypes;
    std::vector<std::string> mergedNames;
    for (size_t i = 0; i < td1.size(); i++) {
        mergedTypes.push_back(td1.types[i]);
        mergedNames.push_back(td1.names[i]);
    }
    for (size_t i = 0; i < td2.size(); i++) {
        mergedTypes.push_back(td2.types[i]);
        mergedNames.push_back(td2.names[i]);
    }
    return TupleDesc(mergedTypes, mergedNames);
}