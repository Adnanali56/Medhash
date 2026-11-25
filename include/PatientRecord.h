// PatientRecord.h
// The value stored in the medical hash tables. patientID is the search key;
// all other fields form the value associated with that key.
// All data used with this class must be synthetic (no real patients).

#ifndef PATIENTRECORD_H
#define PATIENTRECORD_H

#include <iostream>
#include <string>

class PatientRecord
{
public:
    PatientRecord();
    PatientRecord(int id,
                  const std::string& first,
                  const std::string& last,
                  int patientAge,
                  const std::string& blood,
                  const std::string& diagnosis,
                  int priority);

    // Getters (const: they never modify the record)
    int getPatientID() const;
    std::string getFirstName() const;
    std::string getLastName() const;
    std::string getFullName() const;
    int getAge() const;
    std::string getBloodType() const;
    std::string getDiagnosisCode() const;
    int getPriorityLevel() const;

    // Setters (large inputs passed by const reference)
    void setPatientID(int id);
    void setFirstName(const std::string& first);
    void setLastName(const std::string& last);
    void setAge(int patientAge);
    void setBloodType(const std::string& blood);
    void setDiagnosisCode(const std::string& diagnosis);
    void setPriorityLevel(int priority);

    // Full multi-line display of every field
    void display(std::ostream& os = std::cout) const;

    // One CSV line (used to save the synthetic dataset)
    std::string toCSV() const;
    static std::string csvHeader();

    // Validation helpers used by the menu
    static bool isValidBloodType(const std::string& blood);

private:
    int patientID;
    std::string firstName;
    std::string lastName;
    int age;
    std::string bloodType;
    std::string diagnosisCode;
    int priorityLevel;
};

// Short form used inside hash-table displays: "First Last"
std::ostream& operator<<(std::ostream& os, const PatientRecord& record);

#endif
