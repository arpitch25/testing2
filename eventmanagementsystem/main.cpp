#include <iostream>
#include <string>
#include <vector>
#include <iomanip>
#include <limits>
#include "MarksheetVerifier.hpp"

void printHeader() {
    std::cout << "\n=======================================================================\n";
    std::cout << "        SECURE MARKSHEET VERIFIER (MERKLE TREE CRYPTOGRAPHY)           \n";
    std::cout << "        Tamper-Proof Academic Record Verification in C++                \n";
    std::cout << "=======================================================================\n";
}

void teacherMenu(MarksheetVerifier& verifier) {
    int choice = 0;
    while (true) {
        std::cout << "\n--- [TEACHER MODE / ADMIN MENU] ---\n";
        std::cout << "1. Load Sample Dataset (Default 5 Students)\n";
        std::cout << "2. Load Records from CSV File\n";
        std::cout << "3. Add a New Student Record Manually\n";
        std::cout << "4. View All Registered Student Records\n";
        std::cout << "5. Build Merkle Tree & Generate Root Hash\n";
        std::cout << "6. View Merkle Tree Structure & Published Root Hash\n";
        std::cout << "7. Export Current Records to CSV File\n";
        std::cout << "8. Return to Main Menu\n";
        std::cout << "Select Option [1-8]: ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number between 1 and 8.\n";
            continue;
        }

        if (choice == 8) break;

        switch (choice) {
            case 1: {
                verifier.loadSampleDataset();
                std::cout << "\n✅ Sample dataset loaded successfully with " 
                          << verifier.getStudentCount() << " records.\n";
                std::cout << "🔑 Merkle Root Hash Published: " << verifier.getPublishedRootHash() << "\n";
                break;
            }
            case 2: {
                std::cout << "Enter CSV filename (e.g. sample_marksheet.csv): ";
                std::string filename;
                std::cin >> filename;
                if (verifier.loadFromCSV(filename)) {
                    std::cout << "\n✅ Loaded " << verifier.getStudentCount() << " records from '" << filename << "'.\n";
                    std::cout << "🔑 Published Root Hash: " << verifier.getPublishedRootHash() << "\n";
                } else {
                    std::cout << "\n❌ Failed to open or parse file '" << filename << "'.\n";
                }
                break;
            }
            case 3: {
                StudentRecord record;
                std::cout << "Enter Roll Number: ";
                std::cin >> record.rollNumber;
                std::cin.ignore();
                std::cout << "Enter Full Name  : ";
                std::getline(std::cin, record.name);
                std::cout << "Enter Course     : ";
                std::getline(std::cin, record.course);

                int numSubjects = 0;
                std::cout << "Enter number of subjects: ";
                std::cin >> numSubjects;
                for (int i = 0; i < numSubjects; ++i) {
                    std::string subject;
                    double mark;
                    std::cout << "  Subject " << (i + 1) << " Name: ";
                    std::cin.ignore();
                    std::getline(std::cin, subject);
                    std::cout << "  Marks for " << subject << ": ";
                    std::cin >> mark;
                    record.subjectMarks[subject] = mark;
                }

                if (verifier.addStudent(record)) {
                    std::cout << "\n✅ Student record added for Roll Number: " << record.rollNumber << "\n";
                    std::cout << "Rebuilding Merkle Tree...\n";
                    verifier.buildAndPublishTree();
                    std::cout << "🔑 New Published Merkle Root Hash: " << verifier.getPublishedRootHash() << "\n";
                } else {
                    std::cout << "\n❌ Error: Duplicate roll number '" << record.rollNumber << "' already exists!\n";
                }
                break;
            }
            case 4: {
                const auto& students = verifier.getStudents();
                if (students.empty()) {
                    std::cout << "\nNo records registered yet. Please load or add student records.\n";
                } else {
                    std::cout << "\n--------------------------------------------------------------------------------------------------------\n";
                    std::cout << std::left << std::setw(14) << "Roll No" 
                              << std::setw(22) << "Name" 
                              << std::setw(15) << "Course" 
                              << std::setw(12) << "Total Marks" 
                              << "Leaf Hash (SHA-256)\n";
                    std::cout << "--------------------------------------------------------------------------------------------------------\n";
                    for (const auto& s : students) {
                        std::cout << std::left << std::setw(14) << s.rollNumber 
                                  << std::setw(22) << s.name 
                                  << std::setw(15) << s.course 
                                  << std::setw(12) << std::fixed << std::setprecision(2) << s.totalMarks 
                                  << s.getLeafHash().substr(0, 16) << "..." << "\n";
                    }
                    std::cout << "--------------------------------------------------------------------------------------------------------\n";
                }
                break;
            }
            case 5: {
                if (verifier.getStudentCount() == 0) {
                    std::cout << "\nNo student records available to build tree.\n";
                } else {
                    verifier.buildAndPublishTree();
                    std::cout << "\n✅ Merkle Tree successfully built with " << verifier.getStudentCount() << " leaf nodes.\n";
                    std::cout << "🔑 Published Root Hash: " << verifier.getPublishedRootHash() << "\n";
                }
                break;
            }
            case 6: {
                if (verifier.getStudentCount() == 0) {
                    std::cout << "\nNo student records available.\n";
                } else {
                    std::cout << "\n🔑 Published Merkle Root Hash: " << verifier.getPublishedRootHash() << "\n";
                    verifier.getMerkleTree().printTree();
                }
                break;
            }
            case 7: {
                std::cout << "Enter output CSV filename (e.g. export_marksheet.csv): ";
                std::string filename;
                std::cin >> filename;
                if (verifier.saveToCSV(filename)) {
                    std::cout << "\n✅ Saved records to '" << filename << "' successfully.\n";
                } else {
                    std::cout << "\n❌ Failed to write to file '" << filename << "'.\n";
                }
                break;
            }
            default:
                std::cout << "Invalid option selected.\n";
        }
    }
}

void studentMenu(const MarksheetVerifier& verifier) {
    std::cout << "\n--- [STUDENT MODE / AUDIT CERTIFICATE] ---\n";
    if (verifier.getStudentCount() == 0) {
        std::cout << "No student records available in the system yet. Please ask Teacher/Admin to load records first.\n";
        return;
    }

    std::cout << "Enter your Roll Number: ";
    std::string roll;
    std::cin >> roll;

    StudentRecord student;
    std::vector<MerkleProofNode> proof;

    if (!verifier.generateProofForStudent(roll, student, proof)) {
        std::cout << "\n❌ Record not found for Roll Number: " << roll << "\n";
        return;
    }

    std::cout << "\n=======================================================================\n";
    std::cout << "             ACADEMIC MARKSHEET & MERKLE PROOF CERTIFICATE             \n";
    std::cout << "=======================================================================\n";
    std::cout << " Roll Number   : " << student.rollNumber << "\n";
    std::cout << " Student Name  : " << student.name << "\n";
    std::cout << " Course        : " << student.course << "\n";
    std::cout << "-----------------------------------------------------------------------\n";
    std::cout << " SUBJECT MARKS BREAKDOWN:\n";
    for (const auto& item : student.subjectMarks) {
        std::cout << "   - " << std::left << std::setw(25) << item.first << ": " << std::fixed << std::setprecision(2) << item.second << "\n";
    }
    std::cout << " TOTAL MARKS   : " << std::fixed << std::setprecision(2) << student.totalMarks << "\n";
    std::cout << "-----------------------------------------------------------------------\n";
    std::cout << " CRYPTOGRAPHIC AUDIT PROOF:\n";
    std::cout << "   - Data String    : " << student.toHashString() << "\n";
    std::cout << "   - Leaf Hash      : " << student.getLeafHash() << "\n";
    std::cout << "   - Published Root : " << verifier.getPublishedRootHash() << "\n";
    std::cout << "   - Merkle Proof Path (" << proof.size() << " steps):\n";
    for (size_t i = 0; i < proof.size(); ++i) {
        std::cout << "       Step " << (i + 1) << ": [" << (proof[i].isLeftSibling ? "LEFT " : "RIGHT") << "] " 
                  << proof[i].siblingHash << "\n";
    }
    std::cout << "=======================================================================\n\n";
}

void verifyMenu(const MarksheetVerifier& verifier) {
    std::cout << "\n--- [VERIFICATION MODULE] ---\n";
    std::cout << "1. Verify Record directly using Roll Number in System\n";
    std::cout << "2. Verify Custom/External Record against Published Root Hash\n";
    std::cout << "Select Option [1-2]: ";

    int choice = 0;
    if (!(std::cin >> choice)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        return;
    }

    if (choice == 1) {
        if (verifier.getStudentCount() == 0) {
            std::cout << "No records available. Load records in Teacher Mode first.\n";
            return;
        }
        std::cout << "Enter Roll Number to verify: ";
        std::string roll;
        std::cin >> roll;

        StudentRecord record;
        std::vector<MerkleProofNode> proof;
        if (!verifier.generateProofForStudent(roll, record, proof)) {
            std::cout << "❌ Roll number not found.\n";
            return;
        }

        std::string root = verifier.getPublishedRootHash();
        bool ok = MarksheetVerifier::verifyRecord(record, proof, root);

        std::cout << "\n-------------------------------------------------------\n";
        std::cout << " VERIFICATION RESULT FOR ROLL: " << roll << "\n";
        std::cout << " Calculated Leaf Hash: " << record.getLeafHash() << "\n";
        std::cout << " Published Root Hash : " << root << "\n";
        std::cout << " Status              : " << (ok ? "✅ [VERIFIED] Genuine Record" : "❌ [TAMPERED] Fraudulent Record") << "\n";
        std::cout << "-------------------------------------------------------\n";
    } else if (choice == 2) {
        std::cout << "Enter Published Root Hash: ";
        std::string pubRoot;
        std::cin >> pubRoot;

        StudentRecord record;
        std::cout << "Enter Roll Number: ";
        std::cin >> record.rollNumber;
        std::cin.ignore();
        std::cout << "Enter Full Name: ";
        std::getline(std::cin, record.name);
        std::cout << "Enter Course: ";
        std::getline(std::cin, record.course);

        int numSubj = 0;
        std::cout << "Enter number of subjects: ";
        std::cin >> numSubj;
        for (int i = 0; i < numSubj; ++i) {
            std::string sName;
            double sScore;
            std::cout << " Subject " << (i + 1) << " Name: ";
            std::cin.ignore();
            std::getline(std::cin, sName);
            std::cout << " Subject Marks: ";
            std::cin >> sScore;
            record.subjectMarks[sName] = sScore;
        }
        record.calculateTotal();

        int proofLen = 0;
        std::cout << "Enter number of proof sibling hashes: ";
        std::cin >> proofLen;
        std::vector<MerkleProofNode> proof;
        for (int i = 0; i < proofLen; ++i) {
            std::string sibHash;
            int isLeft;
            std::cout << " Sibling Hash " << (i + 1) << ": ";
            std::cin >> sibHash;
            std::cout << " Is Sibling on Left? (1 for Yes, 0 for No): ";
            std::cin >> isLeft;
            proof.push_back({sibHash, isLeft == 1});
        }

        bool ok = MarksheetVerifier::verifyRecord(record, proof, pubRoot);
        std::cout << "\n-------------------------------------------------------\n";
        std::cout << " VERIFICATION RESULT:\n";
        std::cout << " Status: " << (ok ? "✅ [VERIFIED] Marksheet is Genuine!" : "❌ [TAMPERED] Data or Proof Mismatch!") << "\n";
        std::cout << "-------------------------------------------------------\n";
    }
}

int main() {
    MarksheetVerifier verifier;
    // Pre-load default sample dataset so system is immediately usable
    verifier.loadSampleDataset();

    printHeader();

    int choice = 0;
    while (true) {
        std::cout << "\n=======================================================\n";
        std::cout << "                    MAIN MENU                          \n";
        std::cout << "=======================================================\n";
        std::cout << "1. Teacher / Admin Mode (Manage Records & Merkle Tree)\n";
        std::cout << "2. Student Mode (View Record & Merkle Proof)\n";
        std::cout << "3. Verification Module (Verify Authenticity)\n";
        std::cout << "4. Run Tamper Detection Demonstration (Automated Audit)\n";
        std::cout << "5. Exit\n";
        std::cout << "Select Option [1-5]: ";

        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Invalid input. Please enter a number between 1 and 5.\n";
            continue;
        }

        if (choice == 5) {
            std::cout << "\nExiting Secure Marksheet Verifier. Thank you!\n";
            break;
        }

        switch (choice) {
            case 1:
                teacherMenu(verifier);
                break;
            case 2:
                studentMenu(verifier);
                break;
            case 3:
                verifyMenu(verifier);
                break;
            case 4:
                verifier.runTamperDemo();
                break;
            default:
                std::cout << "Invalid choice. Please select from 1 to 5.\n";
        }
    }

    return 0;
}
