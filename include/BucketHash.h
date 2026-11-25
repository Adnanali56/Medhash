// BucketHash.h
// Traditional bucket hashing with an overflow area.
//   numBuckets buckets x bucketSize slots (tableSize = numBuckets * bucketSize)
//   bucket = key % numBuckets; the search starts at the FIRST slot of the bucket.
// When every slot of the bucket is occupied, the record goes to the overflow
// area, a linked list built from ChainNode.
//
// The program reports four situations: no collision, ordinary collision
// (another slot of the same bucket), bucket full -> overflow insertion,
// and overflow search / overflow deletion.

#ifndef BUCKETHASH_H
#define BUCKETHASH_H

#include "HashTableBase.h"
#include "HashEntry.h"
#include "ChainNode.h"
#include "HashUtils.h"
#include <iomanip>
#include <sstream>

template<class KeyType, class ValueType>
class BucketHash : public HashTableBase<KeyType, ValueType>
{
public:
    explicit BucketHash(int requestedSlots = 15, int slotsPerBucket = 3);
    BucketHash(const BucketHash& other);               // deep copy
    BucketHash& operator=(const BucketHash& other);    // deep copy assignment
    ~BucketHash() override;

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
    std::string getMethodName() const override { return "Bucket Hashing"; }
    HashStatistics getStatistics() const override;

    int getNumBuckets() const;
    int getBucketSize() const;
    int getOverflowSize() const;

protected:
    using HashTableBase<KeyType, ValueType>::count;
    using HashTableBase<KeyType, ValueType>::stats;
    using HashTableBase<KeyType, ValueType>::lastReport;
    using HashTableBase<KeyType, ValueType>::maxLoadFactor;
    using HashTableBase<KeyType, ValueType>::autoRehash;

    // The slot where the search for a free position begins.
    // Traditional: the first slot of bucket (key % numBuckets).
    virtual int homeSlot(const KeyType& key) const;

    // j-th slot examined inside the home slot's bucket (wraps within the bucket)
    int slotFor(int home, int j) const;

    bool showsOverflowStatistics() const override { return true; }

    HashEntry<KeyType, ValueType>* slots;
    int numBuckets;
    int bucketSize;
    int tableSize;
    ChainNode<KeyType, ValueType>* overflowHead;   // [REQ §6 Overflow Area] linked overflow list
    int overflowSize;

private:
    bool keyExists(const KeyType& key) const;
    int appendToOverflow(ChainNode<KeyType, ValueType>* node);   // returns 1-based position
    void placeWithoutStatistics(const KeyType& key, const ValueType& value);
    void destroyOverflow();
    void copyFrom(const BucketHash& other);
};

// ===================== Implementation =====================

template<class KeyType, class ValueType>
BucketHash<KeyType, ValueType>::BucketHash(int requestedSlots, int slotsPerBucket)
    : HashTableBase<KeyType, ValueType>(0.8, false),   // automatic rehash OFF so overflow can be shown
      slots(nullptr),
      numBuckets(0),
      bucketSize(slotsPerBucket < 1 ? 1 : slotsPerBucket),
      tableSize(0),
      overflowHead(nullptr),
      overflowSize(0)
{
    int buckets = requestedSlots / bucketSize;
    numBuckets = hashutil::nextPrime(buckets < 2 ? 2 : buckets);
    tableSize = numBuckets * bucketSize;
    slots = new HashEntry<KeyType, ValueType>[tableSize];
}

template<class KeyType, class ValueType>
BucketHash<KeyType, ValueType>::BucketHash(const BucketHash& other)
    : HashTableBase<KeyType, ValueType>(other),
      slots(nullptr), numBuckets(0), bucketSize(1), tableSize(0),
      overflowHead(nullptr), overflowSize(0)
{
    copyFrom(other);
}

template<class KeyType, class ValueType>
BucketHash<KeyType, ValueType>& BucketHash<KeyType, ValueType>::operator=(const BucketHash& other)
{
    if (this != &other)
    {
        HashTableBase<KeyType, ValueType>::operator=(other);
        delete[] slots;
        slots = nullptr;
        destroyOverflow();
        copyFrom(other);
    }
    return *this;
}

template<class KeyType, class ValueType>
BucketHash<KeyType, ValueType>::~BucketHash()
{
    delete[] slots;
    destroyOverflow();
}

template<class KeyType, class ValueType>
void BucketHash<KeyType, ValueType>::copyFrom(const BucketHash& other)
{
    numBuckets = other.numBuckets;
    bucketSize = other.bucketSize;
    tableSize = other.tableSize;
    slots = new HashEntry<KeyType, ValueType>[tableSize];
    for (int i = 0; i < tableSize; ++i)
        slots[i] = other.slots[i];

    overflowHead = nullptr;
    overflowSize = 0;
    ChainNode<KeyType, ValueType>* tail = nullptr;
    for (ChainNode<KeyType, ValueType>* source = other.overflowHead; source != nullptr; source = source->getNext())
    {
        ChainNode<KeyType, ValueType>* node = new ChainNode<KeyType, ValueType>(source->getKey(), source->getValue());
        if (tail == nullptr) overflowHead = node;
        else tail->setNext(node);
        tail = node;
        ++overflowSize;
    }
}

template<class KeyType, class ValueType>
void BucketHash<KeyType, ValueType>::destroyOverflow()
{
    while (overflowHead != nullptr)
    {
        ChainNode<KeyType, ValueType>* next = overflowHead->getNext();
        delete overflowHead;
        overflowHead = next;
    }
    overflowSize = 0;
}

template<class KeyType, class ValueType>
int BucketHash<KeyType, ValueType>::homeSlot(const KeyType& key) const
{
    // [REQ §5.5 Traditional Bucket Hashing] bucket = key % numBuckets; start at its first slot
    int bucket = static_cast<int>(hashutil::keyToNumber(key) % static_cast<unsigned long>(numBuckets));
    return bucket * bucketSize;
}

template<class KeyType, class ValueType>
int BucketHash<KeyType, ValueType>::slotFor(int home, int j) const
{
    // [REQ §5.5 / §5.6] search ONLY the slots of the home slot's bucket (wrapping inside it)
    int bucketStart = (home / bucketSize) * bucketSize;
    int offset = home - bucketStart;
    return bucketStart + (offset + j) % bucketSize;
}

template<class KeyType, class ValueType>
bool BucketHash<KeyType, ValueType>::keyExists(const KeyType& key) const
{
    int home = homeSlot(key);
    for (int j = 0; j < bucketSize; ++j)
    {
        int index = slotFor(home, j);
        if (slots[index].isOccupied() && slots[index].getKey() == key)
            return true;
    }
    for (ChainNode<KeyType, ValueType>* node = overflowHead; node != nullptr; node = node->getNext())
    {
        if (node->getKey() == key)
            return true;
    }
    return false;
}

template<class KeyType, class ValueType>
int BucketHash<KeyType, ValueType>::appendToOverflow(ChainNode<KeyType, ValueType>* node)
{
    int position = 1;
    if (overflowHead == nullptr)
    {
        overflowHead = node;
    }
    else
    {
        ChainNode<KeyType, ValueType>* current = overflowHead;
        ++position;
        while (current->getNext() != nullptr)
        {
            current = current->getNext();
            ++position;
        }
        current->setNext(node);
    }
    ++overflowSize;
    return position;
}

template<class KeyType, class ValueType>
bool BucketHash<KeyType, ValueType>::insert(const KeyType& key, const ValueType& value)
{
    std::ostringstream report;
    report << std::fixed << std::setprecision(3);

    if (keyExists(key))
    {
        report << "Duplicate patient ID " << key << " rejected (IDs must be unique).\n";
        lastReport = report.str();
        return false;
    }

    if (autoRehash && static_cast<double>(count + 1) / tableSize > maxLoadFactor)
    {
        int oldSize = tableSize;
        rehash(2 * tableSize);
        report << "Load factor would exceed " << maxLoadFactor
               << " -> automatic rehash: " << oldSize << " -> " << tableSize << " slots\n";
    }

    int home = homeSlot(key);
    int bucket = home / bucketSize;
    // [REQ §6] ordinary collision = home slot occupied but bucket still has room
    bool collided = slots[home].isOccupied();
    int probes = 0;

    report << "Home slot: " << home << " (bucket B" << bucket << ", slots "
           << bucket * bucketSize << "-" << bucket * bucketSize + bucketSize - 1 << ")\n"
           << "Collision strategy: " << this->getMethodName() << "\n";

    for (int j = 0; j < bucketSize; ++j)
    {
        int index = slotFor(home, j);
        ++probes;
        if (!slots[index].isOccupied())
        {
            slots[index].set(key, value);
            ++count;
            stats.recordInsertion(probes, collided);
            if (collided)
                report << "Ordinary collision: home slot occupied; placed in another slot of bucket B"
                       << bucket << ".\n";
            else
                report << "No collision: home slot was free.\n";
            report << "Probes required: " << probes << "\n"
                   << "Final slot: " << index << "\n";
            lastReport = report.str();
            return true;
        }
    }

    // [REQ §6 Bucket Full -> Overflow Insertion]
    int position = appendToOverflow(new ChainNode<KeyType, ValueType>(key, value));
    ++count;
    ++probes;
    stats.recordInsertion(probes, true);
    stats.recordOverflowInsertion();

    report << "BUCKET FULL: all " << bucketSize << " slots of bucket B" << bucket << " are occupied.\n"
           << "Overflow insertion: record placed at overflow position " << position << ".\n"
           << "Probes required: " << probes << "\n";
    lastReport = report.str();
    return true;
}

template<class KeyType, class ValueType>
bool BucketHash<KeyType, ValueType>::search(const KeyType& key, ValueType& result) const
{
    int home = homeSlot(key);
    int bucket = home / bucketSize;
    int comparisons = 0;
    std::ostringstream report;
    report << "Home slot: " << home << " (bucket B" << bucket << ")\n";

    // 1) search every slot of the bucket
    for (int j = 0; j < bucketSize; ++j)
    {
        int index = slotFor(home, j);
        ++comparisons;
        if (slots[index].isOccupied() && slots[index].getKey() == key)
        {
            result = slots[index].getValue();
            report << "Found in bucket B" << bucket << ", slot " << index << "\n"
                   << "Comparisons: " << comparisons << "\n";
            lastReport = report.str();
            stats.recordSearch(true, comparisons);
            return true;
        }
    }

    // 2) [REQ §6 Overflow Search]
    int position = 0;
    for (ChainNode<KeyType, ValueType>* node = overflowHead; node != nullptr; node = node->getNext())
    {
        ++comparisons;
        ++position;
        if (node->getKey() == key)
        {
            result = node->getValue();
            report << "Not in bucket B" << bucket << " -> OVERFLOW SEARCH: found at overflow position "
                   << position << "\n"
                   << "Comparisons: " << comparisons << "\n";
            lastReport = report.str();
            stats.recordSearch(true, comparisons);
            return true;
        }
    }

    report << "Not found in bucket B" << bucket << " or the overflow area.\n"
           << "Comparisons: " << comparisons << "\n";
    lastReport = report.str();
    stats.recordSearch(false, comparisons);
    return false;
}

template<class KeyType, class ValueType>
bool BucketHash<KeyType, ValueType>::remove(const KeyType& key)
{
    int home = homeSlot(key);
    int bucket = home / bucketSize;
    std::ostringstream report;

    for (int j = 0; j < bucketSize; ++j)
    {
        int index = slotFor(home, j);
        if (slots[index].isOccupied() && slots[index].getKey() == key)
        {
            // Every search scans the whole bucket, so the slot can safely become EMPTY.
            slots[index].makeEmpty();
            --count;
            report << "Deleted patient " << key << " from bucket B" << bucket << ", slot " << index << ".\n";
            lastReport = report.str();
            return true;
        }
    }

    ChainNode<KeyType, ValueType>* previous = nullptr;
    ChainNode<KeyType, ValueType>* current = overflowHead;
    int position = 1;
    while (current != nullptr && !(current->getKey() == key))
    {
        previous = current;
        current = current->getNext();
        ++position;
    }

    if (current == nullptr)
    {
        report << "Patient ID " << key << " not found in bucket B" << bucket << " or overflow.\n";
        lastReport = report.str();
        return false;
    }

    // [REQ §6 Overflow Deletion]
    if (previous == nullptr) overflowHead = current->getNext();
    else previous->setNext(current->getNext());
    delete current;
    --overflowSize;
    --count;

    report << "OVERFLOW DELETION: removed patient " << key << " from overflow position "
           << position << ".\n";
    lastReport = report.str();
    return true;
}

template<class KeyType, class ValueType>
bool BucketHash<KeyType, ValueType>::update(const KeyType& key, const ValueType& newValue)
{
    int home = homeSlot(key);
    for (int j = 0; j < bucketSize; ++j)
    {
        int index = slotFor(home, j);
        if (slots[index].isOccupied() && slots[index].getKey() == key)
        {
            slots[index].setValue(newValue);
            lastReport = "Record in slot " + std::to_string(index) + " updated (key unchanged).\n";
            return true;
        }
    }
    for (ChainNode<KeyType, ValueType>* node = overflowHead; node != nullptr; node = node->getNext())
    {
        if (node->getKey() == key)
        {
            node->setValue(newValue);
            lastReport = "Record in the overflow area updated (key unchanged).\n";
            return true;
        }
    }
    lastReport = "Patient ID not found.\n";
    return false;
}

template<class KeyType, class ValueType>
void BucketHash<KeyType, ValueType>::clear()
{
    for (int i = 0; i < tableSize; ++i)
        slots[i].makeEmpty();
    destroyOverflow();
    count = 0;
    stats.reset();
    lastReport = "All buckets and the overflow area cleared; statistics reset.\n";
}

template<class KeyType, class ValueType>
bool BucketHash<KeyType, ValueType>::isFull() const
{
    return false;   // the overflow area can always grow
}

template<class KeyType, class ValueType>
int BucketHash<KeyType, ValueType>::getTableSize() const { return tableSize; }

template<class KeyType, class ValueType>
int BucketHash<KeyType, ValueType>::getNumBuckets() const { return numBuckets; }

template<class KeyType, class ValueType>
int BucketHash<KeyType, ValueType>::getBucketSize() const { return bucketSize; }

template<class KeyType, class ValueType>
int BucketHash<KeyType, ValueType>::getOverflowSize() const { return overflowSize; }

template<class KeyType, class ValueType>
void BucketHash<KeyType, ValueType>::placeWithoutStatistics(const KeyType& key, const ValueType& value)
{
    int home = homeSlot(key);
    for (int j = 0; j < bucketSize; ++j)
    {
        int index = slotFor(home, j);
        if (!slots[index].isOccupied())
        {
            slots[index].set(key, value);
            ++count;
            return;
        }
    }
    appendToOverflow(new ChainNode<KeyType, ValueType>(key, value));
    ++count;
}

// [REQ §7 Rehashing]
// Rehash: more buckets; every record (including overflow) is hashed again.
template<class KeyType, class ValueType>
bool BucketHash<KeyType, ValueType>::rehash(int newSize)
{
    std::vector<std::pair<KeyType, ValueType>> entries = getAllEntries();
    int oldSize = tableSize;
    int oldOverflow = overflowSize;

    delete[] slots;
    destroyOverflow();

    int buckets = newSize / bucketSize;
    numBuckets = hashutil::nextPrime(buckets < 2 ? 2 : buckets);
    tableSize = numBuckets * bucketSize;
    slots = new HashEntry<KeyType, ValueType>[tableSize];
    count = 0;

    for (const auto& entry : entries)
        placeWithoutStatistics(entry.first, entry.second);

    stats.recordRehash();

    std::ostringstream report;
    report << std::fixed << std::setprecision(3)
           << "Rehashed " << this->getMethodName() << ": " << oldSize << " -> " << tableSize
           << " slots (" << numBuckets << " buckets x " << bucketSize << "). "
           << count << " records re-hashed.\n"
           << "Overflow records: " << oldOverflow << " -> " << overflowSize
           << " | New load factor: " << this->getLoadFactor() << "\n";
    lastReport = report.str();
    return true;
}

template<class KeyType, class ValueType>
std::vector<std::pair<KeyType, ValueType>> BucketHash<KeyType, ValueType>::getAllEntries() const
{
    std::vector<std::pair<KeyType, ValueType>> entries;
    for (int i = 0; i < tableSize; ++i)
    {
        if (slots[i].isOccupied())
            entries.push_back(std::make_pair(slots[i].getKey(), slots[i].getValue()));
    }
    for (ChainNode<KeyType, ValueType>* node = overflowHead; node != nullptr; node = node->getNext())
        entries.push_back(std::make_pair(node->getKey(), node->getValue()));
    return entries;
}

template<class KeyType, class ValueType>
HashStatistics BucketHash<KeyType, ValueType>::getStatistics() const
{
    // [REQ §11 Statistics] overflow count
    HashStatistics current = stats;
    current.setCurrentOverflow(overflowSize);
    return current;
}

template<class KeyType, class ValueType>
void BucketHash<KeyType, ValueType>::print(std::ostream& os) const
{
    std::ios oldState(nullptr);
    oldState.copyfmt(os);

    os << std::fixed << std::setprecision(3)
       << "Method: " << this->getMethodName()
       << " | " << numBuckets << " buckets x " << bucketSize << " slots = " << tableSize
       << " | Records: " << count
       << " | Load factor: " << this->getLoadFactor()
       << " | Overflow: " << overflowSize << "\n";
    os << std::left << std::setw(16) << "Bucket (slots)" << "Contents\n";
    os << std::string(60, '-') << "\n";

    for (int b = 0; b < numBuckets; ++b)
    {
        int first = b * bucketSize;
        std::ostringstream label;
        label << "B" << b << " (" << first << "-" << first + bucketSize - 1 << ")";
        os << std::setw(16) << label.str();
        for (int j = 0; j < bucketSize; ++j)
        {
            const HashEntry<KeyType, ValueType>& slot = slots[first + j];
            if (slot.isOccupied())
                os << "[" << slot.getKey() << "] ";
            else
                os << "[EMPTY] ";
        }
        os << "\n";
    }

    os << "\nOverflow: ";
    if (overflowHead == nullptr)
        os << "(empty)";
    for (ChainNode<KeyType, ValueType>* node = overflowHead; node != nullptr; node = node->getNext())
        os << "[" << node->getKey() << "] ";
    os << "\n";

    os.copyfmt(oldState);
}

#endif
