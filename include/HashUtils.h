// HashUtils.h
// Small helpers shared by every hash table: prime numbers for table sizes
// and conversion of a key into a non-negative number that can be hashed.

#ifndef HASHUTILS_H
#define HASHUTILS_H

#include <functional>

namespace hashutil
{
    inline bool isPrime(int n)
    {
        if (n < 2) return false;
        if (n % 2 == 0) return n == 2;
        for (int i = 3; i * i <= n; i += 2)
        {
            if (n % i == 0) return false;
        }
        return true;
    }

    // Smallest prime >= n
    inline int nextPrime(int n)
    {
        if (n <= 2) return 2;
        if (n % 2 == 0) ++n;
        while (!isPrime(n)) n += 2;
        return n;
    }

    // Largest prime strictly smaller than n (used for the double-hashing PRIME)
    inline int previousPrime(int n)
    {
        for (int p = n - 1; p >= 2; --p)
        {
            if (isPrime(p)) return p;
        }
        return 2;
    }

    // Generic key -> number. Integers are used directly so that
    // h(key) = key % tableSize matches the textbook formulas exactly.
    template<class KeyType>
    unsigned long keyToNumber(const KeyType& key)
    {
        return static_cast<unsigned long>(std::hash<KeyType>{}(key));
    }

    template<>
    inline unsigned long keyToNumber<int>(const int& key)
    {
        long value = key;
        return static_cast<unsigned long>(value < 0 ? -value : value);
    }
}

#endif
