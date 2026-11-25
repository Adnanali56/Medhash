// DoubleHash.h
// h1(key) = key % tableSize
// h2(key) = PRIME - (key % PRIME), where PRIME is the largest prime < tableSize
// index   = (h1(key) + i * h2(key)) % tableSize
//
// h2 is always between 1 and PRIME, so it is never zero. Because tableSize is
// prime and h2 < tableSize, the step size is coprime with tableSize and the
// probe sequence can reach every slot.

#ifndef DOUBLEHASH_H
#define DOUBLEHASH_H

#include "OpenAddressHash.h"
#include <string>

template<class KeyType, class ValueType>
class DoubleHash : public OpenAddressHash<KeyType, ValueType>
{
public:
    explicit DoubleHash(int initialSize = 11)
        : OpenAddressHash<KeyType, ValueType>(initialSize, 0.7),
          prime(hashutil::previousPrime(this->tableSize)) {}

    std::string getMethodName() const override { return "Double Hashing"; }

    int getPrime() const { return prime; }

    // [REQ §5.3 Double Hashing] secondary hash h2(key) = PRIME - (key % PRIME); never zero
    int secondaryHash(const KeyType& key) const
    {
        return prime - static_cast<int>(hashutil::keyToNumber(key) % static_cast<unsigned long>(prime));
    }

protected:
    // [REQ §5.3 Double Hashing] index = (h1(key) + i * h2(key)) % tableSize
    int probeIndex(int homeIndex, int i, const KeyType& key) const override
    {
        long long step = static_cast<long long>(i) * secondaryHash(key);
        return static_cast<int>((homeIndex + step) % this->tableSize);
    }

    // [REQ §5.3 Diagnostics] report the secondary step size
    std::string probeDetails(const KeyType& key) const override
    {
        return "Secondary hash: h2 = " + std::to_string(prime) + " - (key % " +
               std::to_string(prime) + ") = step size " + std::to_string(secondaryHash(key)) + "\n";
    }

    // [REQ §7 Rehashing] PRIME is recomputed for the new table size
    void onTableResized() override
    {
        prime = hashutil::previousPrime(this->tableSize);   // PRIME must stay below the new size
    }

private:
    int prime;
};

#endif
