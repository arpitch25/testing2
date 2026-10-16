#ifndef MERKLE_TREE_HPP
#define MERKLE_TREE_HPP

#include <string>
#include <vector>
#include <iostream>
#include "SHA256.hpp"

struct MerkleProofNode {
    std::string siblingHash;
    bool isLeftSibling; // true if sibling is left child, false if sibling is right child
};

class MerkleTree {
private:
    std::vector<std::vector<std::string>> m_layers;
    std::string m_rootHash;

public:
    MerkleTree();

    // Construct the Merkle Tree bottom-up from leaf hashes
    void buildTree(const std::vector<std::string>& leafHashes);

    // Get the Merkle Root Hash
    std::string getRootHash() const;

    // Get all tree layers (for inspection/visualization)
    const std::vector<std::vector<std::string>>& getLayers() const;

    // Generate inclusion proof (audit path) for a specific leaf index
    std::vector<MerkleProofNode> getProof(size_t leafIndex) const;

    // Static function to verify a leaf hash against proof and expected root hash
    static bool verifyProof(const std::string& leafHash,
                            const std::vector<MerkleProofNode>& proof,
                            const std::string& expectedRootHash);

    // Print visual tree structure to console
    void printTree() const;

    // Helper to hash pairing of two nodes
    static std::string combineHashes(const std::string& left, const std::string& right);
};

#endif // MERKLE_TREE_HPP
