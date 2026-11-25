// ChainNode.h
// A singly linked node, written from scratch (no std::list).
// Used for separate chaining and for the bucket overflow area.

#ifndef CHAINNODE_H
#define CHAINNODE_H

template<class KeyType, class ValueType>
class ChainNode
{
public:
    ChainNode(const KeyType& k, const ValueType& v, ChainNode* nextNode = nullptr)
        : key(k), value(v), next(nextNode) {}

    const KeyType& getKey() const { return key; }
    const ValueType& getValue() const { return value; }
    ChainNode* getNext() const { return next; }

    void setValue(const ValueType& v) { value = v; }
    void setNext(ChainNode* nextNode) { next = nextNode; }

private:
    KeyType key;
    ValueType value;
    ChainNode* next;
};

#endif
