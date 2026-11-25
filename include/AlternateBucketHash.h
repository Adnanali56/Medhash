// AlternateBucketHash.h
// Alternate bucket method: the key hashes to a home SLOT, not to the start of
// a bucket.
//   homeSlot = key % tableSize
//   bucket   = homeSlot / bucketSize      (e.g. bucket 0 = slots 0,1 when bucketSize = 2)
// If the home slot is occupied, only the remaining slots of that bucket are
// searched (wrapping inside the bucket). If the bucket is full, the record goes
// to overflow. Everything else (overflow, search, delete, rehash) is inherited.

#ifndef ALTERNATEBUCKETHASH_H
#define ALTERNATEBUCKETHASH_H

#include "BucketHash.h"

template<class KeyType, class ValueType>
class AlternateBucketHash : public BucketHash<KeyType, ValueType>
{
public:
    explicit AlternateBucketHash(int requestedSlots = 15, int slotsPerBucket = 3)
        : BucketHash<KeyType, ValueType>(requestedSlots, slotsPerBucket) {}

    std::string getMethodName() const override { return "Alternate Bucket Hashing + Overflow"; }

protected:
    // [REQ §5.6 Alternate Bucket Hashing] homeSlot = key % tableSize (a SLOT, not a bucket start)
    int homeSlot(const KeyType& key) const override
    {
        return static_cast<int>(hashutil::keyToNumber(key) % static_cast<unsigned long>(this->tableSize));
    }
};

#endif
