// QuadraticProbingHash.h
// Probe sequence: index = (homeIndex + i*i) % tableSize
// Reduces primary clustering. With a prime table size it is only guaranteed
// to find a free slot while the load factor is at most 0.5, so the automatic
// rehash threshold is 0.5. Failure is reported - it never falls back to linear.

#ifndef QUADRATICPROBINGHASH_H
#define QUADRATICPROBINGHASH_H

#include "OpenAddressHash.h"

template<class KeyType, class ValueType>
class QuadraticProbingHash : public OpenAddressHash<KeyType, ValueType>
{
public:
    explicit QuadraticProbingHash(int initialSize = 11)
        : OpenAddressHash<KeyType, ValueType>(initialSize, 0.5) {}   // [REQ §7] load-factor threshold 0.5

    std::string getMethodName() const override { return "Quadratic Probing"; }

protected:
    // [REQ §5.2 Quadratic Probing] index = (homeIndex + i*i) % tableSize
    int probeIndex(int homeIndex, int i, const KeyType&) const override
    {
        long long offset = static_cast<long long>(i) * i;
        return static_cast<int>((homeIndex + offset) % this->tableSize);
    }
};

#endif
