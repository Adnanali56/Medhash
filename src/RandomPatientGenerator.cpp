// RandomPatientGenerator.cpp

#include "RandomPatientGenerator.h"
#include <string>

namespace
{
    const char* FIRST_NAMES[] = {
        "Maya", "Jordan", "Ava", "Sam", "Liam", "Noah", "Emma", "Olivia", "Ethan", "Sofia",
        "Lucas", "Mia", "Aria", "Leo", "Zara", "Omar", "Priya", "Chen", "Elena", "Mateo",
        "Nina", "Kai", "Isla", "Ravi", "Hana", "Diego", "Amara", "Felix", "Lena", "Yusuf"
    };
    const char* LAST_NAMES[] = {
        "Patel", "Lee", "Kumar", "Chen", "Garcia", "Smith", "Nguyen", "Khan", "Brown", "Lopez",
        "Wilson", "Ali", "Kim", "Martin", "Silva", "Okafor", "Novak", "Rossi", "Haddad", "Tanaka",
        "Moreau", "Singh", "Clark", "Diaz", "Ahmed", "Bauer", "Costa", "Ivanov", "Reyes", "Young"
    };
    const char* BLOOD_TYPES[] = { "A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-" };

    const int FIRST_COUNT = sizeof(FIRST_NAMES) / sizeof(FIRST_NAMES[0]);
    const int LAST_COUNT = sizeof(LAST_NAMES) / sizeof(LAST_NAMES[0]);
    const int BLOOD_COUNT = sizeof(BLOOD_TYPES) / sizeof(BLOOD_TYPES[0]);
}

RandomPatientGenerator::RandomPatientGenerator()
    : engine(std::random_device{}()), lastDuplicateCount(0)
{
}

RandomPatientGenerator::RandomPatientGenerator(unsigned int seed)
    : engine(seed), lastDuplicateCount(0)
{
}

int RandomPatientGenerator::randomInt(int low, int high)
{
    std::uniform_int_distribution<int> dist(low, high);
    return dist(engine);
}

PatientRecord RandomPatientGenerator::makeRecord(int id)
{
    std::string first = FIRST_NAMES[randomInt(0, FIRST_COUNT - 1)];
    std::string last = LAST_NAMES[randomInt(0, LAST_COUNT - 1)];
    int age = randomInt(1, 100);
    std::string blood = BLOOD_TYPES[randomInt(0, BLOOD_COUNT - 1)];
    std::string diagnosis = "DX" + std::to_string(randomInt(100, 999));
    int priority = randomInt(1, 5);
    return PatientRecord(id, first, last, age, blood, diagnosis, priority);
}

std::vector<PatientRecord> RandomPatientGenerator::generateUnique(int n, const std::vector<int>& existingIDs)
{
    // used[id] marks IDs already taken (vector<bool> is a utility, not a dictionary)
    std::vector<bool> used(MAX_ID + 1, false);
    for (int id : existingIDs)
    {
        if (id >= 0 && id <= MAX_ID) used[id] = true;
    }

    std::vector<PatientRecord> records;
    lastDuplicateCount = 0;
    int available = (MAX_ID - MIN_ID + 1) - static_cast<int>(existingIDs.size());
    if (n > available) n = available;

    while (static_cast<int>(records.size()) < n)
    {
        int id = randomInt(MIN_ID, MAX_ID);
        if (used[id])
        {
            ++lastDuplicateCount;   // duplicate detected -> regenerate
            continue;
        }
        used[id] = true;
        records.push_back(makeRecord(id));
    }
    return records;
}

std::vector<int> RandomPatientGenerator::generateAbsentIDs(int n, const std::vector<int>& existingIDs)
{
    std::vector<bool> used(MAX_ID + 1, false);
    for (int id : existingIDs)
    {
        if (id >= 0 && id <= MAX_ID) used[id] = true;
    }

    std::vector<int> ids;
    while (static_cast<int>(ids.size()) < n)
    {
        int id = randomInt(MIN_ID, MAX_ID);
        if (!used[id])
        {
            used[id] = true;
            ids.push_back(id);
        }
    }
    return ids;
}

int RandomPatientGenerator::getLastDuplicateCount() const
{
    return lastDuplicateCount;
}
