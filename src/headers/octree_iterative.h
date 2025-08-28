#ifndef OCTREE_ITERATIVE_H
#define OCTREE_ITERATIVE_H

#include <vector>
#include <memory>
#include <array>
#include <stack>
#include <queue>
#include <algorithm>
#include <iostream>
#include <string>
#include <cmath>
#include <tuple>

// Forward declaration
struct Image;
class ImageDatabase;

// No da Octree iterativa
struct OctreeNodeIterative {
    double minR, maxR, minG, maxG, minB, maxB;
    std::vector<Image> images;
    std::array<std::unique_ptr<OctreeNodeIterative>, 8> children;
    bool isLeaf;
    
    OctreeNodeIterative(double minR, double maxR, double minG, double maxG, double minB, double maxB)
        : minR(minR), maxR(maxR), minG(minG), maxG(maxG), minB(minB), maxB(maxB), isLeaf(true) {
        for (auto& child : children) {
            child = nullptr;
        }
    }
    
    bool contains(const Image& img) const {
        return img.r >= minR && img.r <= maxR &&
               img.g >= minG && img.g <= maxG &&
               img.b >= minB && img.b <= maxB;
    }
    
    int getChildIndex(const Image& img) const {
        int index = 0;
        double midR = (minR + maxR) / 2.0;
        double midG = (minG + maxG) / 2.0;
        double midB = (minB + maxB) / 2.0;
        
        if (img.r >= midR) index |= 4;
        if (img.g >= midG) index |= 2;
        if (img.b >= midB) index |= 1;
        
        return index;
    }
    
    void createChildren() {
        double midR = (minR + maxR) / 2.0;
        double midG = (minG + maxG) / 2.0;
        double midB = (minB + maxB) / 2.0;
        
        children[0] = std::make_unique<OctreeNodeIterative>(minR, midR, minG, midG, minB, midB);
        children[1] = std::make_unique<OctreeNodeIterative>(minR, midR, minG, midG, midB, maxB);
        children[2] = std::make_unique<OctreeNodeIterative>(minR, midR, minG, maxG, minB, midB);
        children[3] = std::make_unique<OctreeNodeIterative>(minR, midR, minG, maxG, midB, maxB);
        children[4] = std::make_unique<OctreeNodeIterative>(midR, maxR, minG, midG, minB, midB);
        children[5] = std::make_unique<OctreeNodeIterative>(midR, maxR, minG, midG, midB, maxB);
        children[6] = std::make_unique<OctreeNodeIterative>(midR, maxR, minG, maxG, minB, midB);
        children[7] = std::make_unique<OctreeNodeIterative>(midR, maxR, minG, maxG, minB, maxB);
        
        isLeaf = false;
    }
};

// Estrutura para manter estado durante insercao iterativa
struct InsertionStackFrame {
    OctreeNodeIterative* node;
    const Image* img;
    int depth;
    bool needsSplit;
    
    InsertionStackFrame(OctreeNodeIterative* n, const Image* i, int d, bool split = false) 
        : node(n), img(i), depth(d), needsSplit(split) {}
};

// Octree Search Iterativa
class OctreeIterativeSearch : public ImageDatabase {
private:
    std::unique_ptr<OctreeNodeIterative> root;
    int maxImagesPerNode;
    int totalImages;
    int maxDepth;
    
    void insertIterative(const Image& img) {
        // Usar uma abordagem mais simples: navegar ate o no folha e inserir
        OctreeNodeIterative* currentNode = root.get();
        int currentDepth = 0;
        
        while (true) {
            maxDepth = std::max(maxDepth, currentDepth);
            
            if (currentNode->isLeaf) {
                currentNode->images.push_back(img);
                
                // Verificar se precisa dividir
                if (static_cast<int>(currentNode->images.size()) > maxImagesPerNode) {
                    currentNode->createChildren();
                    
                    // Redistribuir todas as imagens
                    std::vector<Image> imagesToRedistribute = currentNode->images;
                    currentNode->images.clear();
                    
                    // Redistribuir cada imagem para seus filhos
                    for (const auto& existingImg : imagesToRedistribute) {
                        int childIdx = currentNode->getChildIndex(existingImg);
                        
                        // Inserir recursivamente em cada filho (pode gerar mais divisoes)
                        insertIntoChild(currentNode->children[childIdx].get(), existingImg, currentDepth + 1);
                    }
                }
                break; // Insercao concluida
            } else {
                // No interno - descer para filho apropriado
                int childIdx = currentNode->getChildIndex(img);
                currentNode = currentNode->children[childIdx].get();
                currentDepth++;
            }
        }
    }
    
    // Funcao auxiliar 100% iterativa para insercao em filhos
    void insertIntoChild(OctreeNodeIterative* startNode, const Image& img, int startDepth) {
        // Stack para navegacao iterativa
        std::stack<std::pair<OctreeNodeIterative*, int>> nodeStack;
        nodeStack.push({startNode, startDepth});
        
        // Stack para redistribuicao
        std::stack<std::tuple<OctreeNodeIterative*, Image, int>> redistribStack;
        
        while (!nodeStack.empty() || !redistribStack.empty()) {
            // Processar redistribuicoes pendentes primeiro
            if (!redistribStack.empty()) {
                auto [node, image, depth] = redistribStack.top();
                redistribStack.pop();
                nodeStack.push({node, depth});
                
                // Navegar ate no folha para esta imagem
                OctreeNodeIterative* currentNode = node;
                int currentDepth = depth;
                
                while (!currentNode->isLeaf) {
                    int childIdx = currentNode->getChildIndex(image);
                    currentNode = currentNode->children[childIdx].get();
                    currentDepth++;
                }
                
                // Inserir na folha
                currentNode->images.push_back(image);
                maxDepth = std::max(maxDepth, currentDepth);
                
                // Verificar se precisa dividir
                if (static_cast<int>(currentNode->images.size()) > maxImagesPerNode && currentDepth < 10) { // Limitar profundidade
                    currentNode->createChildren();
                    
                    std::vector<Image> toRedistribute = currentNode->images;
                    currentNode->images.clear();
                    
                    for (const auto& existingImg : toRedistribute) {
                        int childIdx = currentNode->getChildIndex(existingImg);
                        redistribStack.push({currentNode->children[childIdx].get(), existingImg, currentDepth + 1});
                    }
                }
                continue;
            }
            
            // Processar nos normais
            auto [node, depth] = nodeStack.top();
            nodeStack.pop();
            
            maxDepth = std::max(maxDepth, depth);
            
            if (node->isLeaf) {
                node->images.push_back(img);
                
                if (static_cast<int>(node->images.size()) > maxImagesPerNode && depth < 10) { // Limitar profundidade
                    node->createChildren();
                    
                    std::vector<Image> imagesToRedistribute = node->images;
                    node->images.clear();
                    
                    for (const auto& existingImg : imagesToRedistribute) {
                        int childIdx = node->getChildIndex(existingImg);
                        redistribStack.push({node->children[childIdx].get(), existingImg, depth + 1});
                    }
                }
            } else {
                int childIdx = node->getChildIndex(img);
                nodeStack.push({node->children[childIdx].get(), depth + 1});
            }
        }
    }
    
    bool nodeIntersectsQueryRadius(OctreeNodeIterative* node, const Image& query, double threshold) const {
        double minDistSq = 0.0;
        
        if (query.r < node->minR) {
            double diff = node->minR - query.r;
            minDistSq += diff * diff;
        } else if (query.r > node->maxR) {
            double diff = query.r - node->maxR;
            minDistSq += diff * diff;
        }
        
        if (query.g < node->minG) {
            double diff = node->minG - query.g;
            minDistSq += diff * diff;
        } else if (query.g > node->maxG) {
            double diff = query.g - node->maxG;
            minDistSq += diff * diff;
        }
        
        if (query.b < node->minB) {
            double diff = node->minB - query.b;
            minDistSq += diff * diff;
        } else if (query.b > node->maxB) {
            double diff = query.b - node->maxB;
            minDistSq += diff * diff;
        }
        
        return sqrt(minDistSq) <= threshold;
    }
    
    void searchIterative(const Image& query, double threshold, std::vector<Image>& results) {
        std::queue<OctreeNodeIterative*> queue;
        queue.push(root.get());
        
        while (!queue.empty()) {
            OctreeNodeIterative* node = queue.front();
            queue.pop();
            
            if (!node) continue;
            
            if (!nodeIntersectsQueryRadius(node, query, threshold)) {
                continue;
            }
            
            if (node->isLeaf) {
                // No folha - verificar todas as imagens
                for (const auto& img : node->images) {
                    double distance = query.distanceTo(img);
                    if (distance <= threshold) {
                        results.push_back(img);
                    }
                }
            } else {
                // No interno - adicionar filhos a fila
                for (const auto& child : node->children) {
                    if (child) {
                        queue.push(child.get());
                    }
                }
            }
        }
    }
    
    void countNodes(OctreeNodeIterative* node, int& leafCount, int& internalCount) const {
        if (!node) return;
        
        std::queue<OctreeNodeIterative*> queue;
        queue.push(node);
        
        while (!queue.empty()) {
            OctreeNodeIterative* current = queue.front();
            queue.pop();
            
            if (current->isLeaf) {
                leafCount++;
            } else {
                internalCount++;
                for (const auto& child : current->children) {
                    if (child) {
                        queue.push(child.get());
                    }
                }
            }
        }
    }
    
public:
    OctreeIterativeSearch(int maxImages = 10) 
        : maxImagesPerNode(maxImages), totalImages(0), maxDepth(0) {
        root = std::make_unique<OctreeNodeIterative>(0, 255, 0, 255, 0, 255);
    }
    
    void insert(const Image& img) override {
        insertIterative(img);
        totalImages++;
    }
    
    std::vector<Image> findSimilar(const Image& query, double threshold) override {
        std::vector<Image> results;
        searchIterative(query, threshold, results);
        
        std::sort(results.begin(), results.end(), 
                 [&query](const Image& a, const Image& b) {
                     return query.distanceTo(a) < query.distanceTo(b);
                 });
        
        return results;
    }
    
    std::string getName() const override {
        return "Octree Iterative (maxPerNode=" + std::to_string(maxImagesPerNode) + ")";
    }
    
    void printStats() const {
        int leafCount = 0, internalCount = 0;
        countNodes(root.get(), leafCount, internalCount);
        
        std::cout << "Octree Iterative Stats:" << std::endl;
        std::cout << "  Total de imagens: " << totalImages << std::endl;
        std::cout << "  Max imagens por no: " << maxImagesPerNode << std::endl;
        std::cout << "  Profundidade maxima: " << maxDepth << std::endl;
        std::cout << "  Nos folha: " << leafCount << std::endl;
        std::cout << "  Nos internos: " << internalCount << std::endl;
        std::cout << "  Total de nos: " << (leafCount + internalCount) << std::endl;
        
        if (leafCount > 0) {
            std::cout << "  Imagens por folha (media): " 
                      << static_cast<double>(totalImages) / leafCount << std::endl;
        }
    }
};

#endif