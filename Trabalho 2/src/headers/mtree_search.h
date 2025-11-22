#ifndef MTREE_SEARCH_H
#define MTREE_SEARCH_H

#include "image_database.h"
#include <vector>
#include <memory>
#include <algorithm>
#include <limits>
#include <queue>
#include <cmath>

/**
 * @brief M-Tree (Metric Tree) para busca de similaridade em espaços métricos
 *
 * CONCEITO:
 * M-Tree é uma estrutura de árvore balanceada que funciona em qualquer espaço
 * métrico (onde existe uma função de distância que satisfaz propriedades métricas).
 *
 * PROPRIEDADES MÉTRICAS UTILIZADAS:
 * 1. Não-negatividade: d(x,y) >= 0
 * 2. Identidade: d(x,y) = 0 ⟺ x = y
 * 3. Simetria: d(x,y) = d(y,x)
 * 4. Desigualdade Triangular: d(x,z) <= d(x,y) + d(y,z)
 *
 * TÉCNICA DE PODA:
 * Usa a desigualdade triangular para podar subárvores sem calcular todas as distâncias:
 * Se |d(query, parent) - radius| > threshold, pode podar toda a subárvore
 *
 * ESTRUTURA:
 * - Cada nó tem um routing object (objeto representativo)
 * - Cada nó tem um covering radius (raio de cobertura)
 * - Nós folha armazenam as imagens
 * - Nós internos armazenam ponteiros para subárvores
 *
 * COMPLEXIDADES:
 * - Inserção: O(log n) esperado
 * - Busca: O(log n + k) onde k = resultados
 * - Espaço: O(n)
 *
 * PARÂMETROS:
 * - maxCapacity: Capacidade máxima de cada nó antes de split
 *
 * REFERÊNCIA:
 * Ciaccia, Patella & Zezula (1997) - "M-tree: An Efficient Access Method for Similarity Search in Metric Spaces"
 */
class MTree_Search : public ImageDatabase {
private:
    // Capacidade máxima de cada nó
    int maxCapacity;
    int totalImages;

    // Forward declaration
    class MTreeNode;

    /**
     * @brief Entrada em um nó do M-Tree
     * Armazena uma imagem ou referência a um nó filho, junto com metadados
     */
    struct Entry {
        Image image;                        // Objeto de roteamento (routing object)
        std::unique_ptr<MTreeNode> subtree;  // Subárvore (nullptr se folha)
        double coveringRadius;              // Raio de cobertura desta entrada
        double distanceToParent;            // Distância ao objeto pai (para poda)

        Entry(const Image& img)
            : image(img), subtree(nullptr), coveringRadius(0.0), distanceToParent(0.0) {}

        bool isLeaf() const { return subtree == nullptr; }
    };

    /**
     * @brief Nó do M-Tree
     * Pode ser nó folha (armazena imagens) ou nó interno (referências a subárvores)
     */
    class MTreeNode {
    public:
        std::vector<Entry> entries;  // Entradas deste nó
        bool isLeaf;                 // Flag indicando se é folha

        MTreeNode(bool leaf = true) : isLeaf(leaf) {}

        bool isFull(int capacity) const {
            return entries.size() >= static_cast<size_t>(capacity);
        }
    };

    std::unique_ptr<MTreeNode> root;  // Raiz da árvore
    int maxDepth;                     // Profundidade máxima observada

    /**
     * @brief Insere uma imagem recursivamente no M-Tree
     * @param node Nó atual
     * @param img Imagem a inserir
     * @param depth Profundidade atual
     * @return Ponteiro para possível novo nó criado por split
     */
    std::unique_ptr<Entry> insertRecursive(MTreeNode* node, const Image& img, int depth = 0) {
        maxDepth = std::max(maxDepth, depth);

        if (node->isLeaf) {
            // Nó folha: adiciona a imagem diretamente
            node->entries.emplace_back(img);

            // Se excedeu capacidade, precisa fazer split
            if (node->isFull(maxCapacity)) {
                return splitNode(node);
            }
            return nullptr;

        } else {
            // Nó interno: escolhe melhor subárvore para inserir
            int bestIndex = chooseSubtree(node, img);

            // Insere recursivamente
            auto promoted = insertRecursive(node->entries[bestIndex].subtree.get(), img, depth + 1);

            // Atualiza covering radius
            double dist = img.distanceTo(node->entries[bestIndex].image);
            node->entries[bestIndex].coveringRadius =
                std::max(node->entries[bestIndex].coveringRadius, dist);

            // Se houve split na subárvore, adiciona entrada promovida
            if (promoted) {
                node->entries.push_back(std::move(*promoted));

                // Se este nó também ficou cheio, faz split
                if (node->isFull(maxCapacity)) {
                    return splitNode(node);
                }
            }

            return nullptr;
        }
    }

    /**
     * @brief Escolhe a melhor subárvore para inserir uma imagem
     * HEURÍSTICA: Escolhe a entrada cuja raiz está mais próxima da imagem
     * @param node Nó interno
     * @param img Imagem a inserir
     * @return Índice da melhor entrada
     */
    int chooseSubtree(MTreeNode* node, const Image& img) {
        double minDist = std::numeric_limits<double>::max();
        int bestIndex = 0;

        for (size_t i = 0; i < node->entries.size(); i++) {
            double dist = img.distanceTo(node->entries[i].image);

            // Prefere subárvore que já cobre a imagem
            if (dist <= node->entries[i].coveringRadius) {
                return i;
            }

            // Senão, escolhe a mais próxima
            if (dist < minDist) {
                minDist = dist;
                bestIndex = i;
            }
        }

        return bestIndex;
    }

    /**
     * @brief Divide um nó cheio em dois nós
     * POLÍTICA DE SPLIT: Balanced Split (divide mais ou menos igualmente)
     * @param node Nó a dividir
     * @return Nova entrada promovida para o nó pai
     */
    std::unique_ptr<Entry> splitNode(MTreeNode* node) {
        if (node->entries.size() < 2) return nullptr;

        // Escolhe dois objetos de roteamento (routing objects) como sementes
        // HEURÍSTICA: Escolhe os dois objetos mais distantes
        auto [seed1, seed2] = chooseSplitSeeds(node);

        // Cria dois novos nós
        auto newNode1 = std::make_unique<MTreeNode>(node->isLeaf);
        auto newNode2 = std::make_unique<MTreeNode>(node->isLeaf);

        // Distribui as entradas entre os dois nós
        std::vector<Entry> entries = std::move(node->entries);

        newNode1->entries.push_back(std::move(entries[seed1]));
        newNode2->entries.push_back(std::move(entries[seed2]));

        // Distribui as outras entradas
        for (size_t i = 0; i < entries.size(); i++) {
            if (i == seed1 || i == seed2) continue;

            double dist1 = entries[i].image.distanceTo(newNode1->entries[0].image);
            double dist2 = entries[i].image.distanceTo(newNode2->entries[0].image);

            // Atribui à subárvore mais próxima (balanceamento)
            if (dist1 <= dist2) {
                newNode1->entries.push_back(std::move(entries[i]));
            } else {
                newNode2->entries.push_back(std::move(entries[i]));
            }
        }

        // Calcula covering radius para cada novo nó
        double radius1 = computeCoveringRadius(newNode1.get());
        double radius2 = computeCoveringRadius(newNode2.get());

        // Substitui nó original pelo primeiro novo nó
        node->entries = std::move(newNode1->entries);

        // Cria entrada para segundo nó (será inserida no pai)
        auto promoted = std::make_unique<Entry>(newNode2->entries[0].image);
        promoted->subtree = std::move(newNode2);
        promoted->coveringRadius = radius2;

        return promoted;
    }

    /**
     * @brief Escolhe duas sementes para split (objetos mais distantes)
     * @param node Nó a dividir
     * @return Par de índices das sementes escolhidas
     */
    std::pair<size_t, size_t> chooseSplitSeeds(MTreeNode* node) {
        double maxDist = -1;
        size_t seed1 = 0, seed2 = 1;

        // Procura par de objetos mais distantes (O(n²))
        for (size_t i = 0; i < node->entries.size(); i++) {
            for (size_t j = i + 1; j < node->entries.size(); j++) {
                double dist = node->entries[i].image.distanceTo(node->entries[j].image);
                if (dist > maxDist) {
                    maxDist = dist;
                    seed1 = i;
                    seed2 = j;
                }
            }
        }

        return {seed1, seed2};
    }

    /**
     * @brief Calcula o covering radius de um nó
     * @param node Nó para calcular o raio
     * @return Raio de cobertura (máxima distância ao routing object)
     */
    double computeCoveringRadius(MTreeNode* node) {
        if (node->entries.empty()) return 0.0;

        double maxDist = 0.0;
        const Image& routingObj = node->entries[0].image;

        for (const auto& entry : node->entries) {
            double dist = routingObj.distanceTo(entry.image);
            if (entry.subtree) {
                dist += entry.coveringRadius;  // Considera raio da subárvore
            }
            maxDist = std::max(maxDist, dist);
        }

        return maxDist;
    }

    /**
     * @brief Busca range query recursiva com poda baseada em desigualdade triangular
     * @param node Nó atual
     * @param query Imagem de consulta
     * @param threshold Raio de busca
     * @param results Vetor de resultados (por referência)
     * @param distToParent Distância do query ao pai deste nó (para poda)
     */
    void rangeSearchRecursive(MTreeNode* node, const Image& query, double threshold,
                             std::vector<Image>& results, double distToParent = 0.0) {
        if (!node) return;

        for (auto& entry : node->entries) {
            // Calcula distância ao routing object
            double dist = query.distanceTo(entry.image);

            if (entry.isLeaf()) {
                // Nó folha: verifica se está dentro do threshold
                if (dist <= threshold) {
                    results.push_back(entry.image);
                }
            } else {
                // Nó interno: usa PODA baseada em desigualdade triangular
                // Se |dist - coveringRadius| > threshold, pode podar

                bool canPrune = false;

                // PODA 1: Distância mínima possível é maior que threshold
                if (dist - entry.coveringRadius > threshold) {
                    canPrune = true;
                }

                // PODA 2: Usando distância ao pai (se disponível)
                // Pela desigualdade triangular: |d(q,o) - d(o,p)| <= d(q,p)
                // Se |dist - entry.distanceToParent| > threshold + distToParent, pode podar
                if (distToParent > 0 &&
                    std::abs(dist - entry.distanceToParent) > threshold + distToParent) {
                    canPrune = true;
                }

                if (!canPrune) {
                    // Não pode podar: busca recursivamente
                    rangeSearchRecursive(entry.subtree.get(), query, threshold, results, dist);
                }
            }
        }
    }

    /**
     * @brief Conta nós recursivamente para estatísticas
     */
    void countNodes(MTreeNode* node, int& leafCount, int& internalCount) const {
        if (!node) return;

        if (node->isLeaf) {
            leafCount++;
        } else {
            internalCount++;
            for (const auto& entry : node->entries) {
                if (entry.subtree) {
                    countNodes(entry.subtree.get(), leafCount, internalCount);
                }
            }
        }
    }

public:
    /**
     * @brief Construtor do M-Tree
     * @param capacity Capacidade máxima de cada nó antes de split
     */
    MTree_Search(int capacity = 10)
        : maxCapacity(capacity), totalImages(0), maxDepth(0) {
        root = std::make_unique<MTreeNode>(true);  // Raiz começa como folha
    }

    void insert(const Image& img) override {
        auto promoted = insertRecursive(root.get(), img);

        // Se raiz foi dividida, cria nova raiz
        if (promoted) {
            auto newRoot = std::make_unique<MTreeNode>(false);  // Nó interno

            // Primeira entrada: antiga raiz
            Entry rootEntry(root->entries[0].image);
            rootEntry.subtree = std::move(root);
            rootEntry.coveringRadius = computeCoveringRadius(rootEntry.subtree.get());
            newRoot->entries.push_back(std::move(rootEntry));

            // Segunda entrada: nó promovido do split
            newRoot->entries.push_back(std::move(*promoted));

            root = std::move(newRoot);
        }

        totalImages++;
    }

    std::vector<Image> findSimilar(const Image& query, double threshold) override {
        std::vector<Image> results;
        rangeSearchRecursive(root.get(), query, threshold, results);

        // Ordena por distância
        std::sort(results.begin(), results.end(),
                 [&query](const Image& a, const Image& b) {
                     return query.distanceTo(a) < query.distanceTo(b);
                 });

        return results;
    }

    std::string getName() const override {
        return "M-Tree Search (capacity=" + std::to_string(maxCapacity) + ")";
    }

    /**
     * @brief Estatísticas da estrutura M-Tree
     */
    void printStats() const {
        int leafCount = 0, internalCount = 0;
        countNodes(root.get(), leafCount, internalCount);

        printf("\n=== M-Tree Statistics ===\n");
        printf("Max capacity per node: %d\n", maxCapacity);
        printf("Total images: %d\n", totalImages);
        printf("Max depth: %d\n", maxDepth);
        printf("Leaf nodes: %d\n", leafCount);
        printf("Internal nodes: %d\n", internalCount);
        printf("Total nodes: %d\n", leafCount + internalCount);

        if (leafCount > 0) {
            printf("Average images per leaf: %.2f\n",
                   static_cast<double>(totalImages) / leafCount);
        }

        printf("Tree height: %d\n", maxDepth);
        printf("Branching factor: %.2f\n",
               internalCount > 0 ? static_cast<double>(leafCount) / internalCount : 0.0);
    }
};

#endif // MTREE_SEARCH_H
