// SeparateChainingHash.h
// Open hashing / separate chaining. Every table position is the head of a
// singly linked chain built from ChainNode (no std::list).
//   [1] -> [121, Ava Kumar] -> [431, Sam Chen] -> NULL
// The load factor can exceed 1 because a chain can hold any number of nodes.

#ifndef SEPARATECHAININGHASH_H
#define SEPARATECHAININGHASH_H

#include "HashTableBase.h"
#include "ChainNode.h"
#include "HashUtils.h"
#include <iomanip>
#include <sstream>

template<class KeyType, class ValueType>
class SeparateChainingHash : public HashTableBase<KeyType, ValueType>
{
public:
    explicit SeparateChainingHash(int initialSize = 11, double maxLoad = 2.0);
    SeparateChainingHash(const SeparateChainingHash& other);              // deep copy
    SeparateChainingHash& operator=(const SeparateChainingHash& other);   // deep copy assignment
    ~SeparateChainingHash() override;

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
    std::string getMethodName() const override { return "Separate Chaining"; }
    HashStatistics getStatistics() const override;

    int getChainLength(int index) const;

protected:
    using HashTableBase<KeyType, ValueType>::count;
    using HashTableBase<KeyType, ValueType>::stats;
    using HashTableBase<KeyType, ValueType>::lastReport;
    using HashTableBase<KeyType, ValueType>::maxLoadFactor;
    using HashTableBase<KeyType, ValueType>::autoRehash;

    bool showsChainStatistics() const override { return true; }

private:
    int hash(const KeyType& key) const;
    void destroyAllNodes();
    void copyFrom(const SeparateChainingHash& other);
    static void appendToChain(ChainNode<KeyType, ValueType>*& head, ChainNode<KeyType, ValueType>* node);

    ChainNode<KeyType, ValueType>** table;   // [REQ §5.4 Separate Chaining] array of chain heads
    int tableSize;
};

// ===================== Implementation =====================

template<class KeyType, class ValueType>
SeparateChainingHash<KeyType, ValueType>::SeparateChainingHash(int initialSize, double maxLoad)
    : HashTableBase<KeyType, ValueType>(maxLoad, true),
      table(nullptr),
      tableSize(hashutil::nextPrime(initialSize < 2 ? 2 : initialSize))
{
    table = new ChainNode<KeyType, ValueType>*[tableSize]();   // () sets every head to nullptr
}

template<class KeyType, class ValueType>
SeparateChainingHash<KeyType, ValueType>::SeparateChainingHash(const SeparateChainingHash& other)
    : HashTableBase<KeyType, ValueType>(other), table(nullptr), tableSize(0)
{
    copyFrom(other);
}

template<class KeyType, class ValueType>
SeparateChainingHash<KeyType, ValueType>&
SeparateChainingHash<KeyType, ValueType>::operator=(const SeparateChainingHash& other)
{
    if (this != &other)
    {
        HashTableBase<KeyType, ValueType>::operator=(other);
        destroyAllNodes();
        delete[] table;
        table = nullptr;
        copyFrom(other);
    }
    return *this;
}

template<class KeyType, class ValueType>
SeparateChainingHash<KeyType, ValueType>::~SeparateChainingHash()
{
    destroyAllNodes();
    delete[] table;
}

// [REQ §13 Deep Copy]
// Deep copy: every node is newly allocated, so the two tables never share nodes.
template<class KeyType, class ValueType>
void SeparateChainingHash<KeyType, ValueType>::copyFrom(const SeparateChainingHash& other)
{
    tableSize = other.tableSize;
    table = new ChainNode<KeyType, ValueType>*[tableSize]();
    for (int i = 0; i < tableSize; ++i)
    {
        ChainNode<KeyType, ValueType>* tail = nullptr;
        for (ChainNode<KeyType, ValueType>* source = other.table[i]; source != nullptr; source = source->getNext())
        {
            ChainNode<KeyType, ValueType>* node =
                new ChainNode<KeyType, ValueType>(source->getKey(), source->getValue());
            if (tail == nullptr)
                table[i] = node;
            else
                tail->setNext(node);
            tail = node;
        }
    }
}

template<class KeyType, class ValueType>
void SeparateChainingHash<KeyType, ValueType>::destroyAllNodes()
{
    if (table == nullptr) return;
    for (int i = 0; i < tableSize; ++i)
    {
        ChainNode<KeyType, ValueType>* current = table[i];
        while (current != nullptr)
        {
            ChainNode<KeyType, ValueType>* next = current->getNext();
            delete current;
            current = next;
        }
        table[i] = nullptr;
    }
}

template<class KeyType, class ValueType>
void SeparateChainingHash<KeyType, ValueType>::appendToChain(ChainNode<KeyType, ValueType>*& head,
                                                             ChainNode<KeyType, ValueType>* node)
{
    if (head == nullptr)
    {
        head = node;
        return;
    }
    ChainNode<KeyType, ValueType>* current = head;
    while (current->getNext() != nullptr)
        current = current->getNext();
    current->setNext(node);
}

template<class KeyType, class ValueType>
int SeparateChainingHash<KeyType, ValueType>::hash(const KeyType& key) const
{
    return static_cast<int>(hashutil::keyToNumber(key) % static_cast<unsigned long>(tableSize));
}

template<class KeyType, class ValueType>
bool SeparateChainingHash<KeyType, ValueType>::insert(const KeyType& key, const ValueType& value)
{
    std::ostringstream report;
    report << std::fixed << std::setprecision(3);

    // [REQ §7 Load Factor] chaining allows load factor > 1; automatic rehash only above 2.0
    if (autoRehash && static_cast<double>(count + 1) / tableSize > maxLoadFactor)
    {
        int oldSize = tableSize;
        rehash(2 * tableSize);
        report << "Load factor would exceed " << maxLoadFactor
               << " -> automatic rehash: table size " << oldSize << " -> " << tableSize << "\n";
    }

    int bucket = hash(key);
    int examined = 0;
    for (ChainNode<KeyType, ValueType>* current = table[bucket]; current != nullptr; current = current->getNext())
    {
        ++examined;
        if (current->getKey() == key)
        {
            report << "Duplicate patient ID " << key << " rejected (IDs must be unique).\n";
            lastReport = report.str();
            return false;
        }
    }

    // [REQ §5.4 Separate Chaining] a collision = the chain already holds nodes; append the new node
    bool collided = (table[bucket] != nullptr);
    appendToChain(table[bucket], new ChainNode<KeyType, ValueType>(key, value));   // insert at tail
    ++count;

    int probes = examined + 1;   // nodes checked for duplicates + the slot access itself
    stats.recordInsertion(probes, collided);

    report << "Primary hash index (chain): " << bucket << "\n"
           << "Collision strategy: Separate Chaining\n"
           << "Collision: ";
    if (collided)
        report << "yes - chain already held " << examined << " node(s)\n";
    else
        report << "no - chain was empty\n";
    report << "Probes required: " << probes << "\n"
           << "Inserted at chain position: " << examined + 1 << " (tail)\n";
    lastReport = report.str();
    return true;
}

template<class KeyType, class ValueType>
bool SeparateChainingHash<KeyType, ValueType>::search(const KeyType& key, ValueType& result) const
{
    int bucket = hash(key);
    int comparisons = 0;
    int position = 0;
    std::ostringstream report;

    for (ChainNode<KeyType, ValueType>* current = table[bucket]; current != nullptr; current = current->getNext())
    {
        ++comparisons;
        ++position;
        if (current->getKey() == key)
        {
            result = current->getValue();
            // [REQ §9.2 Search] chaining reports bucket number, chain position, comparisons
            report << "Bucket (chain) number: " << bucket << "\n"
                   << "Chain position:        " << position << "\n"
                   << "Comparisons:           " << comparisons << "\n";
            lastReport = report.str();
            stats.recordSearch(true, comparisons);
            return true;
        }
    }

    report << "Bucket (chain) number: " << bucket << "\n"
           << "Comparisons:           " << comparisons
           << " (unsuccessful: reached the end of the chain)\n";
    lastReport = report.str();
    stats.recordSearch(false, comparisons);
    return false;
}

template<class KeyType, class ValueType>
bool SeparateChainingHash<KeyType, ValueType>::remove(const KeyType& key)
{
    int bucket = hash(key);
    ChainNode<KeyType, ValueType>* previous = nullptr;
    ChainNode<KeyType, ValueType>* current = table[bucket];
    int position = 1;

    while (current != nullptr && !(current->getKey() == key))
    {
        previous = current;
        current = current->getNext();
        ++position;
    }

    std::ostringstream report;
    if (current == nullptr)
    {
        report << "Patient ID " << key << " not found in chain " << bucket << ".\n";
        lastReport = report.str();
        return false;
    }

    // [REQ §9.4 Delete] chaining deletion of the head, middle, tail, or only node
    std::string kind;
    if (previous == nullptr && current->getNext() == nullptr) kind = "ONLY node";
    else if (previous == nullptr)                           kind = "HEAD node";
    else if (current->getNext() == nullptr)                 kind = "TAIL node";
    else                                                    kind = "MIDDLE node";

    if (previous == nullptr)
        table[bucket] = current->getNext();          // head (or only) node: move the head pointer
    else
        previous->setNext(current->getNext());       // middle or tail: bypass the node

    delete current;
    --count;

    report << "Deleted the " << kind << " of chain " << bucket
           << " (position " << position << "). Node memory released.\n";
    lastReport = report.str();
    return true;
}

template<class KeyType, class ValueType>
bool SeparateChainingHash<KeyType, ValueType>::update(const KeyType& key, const ValueType& newValue)
{
    int bucket = hash(key);
    for (ChainNode<KeyType, ValueType>* current = table[bucket]; current != nullptr; current = current->getNext())
    {
        if (current->getKey() == key)
        {
            current->setValue(newValue);
            lastReport = "Record in chain " + std::to_string(bucket) + " updated (key unchanged).\n";
            return true;
        }
    }
    lastReport = "Patient ID not found.\n";
    return false;
}

template<class KeyType, class ValueType>
void SeparateChainingHash<KeyType, ValueType>::clear()
{
    destroyAllNodes();
    count = 0;
    stats.reset();
    lastReport = "All chains cleared and every node released; statistics reset.\n";
}

template<class KeyType, class ValueType>
bool SeparateChainingHash<KeyType, ValueType>::isFull() const
{
    return false;   // chains grow dynamically
}

template<class KeyType, class ValueType>
int SeparateChainingHash<KeyType, ValueType>::getTableSize() const
{
    return tableSize;
}

template<class KeyType, class ValueType>
int SeparateChainingHash<KeyType, ValueType>::getChainLength(int index) const
{
    int length = 0;
    for (ChainNode<KeyType, ValueType>* current = table[index]; current != nullptr; current = current->getNext())
        ++length;
    return length;
}

// [REQ §7 Rehashing]
// Rehash: every node is re-hashed with the NEW table size and relinked
// into its new chain. Nodes are moved, not copied, so no memory is lost.
template<class KeyType, class ValueType>
bool SeparateChainingHash<KeyType, ValueType>::rehash(int newSize)
{
    int target = hashutil::nextPrime(newSize < 2 ? 2 : newSize);
    ChainNode<KeyType, ValueType>** oldTable = table;
    int oldSize = tableSize;

    table = new ChainNode<KeyType, ValueType>*[target]();
    tableSize = target;

    for (int i = 0; i < oldSize; ++i)
    {
        ChainNode<KeyType, ValueType>* current = oldTable[i];
        while (current != nullptr)
        {
            ChainNode<KeyType, ValueType>* next = current->getNext();
            current->setNext(nullptr);
            appendToChain(table[hash(current->getKey())], current);
            current = next;
        }
    }

    delete[] oldTable;   // only the array of heads; the nodes now belong to the new table
    stats.recordRehash();

    std::ostringstream report;
    report << std::fixed << std::setprecision(3)
           << "Rehashed Separate Chaining: table size " << oldSize << " -> " << tableSize
           << ". " << count << " records re-hashed into new chains.\n"
           << "New load factor: " << this->getLoadFactor() << "\n";
    lastReport = report.str();
    return true;
}

template<class KeyType, class ValueType>
std::vector<std::pair<KeyType, ValueType>> SeparateChainingHash<KeyType, ValueType>::getAllEntries() const
{
    std::vector<std::pair<KeyType, ValueType>> entries;
    for (int i = 0; i < tableSize; ++i)
    {
        for (ChainNode<KeyType, ValueType>* current = table[i]; current != nullptr; current = current->getNext())
            entries.push_back(std::make_pair(current->getKey(), current->getValue()));
    }
    return entries;
}

template<class KeyType, class ValueType>
HashStatistics SeparateChainingHash<KeyType, ValueType>::getStatistics() const
{
    // [REQ §11 Statistics] longest chain and average chain length
    int longest = 0;
    int nonEmpty = 0;
    for (int i = 0; i < tableSize; ++i)
    {
        int length = getChainLength(i);
        if (length > 0) ++nonEmpty;
        if (length > longest) longest = length;
    }
    HashStatistics current = stats;
    current.setChainStats(longest, nonEmpty == 0 ? 0.0 : static_cast<double>(count) / nonEmpty);
    return current;
}

template<class KeyType, class ValueType>
void SeparateChainingHash<KeyType, ValueType>::print(std::ostream& os) const
{
    std::ios oldState(nullptr);
    oldState.copyfmt(os);

    HashStatistics current = getStatistics();
    os << std::fixed << std::setprecision(3)
       << "Method: Separate Chaining | Table size: " << tableSize
       << " | Records: " << count
       << " | Load factor: " << this->getLoadFactor()
       << " | Longest chain: " << current.getLongestChain() << "\n";
    os << std::left << std::setw(7) << "Index" << "Chain\n";
    os << std::string(60, '-') << "\n";

    for (int i = 0; i < tableSize; ++i)
    {
        os << std::setw(7) << i;
        if (table[i] == nullptr)
        {
            os << "EMPTY\n";
            continue;
        }
        for (ChainNode<KeyType, ValueType>* node = table[i]; node != nullptr; node = node->getNext())
            os << "[" << node->getKey() << ", " << node->getValue() << "] -> ";
        os << "NULL\n";
    }

    os.copyfmt(oldState);
}

#endif
