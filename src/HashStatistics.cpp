// HashStatistics.cpp

#include "HashStatistics.h"
#include <iomanip>

HashStatistics::HashStatistics()
{
    reset();
}

void HashStatistics::reset()
{
    insertions = 0;
    collisions = 0;
    totalProbes = 0;
    maxProbes = 0;
    successfulSearches = 0;
    successfulComparisons = 0;
    unsuccessfulSearches = 0;
    unsuccessfulComparisons = 0;
    rehashCount = 0;
    overflowInsertions = 0;
    currentOverflow = 0;
    longestChain = 0;
    averageChainLength = 0.0;
}

void HashStatistics::recordInsertion(int probes, bool collided)
{
    ++insertions;
    totalProbes += probes;
    if (probes > maxProbes) maxProbes = probes;
    if (collided) ++collisions;
}

void HashStatistics::recordSearch(bool found, int comparisons)
{
    if (found)
    {
        ++successfulSearches;
        successfulComparisons += comparisons;
    }
    else
    {
        ++unsuccessfulSearches;
        unsuccessfulComparisons += comparisons;
    }
}

void HashStatistics::recordRehash() { ++rehashCount; }
void HashStatistics::recordOverflowInsertion() { ++overflowInsertions; }
void HashStatistics::setCurrentOverflow(int records) { currentOverflow = records; }

void HashStatistics::setChainStats(int longest, double average)
{
    longestChain = longest;
    averageChainLength = average;
}

int HashStatistics::getInsertions() const { return insertions; }
int HashStatistics::getCollisions() const { return collisions; }
long HashStatistics::getTotalProbes() const { return totalProbes; }
int HashStatistics::getMaxProbes() const { return maxProbes; }

double HashStatistics::getAverageProbes() const
{
    return insertions == 0 ? 0.0 : static_cast<double>(totalProbes) / insertions;
}

int HashStatistics::getSuccessfulSearches() const { return successfulSearches; }
long HashStatistics::getSuccessfulComparisons() const { return successfulComparisons; }

double HashStatistics::getAverageSuccessfulComparisons() const
{
    return successfulSearches == 0 ? 0.0
         : static_cast<double>(successfulComparisons) / successfulSearches;
}

int HashStatistics::getUnsuccessfulSearches() const { return unsuccessfulSearches; }
long HashStatistics::getUnsuccessfulComparisons() const { return unsuccessfulComparisons; }

double HashStatistics::getAverageUnsuccessfulComparisons() const
{
    return unsuccessfulSearches == 0 ? 0.0
         : static_cast<double>(unsuccessfulComparisons) / unsuccessfulSearches;
}

int HashStatistics::getRehashCount() const { return rehashCount; }
int HashStatistics::getOverflowInsertions() const { return overflowInsertions; }
int HashStatistics::getCurrentOverflow() const { return currentOverflow; }
int HashStatistics::getLongestChain() const { return longestChain; }
double HashStatistics::getAverageChainLength() const { return averageChainLength; }

void HashStatistics::display(std::ostream& os,
                             const std::string& methodName,
                             int recordCount,
                             int tableSize,
                             double loadFactor,
                             bool showChains,
                             bool showOverflow) const
{
    std::ios oldState(nullptr);
    oldState.copyfmt(os);

    os << std::fixed << std::setprecision(3);
    os << "---------------- HASH STATISTICS ----------------\n";
    os << std::left;
    os << std::setw(40) << "Method:" << methodName << "\n";
    os << std::setw(40) << "Number of records:" << recordCount << "\n";
    os << std::setw(40) << "Table size:" << tableSize << "\n";
    os << std::setw(40) << "Load factor:" << loadFactor << "\n";
    os << std::setw(40) << "Number of collisions:" << collisions << "\n";
    os << std::setw(40) << "Total probes (insertions):" << totalProbes << "\n";
    os << std::setw(40) << "Maximum probes for one insertion:" << maxProbes << "\n";
    os << std::setw(40) << "Average probes per insertion:" << getAverageProbes() << "\n";
    os << std::setw(40) << "Successful searches:" << successfulSearches
       << "  (total comparisons " << successfulComparisons
       << ", avg " << getAverageSuccessfulComparisons() << ")\n";
    os << std::setw(40) << "Unsuccessful searches:" << unsuccessfulSearches
       << "  (total comparisons " << unsuccessfulComparisons
       << ", avg " << getAverageUnsuccessfulComparisons() << ")\n";

    if (showChains)
    {
        os << std::setw(40) << "Longest chain:" << longestChain << "\n";
        os << std::setw(40) << "Average chain length (non-empty):" << averageChainLength << "\n";
    }
    if (showOverflow)
    {
        os << std::setw(40) << "Records in overflow now:" << currentOverflow << "\n";
        os << std::setw(40) << "Total overflow insertions:" << overflowInsertions << "\n";
    }

    os << std::setw(40) << "Number of rehashes:" << rehashCount << "\n";
    os << "-------------------------------------------------\n";

    os.copyfmt(oldState);
}
