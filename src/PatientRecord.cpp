// PatientRecord.cpp

#include "PatientRecord.h"
#include <sstream>

PatientRecord::PatientRecord()
    : patientID(0), firstName(""), lastName(""), age(0),
      bloodType(""), diagnosisCode(""), priorityLevel(0)
{
}

PatientRecord::PatientRecord(int id,
                             const std::string& first,
                             const std::string& last,
                             int patientAge,
                             const std::string& blood,
                             const std::string& diagnosis,
                             int priority)
    : patientID(id), firstName(first), lastName(last), age(patientAge),
      bloodType(blood), diagnosisCode(diagnosis), priorityLevel(priority)
{
}

int PatientRecord::getPatientID() const { return patientID; }
std::string PatientRecord::getFirstName() const { return firstName; }
std::string PatientRecord::getLastName() const { return lastName; }
std::string PatientRecord::getFullName() const { return firstName + " " + lastName; }
int PatientRecord::getAge() const { return age; }
std::string PatientRecord::getBloodType() const { return bloodType; }
std::string PatientRecord::getDiagnosisCode() const { return diagnosisCode; }
int PatientRecord::getPriorityLevel() const { return priorityLevel; }

void PatientRecord::setPatientID(int id) { patientID = id; }
void PatientRecord::setFirstName(const std::string& first) { firstName = first; }
void PatientRecord::setLastName(const std::string& last) { lastName = last; }
void PatientRecord::setAge(int patientAge) { age = patientAge; }
void PatientRecord::setBloodType(const std::string& blood) { bloodType = blood; }
void PatientRecord::setDiagnosisCode(const std::string& diagnosis) { diagnosisCode = diagnosis; }
void PatientRecord::setPriorityLevel(int priority) { priorityLevel = priority; }

void PatientRecord::display(std::ostream& os) const
{
    os << "Patient ID:     " << patientID << "\n"
       << "Name:           " << getFullName() << "\n"
       << "Age:            " << age << "\n"
       << "Blood Type:     " << bloodType << "\n"
       << "Diagnosis Code: " << diagnosisCode << "\n"
       << "Priority Level: " << priorityLevel << "\n";
}

std::string PatientRecord::toCSV() const
{
    std::ostringstream out;
    out << patientID << ',' << firstName << ',' << lastName << ',' << age << ','
        << bloodType << ',' << diagnosisCode << ',' << priorityLevel;
    return out.str();
}

std::string PatientRecord::csvHeader()
{
    return "patientID,firstName,lastName,age,bloodType,diagnosisCode,priorityLevel";
}

bool PatientRecord::isValidBloodType(const std::string& blood)
{
    static const std::string types[] = { "A+", "A-", "B+", "B-", "AB+", "AB-", "O+", "O-" };
    for (const std::string& t : types)
    {
        if (t == blood)
            return true;
    }
    return false;
}

std::ostream& operator<<(std::ostream& os, const PatientRecord& record)
{
    os << record.getFullName();
    return os;
}
