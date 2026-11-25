// HashStatistics.h
// Counters measured by every hash table while the program runs.
// Nothing here is hard-coded: every value comes from real operations.
//
// Definitions used throughout the project:
//   probes      = table positions (or chain nodes) examined to place a record
//   collision   = an insertion whose home position was already in use
//   comparisons = positions/nodes examined during a search

#ifndef HASHSTATISTICS_H
#define HASHSTATISTICS_H

#include <iostream>
#include <string>

// [REQ §11 Hash Statistics] all counters required by section 11
class HashStatistics
{
public:
    HashStatistics();

    void reset();

    void recordInsertion(int probes, bool collided);
    void recordSearch(bool found, int comparisons);
    void recordRehash();
    void recordOverflowInsertion();

    void setCurrentOverflow(int records);
    void setChainStats(int longest, double average);

    int getInsertions() const;
    int getCollisions() const;
    long getTotalProbes() const;
    int getMaxProbes() const;
    double getAverageProbes() const;

    int getSuccessfulSearches() const;
    long getSuccessfulComparisons() const;
    double getAverageSuccessfulComparisons() const;
    int getUnsuccessfulSearches() const;
    long getUnsuccessfulComparisons() const;
    double getAverageUnsuccessfulComparisons() const;

    int getRehashCount() const;
    int getOverflowInsertions() const;
    int getCurrentOverflow() const;
    int getLongestChain() const;
    double getAverageChainLength() const;

    // Prints only the statistics that make sense for the method in use.
    void display(std::ostream& os,
                 const std::string& methodName,
                 int recordCount,
                 int tableSize,
                 double loadFactor,
                 bool showChains,
                 bool showOverflow) const;

private:
    int insertions;
    int collisions;
    long totalProbes;
    int maxProbes;

    int successfulSearches;
    long successfulComparisons;
    int unsuccessfulSearches;
    long unsuccessfulComparisons;

    int rehashCount;
    int overflowInsertions;   // total records ever sent to overflow
    int currentOverflow;      // records in overflow right now

    int longestChain;
    double averageChainLength;
};

#endif
