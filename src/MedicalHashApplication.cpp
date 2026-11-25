// MedicalHashApplication.cpp

#include "MedicalHashApplication.h"
#include "LinearProbingHash.h"
#include "QuadraticProbingHash.h"
#include "DoubleHash.h"
#include "SeparateChainingHash.h"
#include "BucketHash.h"
#include "AlternateBucketHash.h"
#include "InputEnded.h"

#include <cctype>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

using std::cout;
using std::string;
using std::vector;

// ===================== Input helpers =====================
namespace
{
    string readLine(const string& prompt)
    {
        cout << prompt;
        string line;
        if (!std::getline(std::cin, line))
            throw InputEnded();
        // trim surrounding spaces
        size_t start = line.find_first_not_of(" \t\r");
        size_t end = line.find_last_not_of(" \t\r");
        return start == string::npos ? "" : line.substr(start, end - start + 1);
    }

    bool parseInt(const string& text, int& value)
    {
        if (text.empty()) return false;
        std::istringstream in(text);
        in >> value;
        return !in.fail() && in.eof();
    }

    int readInt(const string& prompt, int low, int high)
    {
        while (true)
        {
            int value = 0;
            string text = readLine(prompt);
            if (parseInt(text, value) && value >= low && value <= high)
                return value;
            cout << "  Please enter a whole number from " << low << " to " << high << ".\n";
        }
    }

    // Empty input returns the default value
    int readIntDefault(const string& prompt, int low, int high, int defaultValue)
    {
        while (true)
        {
            int value = 0;
            string text = readLine(prompt + " [default " + std::to_string(defaultValue) + "]: ");
            if (text.empty())
                return defaultValue;
            if (parseInt(text, value) && value >= low && value <= high)
                return value;
            cout << "  Please enter a whole number from " << low << " to " << high << ".\n";
        }
    }

    string readNonEmpty(const string& prompt)
    {
        while (true)
        {
            string text = readLine(prompt);
            if (!text.empty() && text.find(',') == string::npos)
                return text;
            cout << "  Please enter a value (commas are not allowed).\n";
        }
    }

    string readBloodType(const string& prompt)
    {
        while (true)
        {
            string text = readLine(prompt);
            for (char& c : text) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            if (PatientRecord::isValidBloodType(text))
                return text;
            cout << "  Valid blood types: A+, A-, B+, B-, AB+, AB-, O+, O-\n";
        }
    }

    string readDiagnosisCode(const string& prompt)
    {
        while (true)
        {
            string text = readLine(prompt);
            for (char& c : text) c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
            int number = 0;
            if (text.size() == 5 && text.substr(0, 2) == "DX" && parseInt(text.substr(2), number)
                && number >= 100 && number <= 999)
                return text;
            cout << "  Use the format DX100 to DX999.\n";
        }
    }

    bool readYesNo(const string& prompt)
    {
        while (true)
        {
            string text = readLine(prompt + " (y/n): ");
            if (text == "y" || text == "Y") return true;
            if (text == "n" || text == "N") return false;
        }
    }

    void printBanner()
    {
        cout << "\n==================================================\n"
             << "   MEDICAL RECORD HASHING SYSTEM\n"
             << "==================================================\n";
    }
}

// ===================== Construction =====================

MedicalHashApplication::MedicalHashApplication()
    : dictionary(nullptr), strategy(0), generator()
{
}

MedicalHashApplication::~MedicalHashApplication()
{
    delete dictionary;
}

string MedicalHashApplication::strategyName(int strategyNumber)
{
    switch (strategyNumber)
    {
        case LINEAR:           return "Linear Probing";
        case QUADRATIC:        return "Quadratic Probing";
        case DOUBLE_HASH:      return "Double Hashing";
        case CHAINING:         return "Separate Chaining";
        case BUCKET:           return "Bucket Hashing";
        case ALTERNATE_BUCKET: return "Alternate Bucket Hashing + Overflow";
        default:               return "Unknown";
    }
}

string MedicalHashApplication::shortStrategyName(int strategyNumber)
{
    if (strategyNumber == ALTERNATE_BUCKET) return "Alternate Buckets";
    return strategyName(strategyNumber);
}

MedicalHashApplication::Dictionary*
MedicalHashApplication::createDictionary(int strategyNumber, int tableSize, int bucketSize)
{
    // [REQ §5 All strategies] [REQ §16 Polymorphism]
    // Every concrete table is returned as a DictionaryADT pointer
    switch (strategyNumber)
    {
        case LINEAR:           return new LinearProbingHash<int, PatientRecord>(tableSize);
        case QUADRATIC:        return new QuadraticProbingHash<int, PatientRecord>(tableSize);
        case DOUBLE_HASH:      return new DoubleHash<int, PatientRecord>(tableSize);
        case CHAINING:         return new SeparateChainingHash<int, PatientRecord>(tableSize);
        case BUCKET:           return new BucketHash<int, PatientRecord>(tableSize, bucketSize);
        case ALTERNATE_BUCKET: return new AlternateBucketHash<int, PatientRecord>(tableSize, bucketSize);
        default:               return nullptr;
    }
}

vector<int> MedicalHashApplication::currentIDs() const
{
    vector<int> ids;
    if (dictionary == nullptr) return ids;
    for (const auto& entry : dictionary->getAllEntries())
        ids.push_back(entry.first);
    return ids;
}

// ===================== Menus =====================

void MedicalHashApplication::run()
{
    while (true)
    {
        int choice = showStrategyMenu();
        if (choice == 0)
        {
            cout << "Goodbye.\n";
            return;
        }
        if (choice == 7)
        {
            compareWithNewDataset();
            continue;
        }
        if (chooseStrategy(choice))
        {
            applicationMenu();   // returns only when the user exits the program
            cout << "Goodbye.\n";
            return;
        }
    }
}

int MedicalHashApplication::showStrategyMenu() const
{
    printBanner();
    cout << "\nSelect Hashing / Collision Strategy\n\n"
         << "1. Linear Probing\n"
         << "2. Quadratic Probing\n"
         << "3. Double Hashing\n"
         << "4. Separate Chaining\n"
         << "5. Bucket Hashing\n"
         << "6. Alternate Bucket Hashing + Overflow\n"
         << "7. Compare All Techniques\n"
         << "0. Exit\n\n";
    return readInt("Selection: ", 0, 7);
}

// Creates the chosen table (asks for its size). Returns false on invalid choice.
bool MedicalHashApplication::chooseStrategy(int choice)
{
    if (choice < LINEAR || choice > ALTERNATE_BUCKET)
        return false;

    int size = 0;
    int bucketSize = 3;
    if (choice == BUCKET || choice == ALTERNATE_BUCKET)
    {
        size = readIntDefault("Total number of slots", 2, 1000000, 15);
        bucketSize = readIntDefault("Slots per bucket", 1, 100, 3);
    }
    else
    {
        size = readIntDefault("Initial table size (rounded up to a prime)", 2, 1000000, 11);
    }

    delete dictionary;
    dictionary = createDictionary(choice, size, bucketSize);
    strategy = choice;

    cout << "\nCreated " << dictionary->getMethodName() << " with table size "
         << dictionary->getTableSize() << ". Automatic rehash: "
         << (dictionary->getAutoRehash() ? "ON" : "OFF") << " (threshold "
         << dictionary->getMaxLoadFactor() << ").\n";
    return true;
}

void MedicalHashApplication::applicationMenu()
{
    while (true)
    {
        printBanner();
        cout << "Active method: " << dictionary->getMethodName()
             << " | Records: " << dictionary->getCount()
             << " | Table size: " << dictionary->getTableSize() << "\n\n"
             << "1. Add Patient Record\n"
             << "2. Search for Patient\n"
             << "3. Update Patient Record\n"
             << "4. Delete Patient Record\n"
             << "5. Display Entire Hash Table\n"
             << "6. Display Patient Count\n"
             << "7. Display Hash Statistics\n"
             << "8. Generate Random Patient Records\n"
             << "9. Change Collision Strategy\n"
             << "10. Rehash / Resize Table\n"
             << "11. Test Copy Constructor\n"
             << "12. Test Assignment Operator\n"
             << "13. Clear Database\n"
             << "14. Compare Hashing Techniques\n"
             << "0. Exit\n\n";

        int choice = readInt("Enter choice: ", 0, 14);
        cout << "\n";
        switch (choice)
        {
            case 1:  addPatient(); break;
            case 2:  searchPatient(); break;
            case 3:  updatePatient(); break;
            case 4:  deletePatient(); break;
            case 5:  displayTable(); break;
            case 6:  displayCount(); break;
            case 7:  displayStatistics(); break;
            case 8:  generateRandomRecords(); break;
            case 9:  changeStrategy(); break;
            case 10: rehashMenu(); break;
            case 11: testCopyConstructor(); break;
            case 12: testAssignmentOperator(); break;
            case 13: clearDatabase(); break;
            case 14: compareFromCurrentData(); break;
            case 0:  return;
        }
    }
}

// ===================== Operations =====================

PatientRecord MedicalHashApplication::promptPatientFields(int id) const
{
    string first = readNonEmpty("First name: ");
    string last = readNonEmpty("Last name: ");
    int age = readInt("Age (1-100): ", 1, 100);
    string blood = readBloodType("Blood type (A+, A-, B+, B-, AB+, AB-, O+, O-): ");
    string diagnosis = readDiagnosisCode("Diagnosis code (DX100-DX999): ");
    int priority = readInt("Priority level (1-5): ", 1, 5);
    return PatientRecord(id, first, last, age, blood, diagnosis, priority);
}

void MedicalHashApplication::addPatient()
{
    cout << "--- Add Patient (synthetic data only) ---\n";
    int id = readInt("Patient ID (1-999999): ", 1, 999999);

    // Early duplicate check (without touching the search statistics);
    // insert() also rejects duplicates on its own.
    for (int existingID : currentIDs())
    {
        if (existingID == id)
        {
            cout << "Patient ID " << id << " already exists. Duplicate IDs are not allowed.\n";
            return;
        }
    }

    PatientRecord record = promptPatientFields(id);
    bool inserted = dictionary->insert(id, record);
    cout << "\n" << dictionary->getLastReport();
    cout << (inserted ? "Patient successfully inserted.\n" : "Patient was NOT inserted.\n");
}

void MedicalHashApplication::searchPatient()
{
    int id = readInt("Patient ID to search: ", 1, 999999);
    PatientRecord record;
    if (dictionary->search(id, record))
    {
        cout << "\nPatient found.\n\n";
        record.display(cout);
        cout << "\n" << dictionary->getLastReport();
    }
    else
    {
        cout << "\nPatient " << id << " not found.\n" << dictionary->getLastReport();
    }
}

void MedicalHashApplication::updatePatient()
{
    int id = readInt("Patient ID to update: ", 1, 999999);
    PatientRecord record;
    if (!dictionary->search(id, record))
    {
        cout << "Patient " << id << " not found.\n";
        return;
    }

    cout << "\nCurrent record:\n";
    record.display(cout);
    cout << "\nThe patient ID (key) cannot be changed by an update.\n"
         << "To change an ID, delete the record and add it again with the new ID.\n\n"
         << "1. First name\n2. Last name\n3. Age\n4. Blood type\n"
         << "5. Diagnosis code\n6. Priority level\n7. All fields\n0. Cancel\n";

    int field = readInt("Field to update: ", 0, 7);
    switch (field)
    {
        case 0: cout << "Update cancelled.\n"; return;
        case 1: record.setFirstName(readNonEmpty("New first name: ")); break;
        case 2: record.setLastName(readNonEmpty("New last name: ")); break;
        case 3: record.setAge(readInt("New age (1-100): ", 1, 100)); break;
        case 4: record.setBloodType(readBloodType("New blood type: ")); break;
        case 5: record.setDiagnosisCode(readDiagnosisCode("New diagnosis code: ")); break;
        case 6: record.setPriorityLevel(readInt("New priority (1-5): ", 1, 5)); break;
        case 7: record = promptPatientFields(id); break;
    }

    if (dictionary->update(id, record))
    {
        cout << "\n" << dictionary->getLastReport() << "Updated record:\n";
        record.display(cout);
    }
}

void MedicalHashApplication::deletePatient()
{
    int id = readInt("Patient ID to delete: ", 1, 999999);
    bool removed = dictionary->remove(id);
    cout << dictionary->getLastReport();
    if (removed)
        cout << "Patient deleted.\n";
}

void MedicalHashApplication::displayTable() const
{
    cout << *dictionary;   // operator<< on the dictionary
}

// [REQ §7 Load Factor] shows count, table size and load factor = count / tableSize
void MedicalHashApplication::displayCount() const
{
    cout << "Patient records: " << dictionary->getCount() << "\n"
         << "Table size:      " << dictionary->getTableSize() << "\n"
         << std::fixed << std::setprecision(3)
         << "Load factor:     " << dictionary->getLoadFactor()
         << "  (records / table size = " << dictionary->getCount() << " / "
         << dictionary->getTableSize() << ")\n";
    cout.unsetf(std::ios::fixed);
}

// [REQ §11 Hash Statistics]
void MedicalHashApplication::displayStatistics() const
{
    dictionary->displayStatistics(cout);
}

void MedicalHashApplication::generateRandomRecords()
{
    int n = readIntDefault("How many random patient records", 1, 100000, 10);
    vector<PatientRecord> records = generator.generateUnique(n, currentIDs());

    int inserted = 0;
    int rehashesBefore = dictionary->getStatistics().getRehashCount();
    for (const PatientRecord& record : records)
    {
        if (dictionary->insert(record.getPatientID(), record))
            ++inserted;
        else
            cout << dictionary->getLastReport();
    }
    int rehashes = dictionary->getStatistics().getRehashCount() - rehashesBefore;

    cout << "Generated " << records.size() << " synthetic records ("
         << generator.getLastDuplicateCount() << " duplicate IDs detected and regenerated).\n"
         << "Inserted: " << inserted << " | Automatic rehashes during insertion: " << rehashes << "\n";
    displayCount();
}

void MedicalHashApplication::changeStrategy()
{
    cout << "Current method: " << dictionary->getMethodName() << "\n\n"
         << "1. Linear Probing\n2. Quadratic Probing\n3. Double Hashing\n"
         << "4. Separate Chaining\n5. Bucket Hashing\n6. Alternate Bucket Hashing + Overflow\n0. Cancel\n";
    int choice = readInt("New strategy: ", 0, 6);
    if (choice == 0) return;

    vector<std::pair<int, PatientRecord>> entries = dictionary->getAllEntries();

    int size = 0;
    int bucketSize = 3;
    if (choice == BUCKET || choice == ALTERNATE_BUCKET)
    {
        size = readIntDefault("Total number of slots", 2, 1000000, dictionary->getTableSize());
        bucketSize = readIntDefault("Slots per bucket", 1, 100, 3);
    }
    else
    {
        size = readIntDefault("Initial table size", 2, 1000000, dictionary->getTableSize());
    }

    Dictionary* replacement = createDictionary(choice, size, bucketSize);
    int moved = 0;
    for (const auto& entry : entries)
    {
        if (replacement->insert(entry.first, entry.second))
            ++moved;
    }

    delete dictionary;   // release the old structure
    dictionary = replacement;
    strategy = choice;

    cout << "Switched to " << dictionary->getMethodName() << ". "
         << moved << " of " << entries.size() << " records moved into the new structure.\n";
}

// [REQ §7 Rehashing / Load Factor] manual rehash and automatic-rehash switch
void MedicalHashApplication::rehashMenu()
{
    cout << std::fixed << std::setprecision(3)
         << "Table size: " << dictionary->getTableSize()
         << " | Records: " << dictionary->getCount()
         << " | Load factor: " << dictionary->getLoadFactor() << "\n"
         << "Automatic rehash: " << (dictionary->getAutoRehash() ? "ON" : "OFF")
         << " (threshold " << dictionary->getMaxLoadFactor() << ")\n\n";
    cout.unsetf(std::ios::fixed);

    cout << "1. Rehash / resize now\n2. Turn automatic rehash "
         << (dictionary->getAutoRehash() ? "OFF" : "ON") << "\n0. Back\n";
    int choice = readInt("Choice: ", 0, 2);

    if (choice == 1)
    {
        int newSize = readIntDefault("New table size (rounded up to a prime)", 2, 1000000,
                                     dictionary->getTableSize() * 2);
        bool small = dictionary->getTableSize() <= 40;
        if (small)
        {
            cout << "\nBEFORE:\n" << *dictionary;
        }
        dictionary->rehash(newSize);
        cout << "\n" << dictionary->getLastReport();
        if (dictionary->getTableSize() <= 60)
        {
            cout << "\nAFTER:\n" << *dictionary;
        }
    }
    else if (choice == 2)
    {
        dictionary->setAutoRehash(!dictionary->getAutoRehash());
        cout << "Automatic rehash is now " << (dictionary->getAutoRehash() ? "ON" : "OFF") << ".\n";
    }
}

// ----- Copy constructor test (uses the concrete type so the real copy constructor runs)
template<class TableType>
void MedicalHashApplication::copyConstructorTest()
{
    TableType* original = dynamic_cast<TableType*>(dictionary);
    if (original == nullptr) return;

    TableType copy(*original);   // COPY CONSTRUCTOR
    cout << "Copy created with the copy constructor: " << copy.getMethodName()
         << ", " << copy.getCount() << " records.\n";

    // Change the ORIGINAL only, then show that the copy did not change (deep copy).
    vector<PatientRecord> extra = generator.generateUnique(1, currentIDs());
    int testID = extra[0].getPatientID();
    original->insert(testID, extra[0]);

    PatientRecord found;
    bool inOriginal = original->search(testID, found);
    bool inCopy = copy.search(testID, found);

    cout << "Inserted test patient " << testID << " into the ORIGINAL only.\n"
         << "  Original records: " << original->getCount()
         << " | contains " << testID << ": " << (inOriginal ? "yes" : "no") << "\n"
         << "  Copy records:     " << copy.getCount()
         << " | contains " << testID << ": " << (inCopy ? "yes" : "no") << "\n";

    if (original->getTableSize() <= 40)
    {
        cout << "\n--- ORIGINAL ---\n" << *original << "\n--- COPY ---\n" << copy;
    }

    cout << ((inOriginal && !inCopy)
             ? "\nDEEP COPY CONFIRMED: the two tables do not share memory.\n"
             : "\nWarning: the copy changed with the original.\n");

    original->remove(testID);
    cout << "Test patient removed from the original. The copy is destroyed when this test\n"
         << "ends, and its destructor releases all of its memory.\n";
}

// ----- Assignment operator test
template<class TableType>
void MedicalHashApplication::assignmentOperatorTest()
{
    TableType* original = dynamic_cast<TableType*>(dictionary);
    if (original == nullptr) return;

    TableType target;   // a separate table with its own contents
    vector<PatientRecord> dummy = generator.generateUnique(2, currentIDs());
    for (const PatientRecord& record : dummy)
        target.insert(record.getPatientID(), record);
    cout << "Target table before assignment: " << target.getCount()
         << " records, table size " << target.getTableSize() << "\n";

    target = *original;   // ASSIGNMENT OPERATOR: frees target's memory, then deep copies
    cout << "After  target = original:      " << target.getCount()
         << " records, table size " << target.getTableSize() << "\n";

    TableType& alias = target;
    target = alias;       // SELF-ASSIGNMENT: must leave the table unchanged
    cout << "After  self-assignment:        " << target.getCount()
         << " records (unchanged, self-assignment guard works)\n";

    vector<PatientRecord> extra = generator.generateUnique(1, currentIDs());
    int testID = extra[0].getPatientID();
    original->insert(testID, extra[0]);
    PatientRecord found;
    bool inTarget = target.search(testID, found);
    cout << "Inserted test patient " << testID << " into the ORIGINAL only. Target contains it: "
         << (inTarget ? "yes" : "no") << "\n";

    cout << "\ncout << target:\n" << target;   // operator<<
    original->remove(testID);
    cout << (inTarget ? "\nWarning: assignment was shallow.\n"
                      : "\nASSIGNMENT CONFIRMED: deep copy, original and target are independent.\n");
}

void MedicalHashApplication::testCopyConstructor()
{
    switch (strategy)
    {
        case LINEAR:           copyConstructorTest<LinearProbingHash<int, PatientRecord>>(); break;
        case QUADRATIC:        copyConstructorTest<QuadraticProbingHash<int, PatientRecord>>(); break;
        case DOUBLE_HASH:      copyConstructorTest<DoubleHash<int, PatientRecord>>(); break;
        case CHAINING:         copyConstructorTest<SeparateChainingHash<int, PatientRecord>>(); break;
        case BUCKET:           copyConstructorTest<BucketHash<int, PatientRecord>>(); break;
        case ALTERNATE_BUCKET: copyConstructorTest<AlternateBucketHash<int, PatientRecord>>(); break;
    }
}

void MedicalHashApplication::testAssignmentOperator()
{
    switch (strategy)
    {
        case LINEAR:           assignmentOperatorTest<LinearProbingHash<int, PatientRecord>>(); break;
        case QUADRATIC:        assignmentOperatorTest<QuadraticProbingHash<int, PatientRecord>>(); break;
        case DOUBLE_HASH:      assignmentOperatorTest<DoubleHash<int, PatientRecord>>(); break;
        case CHAINING:         assignmentOperatorTest<SeparateChainingHash<int, PatientRecord>>(); break;
        case BUCKET:           assignmentOperatorTest<BucketHash<int, PatientRecord>>(); break;
        case ALTERNATE_BUCKET: assignmentOperatorTest<AlternateBucketHash<int, PatientRecord>>(); break;
    }
}

void MedicalHashApplication::clearDatabase()
{
    if (!readYesNo("Delete ALL records and reset statistics?"))
        return;
    dictionary->clear();
    cout << dictionary->getLastReport();
}

// ===================== Comparison =====================

void MedicalHashApplication::saveDataset(const vector<PatientRecord>& data, const string& path)
{
    std::ofstream out(path);
    if (!out)
    {
        cout << "(Could not write " << path << " - run the program from the project folder.)\n";
        return;
    }
    out << PatientRecord::csvHeader() << "\n";
    for (const PatientRecord& record : data)
        out << record.toCSV() << "\n";
    cout << "Synthetic dataset saved to " << path << " (" << data.size() << " records).\n";
}

void MedicalHashApplication::compareWithNewDataset()
{
    cout << "\n--- Compare All Techniques ---\n";
    int n = readIntDefault("Number of synthetic records", 1, 100000, 100);
    int size = readIntDefault("Initial table size / slots for every method", 2, 1000000,
                              hashutil::nextPrime(2 * n));
    int bucketSize = readIntDefault("Slots per bucket (bucket methods)", 1, 100, 3);
    bool autoRehash = readYesNo("Enable automatic rehashing for all methods?");

    vector<PatientRecord> data = generator.generateUnique(n, vector<int>());
    saveDataset(data, "data/synthetic_patients.csv");
    runComparison(data, size, bucketSize, autoRehash);
}

void MedicalHashApplication::compareFromCurrentData()
{
    if (dictionary->getCount() == 0)
    {
        cout << "The database is empty, so a new synthetic dataset will be generated.\n";
        compareWithNewDataset();
        return;
    }

    vector<PatientRecord> data;
    for (const auto& entry : dictionary->getAllEntries())
        data.push_back(entry.second);

    cout << "Comparing all methods using the " << data.size() << " records currently stored.\n";
    int size = readIntDefault("Initial table size / slots for every method", 2, 1000000,
                              dictionary->getTableSize());
    int bucketSize = readIntDefault("Slots per bucket (bucket methods)", 1, 100, 3);
    bool autoRehash = readYesNo("Enable automatic rehashing for all methods?");
    runComparison(data, size, bucketSize, autoRehash);
}

// [REQ §12 Compare All Techniques] [PERFORMANCE ANALYSIS]
// Inserts the SAME dataset into each implementation and reports measured values.
// Nothing is hard-coded: every number comes from the tables' own statistics.
void MedicalHashApplication::runComparison(const vector<PatientRecord>& data, int initialSize,
                                           int bucketSize, bool autoRehash)
{
    struct Result
    {
        string name;
        HashStatistics stats;
        double loadFactor;
        int finalSize;
        int failed;
        double insertMs;
    };

    vector<int> ids;
    for (const PatientRecord& record : data)
        ids.push_back(record.getPatientID());
    vector<int> absent = generator.generateAbsentIDs(static_cast<int>(data.size()), ids);

    vector<Result> results;
    for (int method = LINEAR; method <= ALTERNATE_BUCKET; ++method)
    {
        Dictionary* table = createDictionary(method, initialSize, bucketSize);
        table->setAutoRehash(autoRehash);

        int failed = 0;
        auto start = std::chrono::steady_clock::now();
        for (const PatientRecord& record : data)
        {
            if (!table->insert(record.getPatientID(), record))
                ++failed;
        }
        auto stop = std::chrono::steady_clock::now();

        // [PERFORMANCE ANALYSIS] successful vs unsuccessful search cost
        PatientRecord found;
        for (int id : ids) table->search(id, found);       // successful searches
        for (int id : absent) table->search(id, found);    // unsuccessful searches

        Result result;
        result.name = shortStrategyName(method);
        result.stats = table->getStatistics();
        result.loadFactor = table->getLoadFactor();
        result.finalSize = table->getTableSize();
        result.failed = failed;
        result.insertMs = std::chrono::duration<double, std::milli>(stop - start).count();
        results.push_back(result);

        delete table;
    }

    std::ios oldState(nullptr);
    oldState.copyfmt(cout);

    cout << "\nSame dataset: " << data.size() << " records | initial size " << initialSize
         << " | bucket size " << bucketSize << " | automatic rehash "
         << (autoRehash ? "ON" : "OFF") << "\n\n";

    cout << std::left << std::fixed << std::setprecision(3)
         << std::setw(20) << "Method" << std::setw(12) << "Collisions" << std::setw(11) << "Avg Probe"
         << std::setw(11) << "Max Probe" << std::setw(13) << "Load Factor" << "Rehashes\n"
         << string(75, '-') << "\n";
    for (const Result& r : results)
    {
        cout << std::setw(20) << r.name << std::setw(12) << r.stats.getCollisions()
             << std::setw(11) << r.stats.getAverageProbes() << std::setw(11) << r.stats.getMaxProbes()
             << std::setw(13) << r.loadFactor << r.stats.getRehashCount() << "\n";
    }

    cout << "\n" << std::setw(20) << "Method" << std::setw(11) << "Final Size"
         << std::setw(11) << "Avg Succ" << std::setw(13) << "Avg Unsucc" << std::setw(10) << "Longest"
         << std::setw(10) << "Overflow" << std::setw(8) << "Failed" << "Insert ms\n"
         << std::setw(20) << "" << std::setw(11) << "" << std::setw(11) << "compares"
         << std::setw(13) << "compares" << std::setw(10) << "chain" << "\n"
         << string(93, '-') << "\n";
    for (const Result& r : results)
    {
        cout << std::setw(20) << r.name << std::setw(11) << r.finalSize
             << std::setw(11) << r.stats.getAverageSuccessfulComparisons()
             << std::setw(13) << r.stats.getAverageUnsuccessfulComparisons()
             << std::setw(10) << (r.name == "Separate Chaining" ? std::to_string(r.stats.getLongestChain()) : "-")
             << std::setw(10) << r.stats.getCurrentOverflow()
             << std::setw(8) << r.failed << r.insertMs << "\n";
    }

    // Best performers (from measured values only)
    size_t bestProbe = 0, bestUnsucc = 0;
    for (size_t i = 1; i < results.size(); ++i)
    {
        if (results[i].stats.getAverageProbes() < results[bestProbe].stats.getAverageProbes()) bestProbe = i;
        if (results[i].stats.getAverageUnsuccessfulComparisons()
            < results[bestUnsucc].stats.getAverageUnsuccessfulComparisons()) bestUnsucc = i;
    }
    cout << "\nLowest average probes per insertion: " << results[bestProbe].name << "\n"
         << "Lowest average unsuccessful-search comparisons: " << results[bestUnsucc].name << "\n";

    cout.copyfmt(oldState);

    // [PERFORMANCE ANALYSIS] append measured results to a CSV file for the poster table and chart
    const string path = "data/results.csv";
    bool newFile = !std::ifstream(path).good();
    std::ofstream csv(path, std::ios::app);
    if (csv)
    {
        if (newFile)
            csv << "records,initial_size,bucket_size,auto_rehash,method,final_size,load_factor,collisions,"
                   "avg_probes,max_probes,avg_success_comparisons,avg_unsuccess_comparisons,"
                   "longest_chain,overflow_records,rehashes,failed,insert_ms\n";
        for (const Result& r : results)
        {
            csv << data.size() << ',' << initialSize << ',' << bucketSize << ',' << (autoRehash ? 1 : 0) << ','
                << r.name << ',' << r.finalSize << ',' << r.loadFactor << ',' << r.stats.getCollisions() << ','
                << r.stats.getAverageProbes() << ',' << r.stats.getMaxProbes() << ','
                << r.stats.getAverageSuccessfulComparisons() << ','
                << r.stats.getAverageUnsuccessfulComparisons() << ','
                << r.stats.getLongestChain() << ',' << r.stats.getCurrentOverflow() << ','
                << r.stats.getRehashCount() << ',' << r.failed << ',' << r.insertMs << "\n";
        }
        cout << "Results appended to " << path << "\n";
    }
    else
    {
        cout << "(Could not write " << path << " - run the program from the project folder.)\n";
    }
}
