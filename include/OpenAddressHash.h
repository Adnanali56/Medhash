// OpenAddressHash.h
// Abstract base for open addressing (closed hashing). Every record lives in
// the table array itself. Subclasses only supply the probe sequence:
//   LinearProbingHash, QuadraticProbingHash, DoubleHash.
//
// Slots are EMPTY, OCCUPIED, or DELETED. A search continues past DELETED
// slots and stops only at an EMPTY slot (or after tableSize probes).

#ifndef OPENADDRESSHASH_H
#define OPENADDRESSHASH_H

#include "HashTableBase.h"
#include "HashEntry.h"
#include "HashUtils.h"
#include <algorithm>
#include <iomanip>
#include <sstream>

template<class KeyType, class ValueType>
class OpenAddressHash : public HashTableBase<KeyType, ValueType>
{
public:
    explicit OpenAddressHash(int initialSize = 11, double maxLoad = 0.7);
    OpenAddressHash(const OpenAddressHash& other);                 // deep copy
    OpenAddressHash& operator=(const OpenAddressHash& other);      // deep copy assignment
    ~OpenAddressHash() override;

    bool insert(const KeyType& key, const ValueType& value) override;
    bool remove(const KeyType& key) override;
    bool search(const KeyType& key, ValueType& result) const override;
    bool update(const KeyType& key, const ValueType& newValue) override;
    void clear() override;
    bool isFull() const override;
    int getTableSize() const override;
    bool rehash(int newSize) override;
    std::vector<std::pair<KeyType, ValueType>> getAllEntries() const override;
    void print(std::ostream& os) const override;

    int getDeletedCount() const;

protected:
    using HashTableBase<KeyType, ValueType>::count;
    using HashTableBase<KeyType, ValueType>::stats;
    using HashTableBase<KeyType, ValueType>::lastReport;
    using HashTableBase<KeyType, ValueType>::maxLoadFactor;
    using HashTableBase<KeyType, ValueType>::autoRehash;

    // [REQ §5.1-5.3 Hash Function] primary hash used by linear, quadratic and double hashing:
    // h1(key) = key % tableSize
    int hash(const KeyType& key) const;

    // The i-th position of the probe sequence (i = 0 is the home index)
    virtual int probeIndex(int homeIndex, int i, const KeyType& key) const = 0;

    // Extra diagnostic lines (double hashing prints its step size)
    virtual std::string probeDetails(const KeyType& key) const;

    // Called after the table size changes (double hashing recomputes PRIME)
    virtual void onTableResized();

    HashEntry<KeyType, ValueType>* table;
    int tableSize;
    int deletedCount;

private:
    int findIndex(const KeyType& key, int& comparisons) const;
    int tryInsert(const KeyType& key, const ValueType& value,
                  int& probes, bool& collided, bool& duplicate);
    bool placeWithoutStatistics(const KeyType& key, const ValueType& value);
    void copyFrom(const OpenAddressHash& other);
};

// ===================== Implementation =====================

template<class KeyType, class ValueType>
OpenAddressHash<KeyType, ValueType>::OpenAddressHash(int initialSize, double maxLoad)
    : HashTableBase<KeyType, ValueType>(maxLoad, true),
      table(nullptr),
      tableSize(hashutil::nextPrime(initialSize < 5 ? 5 : initialSize)),
      deletedCount(0)
{
    table = new HashEntry<KeyType, ValueType>[tableSize];
}

template<class KeyType, class ValueType>
OpenAddressHash<KeyType, ValueType>::OpenAddressHash(const OpenAddressHash& other)
    : HashTableBase<KeyType, ValueType>(other), table(nullptr), tableSize(0), deletedCount(0)
{
    copyFrom(other);
}

template<class KeyType, class ValueType>
OpenAddressHash<KeyType, ValueType>&
OpenAddressHash<KeyType, ValueType>::operator=(const OpenAddressHash& other)
{
    if (this != &other)                                   // 1. guard self-assignment
    {
        HashTableBase<KeyType, ValueType>::operator=(other);
        delete[] table;                                   // 2. free existing memory
        table = nullptr;
        copyFrom(other);                                  // 3. deep copy
    }
    return *this;                                         // 4. return *this
}

template<class KeyType, class ValueType>
OpenAddressHash<KeyType, ValueType>::~OpenAddressHash()
{
    delete[] table;
}

template<class KeyType, class ValueType>
void OpenAddressHash<KeyType, ValueType>::copyFrom(const OpenAddressHash& other)
{
    tableSize = other.tableSize;
    deletedCount = other.deletedCount;
    table = new HashEntry<KeyType, ValueType>[tableSize];
    for (int i = 0; i < tableSize; ++i)
        table[i] = other.table[i];
}

template<class KeyType, class ValueType>
int OpenAddressHash<KeyType, ValueType>::hash(const KeyType& key) const
{
    return static_cast<int>(hashutil::keyToNumber(key) % static_cast<unsigned long>(tableSize));
}

template<class KeyType, class ValueType>
std::string OpenAddressHash<KeyType, ValueType>::probeDetails(const KeyType&) const
{
    return "";
}

template<class KeyType, class ValueType>
void OpenAddressHash<KeyType, ValueType>::onTableResized()
{
}

// [REQ §5.1 DELETED handling] [REQ §9.2 Search]
// Returns the slot holding key, or -1. Counts every slot examined.
// DELETED slots are skipped (not treated as the end); only EMPTY stops the search.
template<class KeyType, class ValueType>
int OpenAddressHash<KeyType, ValueType>::findIndex(const KeyType& key, int& comparisons) const
{
    int home = hash(key);
    comparisons = 0;
    for (int i = 0; i < tableSize; ++i)
    {
        int index = probeIndex(home, i, key);
        ++comparisons;
        const HashEntry<KeyType, ValueType>& entry = table[index];
        if (entry.getState() == EntryState::EMPTY)
            return -1;                                    // EMPTY ends the search
        if (entry.isOccupied() && entry.getKey() == key)
            return index;
        // DELETED: keep probing - the key may be further along the sequence
    }
    return -1;
}

// [REQ §5.1-5.3 Collision Resolution] walks the probe sequence of the active strategy.
// [REQ §5.2 Failure Detection] returns -1 when the probe sequence reaches no free slot.
// Places a record in the first EMPTY or DELETED slot of its probe sequence.
// Keeps probing past DELETED slots first to make sure the key is not a duplicate.
template<class KeyType, class ValueType>
int OpenAddressHash<KeyType, ValueType>::tryInsert(const KeyType& key, const ValueType& value,
                                                   int& probes, bool& collided, bool& duplicate)
{
    int home = hash(key);
    int firstAvailable = -1;
    int probesToAvailable = 0;
    duplicate = false;
    collided = table[home].isOccupied();

    for (int i = 0; i < tableSize; ++i)
    {
        int index = probeIndex(home, i, key);
        const HashEntry<KeyType, ValueType>& entry = table[index];

        if (entry.isOccupied())
        {
            if (entry.getKey() == key)
            {
                duplicate = true;
                return -1;
            }
        }
        else
        {
            if (firstAvailable < 0)
            {
                firstAvailable = index;
                probesToAvailable = i + 1;
            }
            if (entry.getState() == EntryState::EMPTY)
                break;                                    // key cannot appear further on
        }
    }

    probes = (firstAvailable >= 0) ? probesToAvailable : tableSize;
    if (firstAvailable < 0)
        return -1;                                        // probe sequence found no slot

    if (table[firstAvailable].getState() == EntryState::DELETED)
        --deletedCount;                                   // re-using a DELETED slot
    table[firstAvailable].set(key, value);
    ++count;
    return firstAvailable;
}

template<class KeyType, class ValueType>
bool OpenAddressHash<KeyType, ValueType>::insert(const KeyType& key, const ValueType& value)
{
    std::ostringstream report;
    report << std::fixed << std::setprecision(3);

    // [REQ §7 Load Factor monitoring] check the load factor BEFORE inserting;
    // if it would pass the threshold, rehash to a larger table automatically.
    if (autoRehash && static_cast<double>(count + 1) / tableSize > maxLoadFactor)
    {
        int oldSize = tableSize;
        rehash(2 * tableSize);
        report << "Load factor would exceed " << maxLoadFactor
               << " -> automatic rehash: table size " << oldSize << " -> " << tableSize << "\n";
    }

    int probes = 0;
    bool collided = false;
    bool duplicate = false;
    int index = tryInsert(key, value, probes, collided, duplicate);

    if (duplicate)
    {
        report << "Duplicate patient ID " << key << " rejected (IDs must be unique).\n";
        lastReport = report.str();
        return false;
    }

    // [REQ §5.2 Quadratic failure] the failure is reported; the same strategy is retried
    // in a larger table. It never silently switches to linear probing.
    if (index < 0 && autoRehash)
    {
        int oldSize = tableSize;
        report << "Probe sequence found no available slot in a table of size " << oldSize << ".\n";
        rehash(2 * tableSize);
        report << "Automatic rehash: table size " << oldSize << " -> " << tableSize << "; retrying.\n";
        index = tryInsert(key, value, probes, collided, duplicate);
    }

    if (index < 0)
    {
        report << "INSERTION FAILED: the " << this->getMethodName()
               << " probe sequence did not reach an available slot after "
               << probes << " probes.\n"
               << "Load factor is " << this->getLoadFactor()
               << ". Rehash to a larger table and try again.\n";
        lastReport = report.str();
        return false;
    }

    stats.recordInsertion(probes, collided);   // [REQ §11 Statistics] collisions, probes, max probes
    // [REQ §9.1 Add] diagnostics: primary index, strategy, probes, final index
    report << "Primary hash index: " << hash(key) << "\n"
           << "Collision strategy: " << this->getMethodName() << "\n"
           << probeDetails(key)
           << "Collision: " << (collided ? "yes" : "no") << "\n"
           << "Probes required: " << probes << "\n"
           << "Final index: " << index << "\n";
    lastReport = report.str();
    return true;
}

template<class KeyType, class ValueType>
bool OpenAddressHash<KeyType, ValueType>::search(const KeyType& key, ValueType& result) const
{
    int comparisons = 0;
    int index = findIndex(key, comparisons);
    std::ostringstream report;
    report << "Home Index:  " << hash(key) << "\n" << probeDetails(key);

    // [REQ §11 Statistics] successful / unsuccessful search comparisons are recorded below
    if (index >= 0)
    {
        result = table[index].getValue();
        report << "Final Index: " << index << "\n"
               << "Comparisons: " << comparisons << "\n";
        stats.recordSearch(true, comparisons);
    }
    else
    {
        report << "Comparisons: " << comparisons
               << " (unsuccessful: stopped at an EMPTY slot or after the full probe sequence)\n";
        stats.recordSearch(false, comparisons);
    }
    lastReport = report.str();
    return index >= 0;
}

template<class KeyType, class ValueType>
bool OpenAddressHash<KeyType, ValueType>::remove(const KeyType& key)
{
    int comparisons = 0;
    int index = findIndex(key, comparisons);
    std::ostringstream report;

    if (index < 0)
    {
        report << "Patient ID " << key << " not found (" << comparisons << " comparisons).\n";
        lastReport = report.str();
        return false;
    }

    // [REQ §5.1 / §9.4 Delete] mark DELETED, never EMPTY, so later probe sequences are not broken
    table[index].markDeleted();
    --count;
    ++deletedCount;
    report << "Patient ID " << key << " removed from index " << index << ".\n"
           << "Slot " << index << " is now marked DELETED (not EMPTY), so searches for keys\n"
           << "later in the same probe sequence continue past it.\n";
    lastReport = report.str();
    return true;
}

template<class KeyType, class ValueType>
bool OpenAddressHash<KeyType, ValueType>::update(const KeyType& key, const ValueType& newValue)
{
    int comparisons = 0;
    int index = findIndex(key, comparisons);
    std::ostringstream report;

    if (index < 0)
    {
        report << "Patient ID " << key << " not found.\n";
        lastReport = report.str();
        return false;
    }

    table[index].setValue(newValue);
    report << "Record at index " << index << " updated (key unchanged).\n";
    lastReport = report.str();
    return true;
}

template<class KeyType, class ValueType>
void OpenAddressHash<KeyType, ValueType>::clear()
{
    for (int i = 0; i < tableSize; ++i)
        table[i].makeEmpty();
    count = 0;
    deletedCount = 0;
    stats.reset();
    lastReport = "All records cleared; statistics reset.\n";
}

template<class KeyType, class ValueType>
bool OpenAddressHash<KeyType, ValueType>::isFull() const
{
    return count >= tableSize;
}

template<class KeyType, class ValueType>
int OpenAddressHash<KeyType, ValueType>::getTableSize() const
{
    return tableSize;
}

template<class KeyType, class ValueType>
int OpenAddressHash<KeyType, ValueType>::getDeletedCount() const
{
    return deletedCount;
}

// Used only while rehashing: the new table has no DELETED slots or duplicates.
template<class KeyType, class ValueType>
bool OpenAddressHash<KeyType, ValueType>::placeWithoutStatistics(const KeyType& key, const ValueType& value)
{
    int home = hash(key);
    for (int i = 0; i < tableSize; ++i)
    {
        int index = probeIndex(home, i, key);
        if (!table[index].isOccupied())
        {
            table[index].set(key, value);
            ++count;
            return true;
        }
    }
    return false;
}

// [REQ §7 Rehashing]
// Rehash: 1) create a larger table, 2) recompute every active record's position
// using the NEW size, 3) reinsert all active records, 4) delete the old table.
template<class KeyType, class ValueType>
bool OpenAddressHash<KeyType, ValueType>::rehash(int newSize)
{
    // The new size is prime and at least 2*count+1 so the load factor is below 0.5
    // (this guarantees quadratic probing can place every record).
    int target = hashutil::nextPrime(std::max({ newSize, 2 * count + 1, 5 }));

    HashEntry<KeyType, ValueType>* oldTable = table;
    int oldSize = tableSize;
    int oldDeleted = deletedCount;

    table = new HashEntry<KeyType, ValueType>[target];
    tableSize = target;
    onTableResized();
    count = 0;
    deletedCount = 0;

    for (int i = 0; i < oldSize; ++i)
    {
        if (oldTable[i].isOccupied())
            placeWithoutStatistics(oldTable[i].getKey(), oldTable[i].getValue());
    }

    delete[] oldTable;
    stats.recordRehash();

    std::ostringstream report;
    report << std::fixed << std::setprecision(3)
           << "Rehashed " << this->getMethodName() << ": table size " << oldSize << " -> " << tableSize
           << ". " << count << " active records were re-hashed with the new size; "
           << oldDeleted << " DELETED markers discarded.\n"
           << "New load factor: " << this->getLoadFactor() << "\n";
    lastReport = report.str();
    return true;
}

template<class KeyType, class ValueType>
std::vector<std::pair<KeyType, ValueType>> OpenAddressHash<KeyType, ValueType>::getAllEntries() const
{
    std::vector<std::pair<KeyType, ValueType>> entries;
    for (int i = 0; i < tableSize; ++i)
    {
        if (table[i].isOccupied())
            entries.push_back(std::make_pair(table[i].getKey(), table[i].getValue()));
    }
    return entries;
}

template<class KeyType, class ValueType>
void OpenAddressHash<KeyType, ValueType>::print(std::ostream& os) const
{
    std::ios oldState(nullptr);
    oldState.copyfmt(os);

    os << std::fixed << std::setprecision(3)
       << "Method: " << this->getMethodName()
       << " | Table size: " << tableSize
       << " | Records: " << count
       << " | Load factor: " << this->getLoadFactor()
       << " | DELETED slots: " << deletedCount << "\n";
    os << std::left << std::setw(7) << "Index" << std::setw(12) << "Patient ID"
       << std::setw(22) << "Name" << "Status\n";
    os << std::string(52, '-') << "\n";

    for (int i = 0; i < tableSize; ++i)
    {
        os << std::setw(7) << i;
        if (table[i].isOccupied())
        {
            std::ostringstream key, value;
            key << table[i].getKey();
            value << table[i].getValue();
            os << std::setw(12) << key.str() << std::setw(22) << value.str();
        }
        else
        {
            os << std::setw(12) << "---" << std::setw(22) << "---";
        }
        os << stateName(table[i].getState()) << "\n";
    }

    os.copyfmt(oldState);
}

#endif
