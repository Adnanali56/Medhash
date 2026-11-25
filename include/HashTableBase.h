// HashTableBase.h
// Shared bookkeeping for every hash table: record count, statistics,
// last-operation report, and automatic-rehash settings.
// It implements the parts of DictionaryADT that are identical for all methods,
// so each concrete table only implements what is specific to its strategy.

#ifndef HASHTABLEBASE_H
#define HASHTABLEBASE_H

#include "DictionaryADT.h"
#include "HashStatistics.h"
#include <string>

template<class KeyType, class ValueType>
class HashTableBase : public DictionaryADT<KeyType, ValueType>
{
public:
    int getCount() const override { return count; }
    bool isEmpty() const override { return count == 0; }
    void display() const override { this->print(std::cout); }

    // [REQ §7 Load Factor] loadFactor = static_cast<double>(itemCount) / tableSize
    double getLoadFactor() const override
    {
        int size = this->getTableSize();
        return size == 0 ? 0.0 : static_cast<double>(count) / size;
    }

    // [REQ §11 Hash Statistics] every table exposes its measured counters here
    HashStatistics getStatistics() const override { return stats; }

    void displayStatistics(std::ostream& os) const override
    {
        HashStatistics current = this->getStatistics();
        current.display(os, this->getMethodName(), count, this->getTableSize(),
                        getLoadFactor(), showsChainStatistics(), showsOverflowStatistics());
    }

    void resetStatistics() override { stats.reset(); }
    void setAutoRehash(bool enabled) override { autoRehash = enabled; }
    bool getAutoRehash() const override { return autoRehash; }
    double getMaxLoadFactor() const override { return maxLoadFactor; }
    std::string getLastReport() const override { return lastReport; }

protected:
    HashTableBase(double maxLoad, bool autoRehashEnabled)
        : count(0), stats(), lastReport(), maxLoadFactor(maxLoad), autoRehash(autoRehashEnabled) {}

    virtual bool showsChainStatistics() const { return false; }
    virtual bool showsOverflowStatistics() const { return false; }

    int count;
    mutable HashStatistics stats;      // mutable: search() is const but still counts comparisons
    mutable std::string lastReport;    // mutable: search() is const but still reports diagnostics
    double maxLoadFactor;              // [REQ §7 Load Factor] threshold that triggers automatic rehash
    bool autoRehash;
};

#endif
