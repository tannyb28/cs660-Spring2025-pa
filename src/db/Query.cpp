#include <db/Query.hpp>

using namespace db;

// Helper to visit variant fields
struct FieldVisitor {
    PredicateOp op;
    const field_t &rhs;
    bool operator()(int lhs, int rhs_val) const {
        switch (op) {
            case PredicateOp::EQ: return lhs == rhs_val;
            case PredicateOp::NE: return lhs != rhs_val;
            case PredicateOp::GT: return lhs  > rhs_val;
            case PredicateOp::GE: return lhs >= rhs_val;
            case PredicateOp::LT: return lhs  < rhs_val;
            case PredicateOp::LE: return lhs <= rhs_val;
        }
        return false;
    }
    bool operator()(double lhs, double rhs_val) const {
        switch (op) {
            case PredicateOp::EQ: return lhs == rhs_val;
            case PredicateOp::NE: return lhs != rhs_val;
            case PredicateOp::GT: return lhs  > rhs_val;
            case PredicateOp::GE: return lhs >= rhs_val;
            case PredicateOp::LT: return lhs  < rhs_val;
            case PredicateOp::LE: return lhs <= rhs_val;
        }
        return false;
    }
    bool operator()(const std::string &lhs, const std::string &rhs_val) const {
        switch (op) {
            case PredicateOp::EQ: return lhs == rhs_val;
            case PredicateOp::NE: return lhs != rhs_val;
            case PredicateOp::GT: return lhs  > rhs_val;
            case PredicateOp::GE: return lhs >= rhs_val;
            case PredicateOp::LT: return lhs  < rhs_val;
            case PredicateOp::LE: return lhs <= rhs_val;
        }
        return false;
    }
    template<typename T, typename U>
    bool operator()(T, U) const { return false; }
};

// Common compare function
static bool compare(const field_t &lhs, const field_t &rhs, PredicateOp op) {
    return std::visit(FieldVisitor{op, rhs}, lhs, rhs);
}

static double toDouble(const field_t &v) {
    if (std::holds_alternative<int>(v)) return static_cast<double>(std::get<int>(v));
    if (std::holds_alternative<double>(v)) return std::get<double>(v);
    throw std::logic_error("Non-numeric field for numeric comparison");
}

void db::projection(const DbFile &in, DbFile &out, const std::vector<std::string> &field_names) {
    // TODO: Implement this function
    const auto &td = in.getTupleDesc();
    for (auto it = in.begin(); it != in.end(); in.next(it)) {
        const Tuple t = in.getTuple(it);
        std::vector<field_t> fields;
        fields.reserve(field_names.size());
        for (auto &name : field_names) {
            fields.push_back(t.get_field(td.index_of(name)));
        }
        out.insertTuple(Tuple(fields));
    }
}

void db::filter(const DbFile &in, DbFile &out, const std::vector<FilterPredicate> &pred) {
    // TODO: Implement this function
    const auto &td = in.getTupleDesc();
    for (auto it = in.begin(); it != in.end(); in.next(it)) {
        const Tuple t = in.getTuple(it);
        bool keep = std::all_of(pred.begin(), pred.end(), [&](auto &p) {
            return compare(t.get_field(td.index_of(p.field_name)), p.value, p.op);
        });
        if (keep) out.insertTuple(t);
    }
}

void db::aggregate(const DbFile &in, DbFile &out, const Aggregate &agg) {
    // TODO: Implement this function
    const TupleDesc &td = in.getTupleDesc();
    bool hasGroup = agg.group.has_value();
    size_t grpIdx = hasGroup ? td.index_of(agg.group.value()) : 0;
    size_t valIdx = td.index_of(agg.field);

    struct Group { field_t key; std::vector<field_t> vals; };
    std::vector<Group> groups;

    // Collect values per group
    for (auto it = in.begin(); it != in.end(); in.next(it)) {
        Tuple t = in.getTuple(it);
        field_t key = hasGroup ? t.get_field(grpIdx) : field_t();
        field_t val = t.get_field(valIdx);
        if (!hasGroup) {
            if (groups.empty()) groups.push_back({key, {}});
            groups[0].vals.push_back(val);
        } else {
            bool found = false;
            for (auto &g : groups) {
                if (g.key == key) { g.vals.push_back(val); found = true; break; }
            }
            if (!found) groups.push_back({key, {val}});
        }
    }

    // Compute and emit
    for (auto &g : groups) {
        field_t result;
        auto &vals = g.vals;
        switch (agg.op) {
            case AggregateOp::COUNT:
                result = static_cast<int>(vals.size());
                break;
            case AggregateOp::SUM:
                if (std::holds_alternative<int>(vals[0])) {
                    int sum = 0;
                    for (auto &v : vals) sum += std::get<int>(v);
                    result = sum;
                } else if (std::holds_alternative<double>(vals[0])) {
                    double sum = 0;
                    for (auto &v : vals) sum += std::get<double>(v);
                    result = sum;
                } else {
                    throw std::logic_error("Cannot sum non-numeric field");
                }
                break;
            case AggregateOp::AVG:
                if (std::holds_alternative<int>(vals[0]) || std::holds_alternative<double>(vals[0])) {
                    double sum = 0;
                    for (auto &v : vals) sum += toDouble(v);
                    result = sum / vals.size();
                } else {
                    throw std::logic_error("Cannot average non-numeric field");
                }
                break;
            case AggregateOp::MIN:
                if (std::holds_alternative<int>(vals[0])) {
                    int m = std::get<int>(vals[0]);
                    for (auto &v : vals) m = std::min(m, std::get<int>(v));
                    result = m;
                } else if (std::holds_alternative<double>(vals[0])) {
                    double m = std::get<double>(vals[0]);
                    for (auto &v : vals) m = std::min(m, std::get<double>(v));
                    result = m;
                } else {
                    std::string m = std::get<std::string>(vals[0]);
                    for (auto &v : vals) m = std::min(m, std::get<std::string>(v));
                    result = m;
                }
                break;
            case AggregateOp::MAX:
                if (std::holds_alternative<int>(vals[0])) {
                    int m = std::get<int>(vals[0]);
                    for (auto &v : vals) m = std::max(m, std::get<int>(v));
                    result = m;
                } else if (std::holds_alternative<double>(vals[0])) {
                    double m = std::get<double>(vals[0]);
                    for (auto &v : vals) m = std::max(m, std::get<double>(v));
                    result = m;
                } else {
                    std::string m = std::get<std::string>(vals[0]);
                    for (auto &v : vals) m = std::max(m, std::get<std::string>(v));
                    result = m;
                }
                break;
        }
        std::vector<field_t> outFields;
        if (hasGroup) outFields.push_back(g.key);
        outFields.push_back(result);
        out.insertTuple(Tuple(outFields));
    }
}

void db::join(const DbFile &left, const DbFile &right, DbFile &out, const JoinPredicate &pred) {
    // TODO: Implement this function
    const auto &ld = left.getTupleDesc();
    const auto &rd = right.getTupleDesc();
    size_t lidx = ld.index_of(pred.left);
    size_t ridx = rd.index_of(pred.right);
    for (auto lit = left.begin(); lit != left.end(); left.next(lit)) {
        const Tuple lt = left.getTuple(lit);
        for (auto rit = right.begin(); rit != right.end(); right.next(rit)) {
            const Tuple rt = right.getTuple(rit);
            if (!compare(lt.get_field(lidx), rt.get_field(ridx), pred.op)) continue;
            std::vector<field_t> fields;
            fields.reserve(ld.size() + rd.size() - (pred.op == PredicateOp::EQ ? 1 : 0));
            for (size_t i = 0; i < ld.size(); ++i) fields.push_back(lt.get_field(i));
            for (size_t i = 0; i < rd.size(); ++i) {
                if (!(pred.op == PredicateOp::EQ && i == ridx)) fields.push_back(rt.get_field(i));
            }
            out.insertTuple(Tuple(fields));
        }
    }
}
