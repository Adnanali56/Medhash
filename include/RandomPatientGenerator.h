// RandomPatientGenerator.h
// Produces fictional (synthetic) patient records. Duplicate IDs are detected
// and regenerated so every generated patient ID is unique.

#ifndef RANDOMPATIENTGENERATOR_H
#define RANDOMPATIENTGENERATOR_H

#include "PatientRecord.h"
#include <random>
#include <vector>

class RandomPatientGenerator
{
public:
    static const int MIN_ID = 100000;
    static const int MAX_ID = 999999;

    RandomPatientGenerator();
    explicit RandomPatientGenerator(unsigned int seed);

    // n new records whose IDs are not in existingIDs (and not repeated)
    std::vector<PatientRecord> generateUnique(int n, const std::vector<int>& existingIDs);

    // n IDs that are NOT in existingIDs (used to measure unsuccessful searches)
    std::vector<int> generateAbsentIDs(int n, const std::vector<int>& existingIDs);

    // How many duplicate IDs were detected and regenerated in the last call
    int getLastDuplicateCount() const;

private:
    PatientRecord makeRecord(int id);
    int randomInt(int low, int high);

    std::mt19937 engine;
    int lastDuplicateCount;
};

#endif
