#ifndef QUADTREE_H
#define QUADTREE_H

#include <vector>
#include <memory>
#include <array>
#include <stack>
#include <queue>
#include <algorithm>
#include <iostream>
#include <string>
#include <cmath>

// Forward declaration
struct Image;
class ImageDatabase;

// No da Quadtree (2D - usando apenas R e G)
struct QuadtreeNode {
    double minR, maxR, minG, maxG;
    std::vector<Image> images;
    std::array<std::unique_ptr<QuadtreeNode>, 4> children;
    bool isLeaf;
    
    QuadtreeNode(double minR, double maxR, double minG, double maxG)
        : minR(minR), maxR(maxR), minG(minG), maxG(maxG), isLeaf(true) {
        for (auto& child : children) {
            child = nullptr;
        }
    }
    
    bool contains(const Image& img) const {
        return img.r >= minR && img.r <= maxR &&
               img.g >= minG && img.g <= maxG;
    }
    
    int getChildIndex(const Image& img) const {
        int index = 0;
        double midR = (minR + maxR) / 2.0;
        double midG = (minG + maxG) / 2.0;
        
        if (img.r >= midR) index |= 2;  // Bit 1: R >= midR
        if (img.g >= midG) index |= 1;  // Bit 0: G >= midG
        
        return index;
    }
    
    void createChildren() {
        double midR = (minR + maxR) / 2.0;
        double midG = (minG + maxG) / 2.0;
        
        // 4 quadrantes: 
        // 0: [minR,midR] x [minG,midG]  (bottom-left)
        // 1: [minR,midR] x [midG,maxG]  (top-left)  
        // 2: [midR,maxR] x [minG,midG]  (bottom-right)
        // 3: [midR,maxR] x [midG,maxG]  (top-right)
        children[0] = std::make_unique<QuadtreeNode>(minR, midR, minG, midG);
        children[1] = std::make_unique<QuadtreeNode>(minR, midR, midG, maxG);
        children[2] = std::make_unique<QuadtreeNode>(midR, maxR, minG, midG);
        children[3] = std::make_unique<QuadtreeNode>(midR, maxR, midG, maxG);
        
        isLeaf = false;
    }
};

// Quadtree Search Iterativa
class QuadtreeIterativeSearch : public ImageDatabase {
private:
    std::unique_ptr<QuadtreeNode> root;
    int maxImagesPerNode;
    int totalImages;
    int maxDepth;
    
    void insertIterative(const Image& img) {
        QuadtreeNode* currentNode = root.get();
        int currentDepth = 0;
        
        while (true) {
            maxDepth = std::max(maxDepth, currentDepth);
            
            if (currentNode->isLeaf) {
                currentNode->images.push_back(img);
                
                // Verificar se precisa dividir
                if (static_cast<int>(currentNode->images.size()) > maxImagesPerNode && currentDepth < 10) {
                    currentNode->createChildren();
                    
                    // Redistribuir todas as imagens
                    std::vector<Image> imagesToRedistribute = currentNode->images;
                    currentNode->images.clear();
                    
                    // Redistribuir cada imagem para seus filhos
                    for (const auto& existingImg : imagesToRedistribute) {
                        int childIdx = currentNode->getChildIndex(existingImg);
                        insertIntoChild(currentNode->children[childIdx].get(), existingImg, currentDepth + 1);
                    }
                }
                break;
            } else {
                int childIdx = currentNode->getChildIndex(img);
                currentNode = currentNode->children[childIdx].get();
                currentDepth++;
            }
        }
    }
    
    // Funcao auxiliar iterativa para insercao em filhos
    void insertIntoChild(QuadtreeNode* startNode, const Image& img, int startDepth) {
        std::stack<std::pair<QuadtreeNode*, int>> nodeStack;
        nodeStack.push({startNode, startDepth});
        
        while (!nodeStack.empty()) {
            auto [node, depth] = nodeStack.top();
            nodeStack.pop();
            
            maxDepth = std::max(maxDepth, depth);
            
            if (node->isLeaf) {
                node->images.push_back(img);
                
                if (static_cast<int>(node->images.size()) > maxImagesPerNode && depth < 10) {
                    node->createChildren();
                    
                    std::vector<Image> imagesToRedistribute = node->images;
                    node->images.clear();
                    
                    for (const auto& existingImg : imagesToRedistribute) {
                        int childIdx = node->getChildIndex(existingImg);
                        nodeStack.push({node->children[childIdx].get(), depth + 1});
                        
                        // Inserir diretamente no no filho
                        QuadtreeNode* targetChild = node->children[childIdx].get();
                        targetChild->images.push_back(existingImg);
                    }
                }
            } else {
                int childIdx = node->getChildIndex(img);
                nodeStack.push({node->children[childIdx].get(), depth + 1});
            }
        }
    }
    
    bool nodeIntersectsQueryRadius(QuadtreeNode* node, const Image& query, double threshold) const {
        // Calcular distancia minima do ponto query ao retangulo do no
        double minDistSq = 0.0;
        
        // Verificar componente R
        if (query.r < node->minR) {
            double diff = node->minR - query.r;
            minDistSq += diff * diff;
        } else if (query.r > node->maxR) {
            double diff = query.r - node->maxR;
            minDistSq += diff * diff;
        }
        
        // Verificar componente G
        if (query.g < node->minG) {
            double diff = node->minG - query.g;
            minDistSq += diff * diff;
        } else if (query.g > node->maxG) {
            double diff = query.g - node->maxG;
            minDistSq += diff * diff;
        }
        
        // Para Quadtree 2D, ainda precisamos considerar B na distancia final
        // mas para pruning, usamos apenas R,G
        return sqrt(minDistSq) <= threshold;
    }
    
    void searchIterative(const Image& query, double threshold, std::vector<Image>& results) {
        std::queue<QuadtreeNode*> queue;
        queue.push(root.get());
        
        while (!queue.empty()) {
            QuadtreeNode* node = queue.front();
            queue.pop();
            
            if (!node) continue;
            
            if (!nodeIntersectsQueryRadius(node, query, threshold)) {
                continue;
            }
            
            if (node->isLeaf) {
                for (const auto& img : node->images) {
                    double distance = query.distanceTo(img); // Distancia 3D completa (R,G,B)
                    if (distance <= threshold) {
                        results.push_back(img);
                    }
                }
            } else {
                for (const auto& child : node->children) {
                    if (child) {
                        queue.push(child.get());
                    }
                }
            }
        }
    }
    
    void countNodes(QuadtreeNode* node, int& leafCount, int& internalCount) const {
        if (!node) return;
        
        std::queue<QuadtreeNode*> queue;
        queue.push(node);
        
        while (!queue.empty()) {
            QuadtreeNode* current = queue.front();
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
    QuadtreeIterativeSearch(int maxImages = 25) 
        : maxImagesPerNode(maxImages), totalImages(0), maxDepth(0) {
        root = std::make_unique<QuadtreeNode>(0, 255, 0, 255);
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
        return "Quadtree Iterative (maxPerNode=" + std::to_string(maxImagesPerNode) + ")";
    }
    
    void printStats() const {
        int leafCount = 0, internalCount = 0;
        countNodes(root.get(), leafCount, internalCount);
        
        std::cout << "Quadtree Iterative Stats:" << std::endl;
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