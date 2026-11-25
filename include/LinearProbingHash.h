// LinearProbingHash.h
// Probe sequence: index = (homeIndex + i) % tableSize
// Simple, but suffers from primary clustering.

#ifndef LINEARPROBINGHASH_H
#define LINEARPROBINGHASH_H

#include "OpenAddressHash.h"

template<class KeyType, class ValueType>
class LinearProbingHash : public OpenAddressHash<KeyType, ValueType>
{
public:
    explicit LinearProbingHash(int initialSize = 11)
        : OpenAddressHash<KeyType, ValueType>(initialSize, 0.7) {}

    std::string getMethodName() const override { return "Linear Probing"; }

protected:
    // [REQ §5.1 Linear Probing] index = (homeIndex + i) % tableSize
    int probeIndex(int homeIndex, int i, const KeyType&) const override
    {
        return static_cast<int>((static_cast<long long>(homeIndex) + i) % this->tableSize);
    }
};

#endif
