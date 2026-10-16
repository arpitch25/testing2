#ifndef MARKSHEET_VERIFIER_HPP
#define MARKSHEET_VERIFIER_HPP

#include <vector>
#include <string>
#include <unordered_map>
#include "Student.hpp"
#include "MerkleTree.hpp"

class MarksheetVerifier {
private:
    std::vector<StudentRecord> m_students;
    std::unordered_map<std::string, size_t> m_rollToIndex;
    MerkleTree m_merkleTree;
    std::string m_publishedRootHash;

public:
    MarksheetVerifier();

    // Add a single student record
    bool addStudent(const StudentRecord& record);

    // Clear all student records
    void clear();

    // Load default sample dataset for immediate testing
    void loadSampleDataset();

    // Load dataset from CSV file
    bool loadFromCSV(const std::string& filename);

    // Save dataset to CSV file
    bool saveToCSV(const std::string& filename) const;

    // Build Merkle tree from current records & publish root hash
    void buildAndPublishTree();

    // Find student by roll number
    bool getStudentByRoll(const std::string& rollNumber, StudentRecord& outRecord, size_t& outIndex) const;

    // Generate Merkle inclusion proof for a student by roll number
    bool generateProofForStudent(const std::string& rollNumber,
                                StudentRecord& outRecord,
                                std::vector<MerkleProofNode>& outProof) const;

    // Verify a student record against audit proof and published root hash
    static bool verifyRecord(const StudentRecord& record,
                             const std::vector<MerkleProofNode>& proof,
                             const std::string& publishedRoot);

    // Run complete interactive / automated tamper detection demonstration
    void runTamperDemo();

    // Accessors
    std::string getPublishedRootHash() const { return m_publishedRootHash; }
    const MerkleTree& getMerkleTree() const { return m_merkleTree; }
    const std::vector<StudentRecord>& getStudents() const { return m_students; }
    size_t getStudentCount() const { return m_students.size(); }
};

#endif // MARKSHEET_VERIFIER_HPP
