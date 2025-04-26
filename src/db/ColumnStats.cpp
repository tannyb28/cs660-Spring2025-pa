#include <db/ColumnStats.hpp>
#include <stdexcept>

using namespace db;

ColumnStats::ColumnStats(unsigned buckets, int min, int max)
: buckets(buckets), min(min), max(max), totalCount(0)
// TODO pa3: some code goes here
{
    if (buckets == 0) {
        throw std::invalid_argument("Number of buckets must be greater than zero.");
    }
    if (min >= max) {
        throw std::invalid_argument("Minimum value must be less than maximum value.");
    }

    this->bucketWidth = (max - min) / buckets + ((max - min) % buckets != 0 ? 1 : 0);
    this->histogram.assign(buckets, 0);
}

void ColumnStats::addValue(int v) {
    // TODO pa3: some code goes here
    // (Assume that no value outside [min, max] is added.)
    if (v < min || v >= max) {
        return;
    }
    size_t bucketIndex = static_cast<size_t>((v - min) / bucketWidth);
    histogram[bucketIndex]++;
    totalCount++;
}

size_t ColumnStats::estimateCardinality(PredicateOp op, int v) const {
    // TODO pa3: some code goes here
    if (totalCount == 0) {
        return 0;
    }

    switch (op) {
        case PredicateOp::EQ: {
            if (v < min || v >= max) return 0;
            size_t bucketIndex = (v - min) / bucketWidth;
            double bucketHeight = histogram[bucketIndex];
            return static_cast<size_t>((bucketHeight / bucketWidth));
        }
        case PredicateOp::GT: {
            if (v >= max) return 0;
            if (v < min) return totalCount;
            size_t bucketIndex = (v - min) / bucketWidth;
            int rightEdge = min + (bucketIndex + 1) * bucketWidth-1;
            double fraction = static_cast<double>(rightEdge - v) / bucketWidth;
            auto partialBucket = static_cast<size_t>(fraction * histogram[bucketIndex]);
            size_t cardinality = partialBucket;
            for (size_t i = bucketIndex + 1; i < buckets; i++) {
                cardinality += histogram[i];
            }
            return cardinality;
        }
        case PredicateOp::LT: {
            if (v <= min) return 0;
            if (v > max) return totalCount;
            size_t bucketIndex = (v - min) / bucketWidth;
            int leftEdge = min + bucketIndex * bucketWidth;
            double fraction = static_cast<double>(v - leftEdge) / bucketWidth;
            auto partialBucket = static_cast<size_t>(fraction * histogram[bucketIndex]);
            size_t cardinality = partialBucket;
            for (size_t i = 0; i < bucketIndex; i++) {
                cardinality += histogram[i];
            }
            return cardinality;
        }
        case PredicateOp::GE: {
            if (v < min) return totalCount;
            if (v >= max) return 0;
            size_t bucketIndex = (v - min) / bucketWidth;
            int rightEdge = min + (bucketIndex + 1) * bucketWidth - 1;
            double fraction = static_cast<double>(rightEdge - v + 1) / bucketWidth;
            auto partialBucket = static_cast<size_t>(fraction * histogram[bucketIndex]);
            size_t cardinality = partialBucket;
            for (size_t i = bucketIndex + 1; i < buckets; i++) {
                cardinality += histogram[i];
            }
            return cardinality;
        }
        case PredicateOp::LE: {
            return estimateCardinality(PredicateOp::EQ, v) + estimateCardinality(PredicateOp::LT, v);
        }
        case PredicateOp::NE: {
            return totalCount - estimateCardinality(PredicateOp::EQ, v);
        }
    }
    return 0; // Default case, should not be reached
}
