#include "MerkleTree.hpp"
#include <iomanip>
#include <algorithm>

MerkleTree::MerkleTree() {
    m_rootHash = "";
}

std::string MerkleTree::combineHashes(const std::string& left, const std::string& right) {
    return SHA256::hash(left + right);
}

void MerkleTree::buildTree(const std::vector<std::string>& leafHashes) {
    m_layers.clear();

    if (leafHashes.empty()) {
        m_rootHash = SHA256::hash("");
        m_layers.push_back({m_rootHash});
        return;
    }

    // Layer 0 is the leaf layer
    m_layers.push_back(leafHashes);

    while (m_layers.back().size() > 1) {
        const auto& currentLayer = m_layers.back();
        std::vector<std::string> nextLayer;

        for (size_t i = 0; i < currentLayer.size(); i += 2) {
            std::string left = currentLayer[i];
            // If odd number of nodes, duplicate the last node
            std::string right = (i + 1 < currentLayer.size()) ? currentLayer[i + 1] : currentLayer[i];
            nextLayer.push_back(combineHashes(left, right));
        }

        m_layers.push_back(nextLayer);
    }

    m_rootHash = m_layers.back()[0];
}

std::string MerkleTree::getRootHash() const {
    return m_rootHash;
}

const std::vector<std::vector<std::string>>& MerkleTree::getLayers() const {
    return m_layers;
}

std::vector<MerkleProofNode> MerkleTree::getProof(size_t leafIndex) const {
    std::vector<MerkleProofNode> proof;

    if (m_layers.empty() || leafIndex >= m_layers[0].size()) {
        return proof;
    }

    size_t idx = leafIndex;
    for (size_t level = 0; level < m_layers.size() - 1; ++level) {
        const auto& currentLayer = m_layers[level];
        bool isEven = (idx % 2 == 0);

        if (isEven) {
            // Sibling is on the right
            std::string siblingHash = (idx + 1 < currentLayer.size()) ? currentLayer[idx + 1] : currentLayer[idx];
            proof.push_back({siblingHash, false});
        } else {
            // Sibling is on the left
            std::string siblingHash = currentLayer[idx - 1];
            proof.push_back({siblingHash, true});
        }

        idx /= 2;
    }

    return proof;
}

bool MerkleTree::verifyProof(const std::string& leafHash,
                             const std::vector<MerkleProofNode>& proof,
                             const std::string& expectedRootHash) {
    std::string currentHash = leafHash;

    for (const auto& node : proof) {
        if (node.isLeftSibling) {
            currentHash = combineHashes(node.siblingHash, currentHash);
        } else {
            currentHash = combineHashes(currentHash, node.siblingHash);
        }
    }

    return (currentHash == expectedRootHash);
}

void MerkleTree::printTree() const {
    if (m_layers.empty()) {
        std::cout << "  [Empty Tree]\n";
        return;
    }

    std::cout << "\n=======================================================\n";
    std::cout << "               MERKLE TREE VISUALIZATION               \n";
    std::cout << "=======================================================\n";

    for (int level = static_cast<int>(m_layers.size()) - 1; level >= 0; --level) {
        if (level == static_cast<int>(m_layers.size()) - 1) {
            std::cout << " [Root Level " << level << "] (1 Node):\n";
        } else if (level == 0) {
            std::cout << " [Leaf Level 0] (" << m_layers[level].size() << " Nodes):\n";
        } else {
            std::cout << " [Internal Level " << level << "] (" << m_layers[level].size() << " Nodes):\n";
        }

        for (size_t i = 0; i < m_layers[level].size(); ++i) {
            std::cout << "   Node [" << i << "]: " << m_layers[level][i].substr(0, 16) << "..." 
                      << m_layers[level][i].substr(m_layers[level][i].length() - 8) << "\n";
        }
        std::cout << "-------------------------------------------------------\n";
    }
}
