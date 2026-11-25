// MedicalHashApplication.h
// The menu-driven program. It owns one DictionaryADT<int, PatientRecord>*
// and works through that abstract interface, so switching between hashing
// strategies does not change any menu code.

#ifndef MEDICALHASHAPPLICATION_H
#define MEDICALHASHAPPLICATION_H

#include "DictionaryADT.h"
#include "PatientRecord.h"
#include "RandomPatientGenerator.h"
#include <string>
#include <vector>

class MedicalHashApplication
{
public:
    MedicalHashApplication();
    ~MedicalHashApplication();

    // The application owns a dynamic dictionary; copying the application is not needed.
    MedicalHashApplication(const MedicalHashApplication&) = delete;
    MedicalHashApplication& operator=(const MedicalHashApplication&) = delete;

    void run();

private:
    using Dictionary = DictionaryADT<int, PatientRecord>;

    // Strategy numbers match the strategy menu (1-6)
    enum Strategy { LINEAR = 1, QUADRATIC, DOUBLE_HASH, CHAINING, BUCKET, ALTERNATE_BUCKET };

    Dictionary* dictionary;
    int strategy;
    RandomPatientGenerator generator;

    // Menus
    int showStrategyMenu() const;
    bool chooseStrategy(int choice);
    void applicationMenu();

    // Application menu operations (1-14)
    void addPatient();
    void searchPatient();
    void updatePatient();
    void deletePatient();
    void displayTable() const;
    void displayCount() const;
    void displayStatistics() const;
    void generateRandomRecords();
    void changeStrategy();
    void rehashMenu();
    void testCopyConstructor();
    void testAssignmentOperator();
    void clearDatabase();
    void compareFromCurrentData();
    void compareWithNewDataset();

    // Helpers
    static Dictionary* createDictionary(int strategyNumber, int tableSize, int bucketSize);
    static std::string strategyName(int strategyNumber);
    static std::string shortStrategyName(int strategyNumber);
    std::vector<int> currentIDs() const;
    PatientRecord promptPatientFields(int id) const;
    void runComparison(const std::vector<PatientRecord>& data, int initialSize,
                       int bucketSize, bool autoRehash);
    static void saveDataset(const std::vector<PatientRecord>& data, const std::string& path);

    template<class TableType> void copyConstructorTest();
    template<class TableType> void assignmentOperatorTest();
};

#endif
