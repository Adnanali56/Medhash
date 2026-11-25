// HashEntry.h
// One slot of an open-addressed (or bucket) table.
// A slot is EMPTY, OCCUPIED, or DELETED. DELETED is required for open
// addressing: making a deleted slot EMPTY would cut later probe sequences short.

#ifndef HASHENTRY_H
#define HASHENTRY_H

enum class EntryState { EMPTY, OCCUPIED, DELETED };

inline const char* stateName(EntryState state)
{
    switch (state)
    {
        case EntryState::EMPTY:    return "EMPTY";
        case EntryState::OCCUPIED: return "OCCUPIED";
        case EntryState::DELETED:  return "DELETED";
    }
    return "UNKNOWN";
}

template<class KeyType, class ValueType>
class HashEntry
{
public:
    HashEntry() : key(), value(), state(EntryState::EMPTY) {}

    HashEntry(const KeyType& k, const ValueType& v)
        : key(k), value(v), state(EntryState::OCCUPIED) {}

    const KeyType& getKey() const { return key; }
    const ValueType& getValue() const { return value; }
    EntryState getState() const { return state; }
    bool isOccupied() const { return state == EntryState::OCCUPIED; }

    void set(const KeyType& k, const ValueType& v)
    {
        key = k;
        value = v;
        state = EntryState::OCCUPIED;
    }

    void setValue(const ValueType& v) { value = v; }
    void markDeleted() { state = EntryState::DELETED; }

    void makeEmpty()
    {
        key = KeyType();
        value = ValueType();
        state = EntryState::EMPTY;
    }

private:
    KeyType key;
    ValueType value;
    EntryState state;
};

#endif
