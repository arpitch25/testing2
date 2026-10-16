#include "MarksheetVerifier.hpp"
#include <fstream>
#include <iostream>
#include <iomanip>

MarksheetVerifier::MarksheetVerifier() {
    m_publishedRootHash = "";
}

void MarksheetVerifier::clear() {
    m_students.clear();
    m_rollToIndex.clear();
    m_publishedRootHash = "";
    m_merkleTree.buildTree({});
}

bool MarksheetVerifier::addStudent(const StudentRecord& record) {
    if (m_rollToIndex.find(record.rollNumber) != m_rollToIndex.end()) {
        return false; // Duplicate roll number
    }
    m_rollToIndex[record.rollNumber] = m_students.size();
    m_students.push_back(record);
    m_students.back().calculateTotal();
    return true;
}

void MarksheetVerifier::loadSampleDataset() {
    clear();

    StudentRecord s1;
    s1.rollNumber = "2510371956";
    s1.name = "Diksha Rawat";
    s1.course = "B.Tech CSE";
    s1.subjectMarks = {{"Data Structures", 94.0}, {"C++ Programming", 96.5}, {"Mathematics", 89.0}};
    addStudent(s1);

    StudentRecord s2;
    s2.rollNumber = "2510380033";
    s2.name = "Aakhya Gupta";
    s2.course = "B.Tech CSE";
    s2.subjectMarks = {{"Data Structures", 91.0}, {"C++ Programming", 93.0}, {"Mathematics", 95.5}};
    addStudent(s2);

    StudentRecord s3;
    s3.rollNumber = "2510370275";
    s3.name = "Divya Singh";
    s3.course = "B.Tech CSE";
    s3.subjectMarks = {{"Data Structures", 88.5}, {"C++ Programming", 90.0}, {"Mathematics", 92.0}};
    addStudent(s3);

    StudentRecord s4;
    s4.rollNumber = "2510370298";
    s4.name = "Jeevika Srivastava";
    s4.course = "B.Tech CSE";
    s4.subjectMarks = {{"Data Structures", 95.0}, {"C++ Programming", 97.0}, {"Mathematics", 94.0}};
    addStudent(s4);

    StudentRecord s5;
    s5.rollNumber = "2510370999";
    s5.name = "Aarav Sharma";
    s5.course = "B.Tech IT";
    s5.subjectMarks = {{"Data Structures", 82.0}, {"C++ Programming", 85.0}, {"Mathematics", 80.0}};
    addStudent(s5);

    buildAndPublishTree();
}

void MarksheetVerifier::buildAndPublishTree() {
    std::vector<std::string> leafHashes;
    for (const auto& student : m_students) {
        leafHashes.push_back(student.getLeafHash());
    }

    m_merkleTree.buildTree(leafHashes);
    m_publishedRootHash = m_merkleTree.getRootHash();
}

bool MarksheetVerifier::getStudentByRoll(const std::string& rollNumber, StudentRecord& outRecord, size_t& outIndex) const {
    auto it = m_rollToIndex.find(rollNumber);
    if (it == m_rollToIndex.end()) {
        return false;
    }
    outIndex = it->second;
    outRecord = m_students[outIndex];
    return true;
}

bool MarksheetVerifier::generateProofForStudent(const std::string& rollNumber,
                                                 StudentRecord& outRecord,
                                                 std::vector<MerkleProofNode>& outProof) const {
    size_t index = 0;
    if (!getStudentByRoll(rollNumber, outRecord, index)) {
        return false;
    }
    outProof = m_merkleTree.getProof(index);
    return true;
}

bool MarksheetVerifier::verifyRecord(const StudentRecord& record,
                                      const std::vector<MerkleProofNode>& proof,
                                      const std::string& publishedRoot) {
    std::string leafHash = record.getLeafHash();
    return MerkleTree::verifyProof(leafHash, proof, publishedRoot);
}

bool MarksheetVerifier::loadFromCSV(const std::string& filename) {
    std::ifstream inFile(filename);
    if (!inFile.is_open()) {
        return false;
    }

    clear();
    std::string line;
    // Check for optional header
    if (std::getline(inFile, line)) {
        if (line.find("roll") == std::string::npos && line.find("Roll") == std::string::npos) {
            // Not a header line, process as student
            if (!line.empty()) {
                addStudent(StudentRecord::fromCSVLine(line));
            }
        }
    }

    while (std::getline(inFile, line)) {
        if (line.empty()) continue;
        addStudent(StudentRecord::fromCSVLine(line));
    }

    inFile.close();
    buildAndPublishTree();
    return true;
}

bool MarksheetVerifier::saveToCSV(const std::string& filename) const {
    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        return false;
    }

    outFile << "RollNumber,Name,Course,SubjectMarks\n";
    for (const auto& s : m_students) {
        outFile << s.toCSVLine() << "\n";
    }

    outFile.close();
    return true;
}

void MarksheetVerifier::runTamperDemo() {
    std::cout << "\n=======================================================\n";
    std::cout << "        TAMPER DETECTION DEMONSTRATION & AUDIT         \n";
    std::cout << "=======================================================\n\n";

    if (m_students.empty()) {
        loadSampleDataset();
    } else {
        buildAndPublishTree();
    }

    std::cout << "[Step 1] Teacher builds Merkle Tree for " << m_students.size() << " student records.\n";
    std::cout << "  -> Published Merkle Root Hash: " << m_publishedRootHash << "\n\n";

    // Select target student (e.g. Diksha Rawat - Roll 2510371956)
    std::string targetRoll = m_students[0].rollNumber;
    StudentRecord genuineRecord;
    std::vector<MerkleProofNode> proof;
    generateProofForStudent(targetRoll, genuineRecord, proof);

    std::cout << "[Step 2] Student (Roll " << targetRoll << " - " << genuineRecord.name << ") retrieves their record & proof.\n";
    std::cout << "  -> Original Record Data: " << genuineRecord.toHashString() << "\n";
    std::cout << "  -> Leaf Hash: " << genuineRecord.getLeafHash() << "\n";
    std::cout << "  -> Proof Path Size: " << proof.size() << " sibling hash nodes.\n\n";

    std::cout << "[Step 3] Run Verification on Original Genuine Record...\n";
    bool genuineResult = verifyRecord(genuineRecord, proof, m_publishedRootHash);
    std::cout << "  -> Calculated Leaf Hash: " << genuineRecord.getLeafHash() << "\n";
    std::cout << "  -> Verification Result  : " << (genuineResult ? "✅ [VERIFIED] Genuine Record Unaltered" : "❌ [TAMPERED] Verification Failed") << "\n\n";

    std::cout << "[Step 4] Simulating Tampering! (Altering Data Structures mark from 94.0 to 99.0)\n";
    StudentRecord tamperedRecord = genuineRecord;
    tamperedRecord.subjectMarks["Data Structures"] = 99.0;
    tamperedRecord.calculateTotal();

    std::cout << "  -> Tampered Record Data: " << tamperedRecord.toHashString() << "\n";
    std::cout << "  -> Tampered Leaf Hash  : " << tamperedRecord.getLeafHash() << "\n";
    std::cout << "  -> Original Leaf Hash  : " << genuineRecord.getLeafHash() << "\n\n";

    std::cout << "[Step 5] Run Verification on Tampered Record against Published Root Hash...\n";
    bool tamperedResult = verifyRecord(tamperedRecord, proof, m_publishedRootHash);
    std::cout << "  -> Verification Result  : " << (tamperedResult ? "✅ [VERIFIED]" : "❌ [TAMPERED] Verification Failed! Cryptographic proof mismatch.") << "\n\n";

    std::cout << "=======================================================\n";
    std::cout << "                 DEMONSTRATION SUMMARY                 \n";
    std::cout << " Genuine Record Verification  : " << (genuineResult ? "SUCCESS (VERIFIED)" : "FAIL") << "\n";
    std::cout << " Tampered Record Verification : " << (!tamperedResult ? "SUCCESS (TAMPER DETECTED)" : "FAIL") << "\n";
    std::cout << "=======================================================\n\n";
}
