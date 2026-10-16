#ifndef STUDENT_HPP
#define STUDENT_HPP

#include <string>
#include <map>
#include <sstream>
#include <iomanip>
#include <vector>
#include "SHA256.hpp"

struct StudentRecord {
    std::string rollNumber;
    std::string name;
    std::string course;
    std::map<std::string, double> subjectMarks;
    double totalMarks = 0.0;

    void calculateTotal() {
        totalMarks = 0.0;
        for (const auto& pair : subjectMarks) {
            totalMarks += pair.second;
        }
    }

    // Canonical string representation used as input to SHA-256 for leaf node hash
    std::string toHashString() const {
        std::ostringstream ss;
        ss << rollNumber << "|" << name << "|" << course << "|";
        bool first = true;
        for (const auto& item : subjectMarks) {
            if (!first) ss << ";";
            ss << item.first << ":" << std::fixed << std::setprecision(2) << item.second;
            first = false;
        }
        ss << "|" << std::fixed << std::setprecision(2) << totalMarks;
        return ss.str();
    }

    std::string getLeafHash() const {
        return SHA256::hash(toHashString());
    }

    std::string toCSVLine() const {
        std::ostringstream ss;
        ss << rollNumber << "," << name << "," << course << ",";
        bool first = true;
        for (const auto& item : subjectMarks) {
            if (!first) ss << ";";
            ss << item.first << ":" << std::fixed << std::setprecision(2) << item.second;
            first = false;
        }
        return ss.str();
    }

    static StudentRecord fromCSVLine(const std::string& line) {
        StudentRecord record;
        std::stringstream ss(line);
        std::string marksStr;

        std::getline(ss, record.rollNumber, ',');
        std::getline(ss, record.name, ',');
        std::getline(ss, record.course, ',');
        std::getline(ss, marksStr);

        std::stringstream marksSS(marksStr);
        std::string subjectToken;
        while (std::getline(marksSS, subjectToken, ';')) {
            size_t colonPos = subjectToken.find(':');
            if (colonPos != std::string::npos) {
                std::string subj = subjectToken.substr(0, colonPos);
                double score = std::stod(subjectToken.substr(colonPos + 1));
                record.subjectMarks[subj] = score;
            }
        }
        record.calculateTotal();
        return record;
    }
};

#endif // STUDENT_HPP
