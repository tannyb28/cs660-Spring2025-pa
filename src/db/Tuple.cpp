#include <cstring>
#include <db/Tuple.hpp>
#include <stdexcept>

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
    // TODO pa1
}

bool TupleDesc::compatible(const Tuple &tuple) const {
    // TODO pa1
    throw std::runtime_error("not implemented");
}

size_t TupleDesc::index_of(const std::string &name) const {
    // TODO pa1
}

size_t TupleDesc::offset_of(const size_t &index) const {
    // TODO pa1
}

size_t TupleDesc::length() const {
    // TODO pa1
}

size_t TupleDesc::size() const {
    // TODO pa1
}

Tuple TupleDesc::deserialize(const uint8_t *data) const {
    // TODO pa1
}

void TupleDesc::serialize(uint8_t *data, const Tuple &t) const {
    // TODO pa1
}

db::TupleDesc TupleDesc::merge(const TupleDesc &td1, const TupleDesc &td2) {
    // TODO pa1
}
