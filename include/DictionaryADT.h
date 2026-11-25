// DictionaryADT.h
// Abstract, generic dictionary interface. The menu program works only through
// a DictionaryADT<int, PatientRecord>* so that changing the hashing strategy
// never requires changing the menu logic (polymorphism).

#ifndef DICTIONARYADT_H
#define DICTIONARYADT_H

#include "HashStatistics.h"
#include <iostream>
#include <string>
#include <utility>
#include <vector>

template<class KeyType, class ValueType>
class DictionaryADT
{
public:
    // ---- Core dictionary operations ----
    virtual bool insert(const KeyType& key, const ValueType& value) = 0;
    virtual bool remove(const KeyType& key) = 0;
    virtual bool search(const KeyType& key, ValueType& result) const = 0;
    virtual bool update(const KeyType& key, const ValueType& newValue) = 0;
    virtual void display() const = 0;
    virtual void clear() = 0;
    virtual int getCount() const = 0;
    virtual bool isEmpty() const = 0;
    virtual bool isFull() const = 0;

    // ---- Additional operations for statistics, rehashing and diagnostics ----
    virtual void print(std::ostream& os) const = 0;
    virtual std::string getMethodName() const = 0;
    virtual int getTableSize() const = 0;
    virtual double getLoadFactor() const = 0;
    virtual HashStatistics getStatistics() const = 0;
    virtual void displayStatistics(std::ostream& os) const = 0;
    virtual void resetStatistics() = 0;
    virtual bool rehash(int newSize) = 0;
    virtual void setAutoRehash(bool enabled) = 0;
    virtual bool getAutoRehash() const = 0;
    virtual double getMaxLoadFactor() const = 0;
    virtual std::vector<std::pair<KeyType, ValueType>> getAllEntries() const = 0;
    virtual std::string getLastReport() const = 0;   // diagnostics of the last operation

    virtual ~DictionaryADT() = default;
};

// cout << dictionary;  prints method, table size, count, load factor, and entries
template<class KeyType, class ValueType>
std::ostream& operator<<(std::ostream& os, const DictionaryADT<KeyType, ValueType>& dictionary)
{
    dictionary.print(os);
    return os;
}

#endif
