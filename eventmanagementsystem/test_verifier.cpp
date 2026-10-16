#include <iostream>
#include <cassert>
#include "MarksheetVerifier.hpp"

int main() {
    std::cout << "[TEST] Starting Merkle Tree Marksheet Verifier Unit Tests...\n";

    MarksheetVerifier verifier;
    verifier.loadSampleDataset();

    assert(verifier.getStudentCount() == 5);
    std::cout << "[TEST 1] Sample dataset loaded 5 records. PASS\n";

    std::string root = verifier.getPublishedRootHash();
    assert(!root.empty());
    assert(root.length() == 64); // Valid 256-bit hex string
    std::cout << "[TEST 2] Root Hash generated successfully: " << root << ". PASS\n";

    // Test 3: Genuine record verification
    StudentRecord student;
    std::vector<MerkleProofNode> proof;
    bool found = verifier.generateProofForStudent("2510371956", student, proof);
    assert(found);
    assert(student.name == "Diksha Rawat");
    
    bool verifiedGenuine = MarksheetVerifier::verifyRecord(student, proof, root);
    assert(verifiedGenuine == true);
    std::cout << "[TEST 3] Genuine record verification PASSED (Result = VERIFIED).\n";

    // Test 4: Tampered mark detection
    StudentRecord tamperedStudent = student;
    tamperedStudent.subjectMarks["Data Structures"] = 99.0;
    tamperedStudent.calculateTotal();

    bool verifiedTampered = MarksheetVerifier::verifyRecord(tamperedStudent, proof, root);
    assert(verifiedTampered == false);
    std::cout << "[TEST 4] Tampered mark record detection PASSED (Result = TAMPERED / Rejected).\n";

    // Test 5: Odd number of nodes test (3 records)
    verifier.clear();
    StudentRecord s1; s1.rollNumber = "101"; s1.name = "Alice"; s1.subjectMarks["Math"] = 90; s1.calculateTotal();
    StudentRecord s2; s2.rollNumber = "102"; s2.name = "Bob"; s2.subjectMarks["Math"] = 85; s2.calculateTotal();
    StudentRecord s3; s3.rollNumber = "103"; s3.name = "Charlie"; s3.subjectMarks["Math"] = 88; s3.calculateTotal();
    verifier.addStudent(s1);
    verifier.addStudent(s2);
    verifier.addStudent(s3);
    verifier.buildAndPublishTree();

    std::string oddRoot = verifier.getPublishedRootHash();
    std::vector<MerkleProofNode> oddProof;
    StudentRecord oddStudent;
    verifier.generateProofForStudent("103", oddStudent, oddProof);
    bool oddVerified = MarksheetVerifier::verifyRecord(oddStudent, oddProof, oddRoot);
    assert(oddVerified == true);
    std::cout << "[TEST 5] Odd leaf count duplication handling PASSED.\n";

    std::cout << "\n🎉 ALL 5 UNIT & INTEGRATION TESTS PASSED SUCCESSFULLY!\n";
    return 0;
}
