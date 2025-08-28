/*
=============================================================================
BENCHMARK 100M APENAS - PAA Assignment 1 - TESTE ESPECÍFICO
=============================================================================

Testa apenas 100M imagens com ordem específica:
1. Quadtree (primeiro)
2. Octree (segundo)  
3. Hash (terceiro)
4. Linear (último)

Com avisos de progresso para cada estrutura.
=============================================================================
*/

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <cmath>
#include <random>
#include <algorithm>
#include <unordered_map>
#include <memory>
#include <array>
#include <fstream>
#include <sstream>
#include <cstdio>
#include <functional>

// ============================================================================
// ESTRUTURA DE DADOS - IMAGEM
// ============================================================================
struct Image {
    int id;
    std::string filename;
    double r, g, b;
    
    Image(int _id, const std::string& _filename, double _r, double _g, double _b) 
        : id(_id), filename(_filename), r(_r), g(_g), b(_b) {}
    
    double distanceTo(const Image& other) const {
        double dr = r - other.r;
        double dg = g - other.g;
        double db = b - other.b;
        return std::sqrt(dr*dr + dg*dg + db*db);
    }
};

// ============================================================================
// INTERFACE COMUM
// ============================================================================
class ImageDatabase {
public:
    virtual ~ImageDatabase() = default;
    virtual void insert(const Image& img) = 0;
    virtual std::vector<Image> findSimilar(const Image& query, double threshold) const = 0;
    virtual size_t size() const = 0;
    virtual std::string getName() const = 0;
};

// ============================================================================
// 1. BUSCA LINEAR
// ============================================================================
class LinearSearch : public ImageDatabase {
private:
    std::vector<Image> images;

public:
    void insert(const Image& img) override {
        images.push_back(img);
    }
    
    std::vector<Image> findSimilar(const Image& query, double threshold) const override {
        std::vector<Image> results;
        for (const auto& img : images) {
            if (query.distanceTo(img) <= threshold) {
                results.push_back(img);
            }
        }
        return results;
    }
    
    size_t size() const override { return images.size(); }
    std::string getName() const override { return "Linear Search"; }
};

// ============================================================================
// 2. HASH SEARCH
// ============================================================================
class HashSearch : public ImageDatabase {
private:
    static constexpr int GRID_SIZE = 32;
    static constexpr double CELL_SIZE = 255.0 / GRID_SIZE;
    std::unordered_map<uint64_t, std::vector<Image>> grid;
    size_t totalImages = 0;

    uint64_t getHashKey(double r, double g, double b) const {
        int cellR = std::min((int)(r / CELL_SIZE), GRID_SIZE - 1);
        int cellG = std::min((int)(g / CELL_SIZE), GRID_SIZE - 1);
        int cellB = std::min((int)(b / CELL_SIZE), GRID_SIZE - 1);
        return ((uint64_t)cellR << 32) | ((uint64_t)cellG << 16) | (uint64_t)cellB;
    }

public:
    void insert(const Image& img) override {
        uint64_t key = getHashKey(img.r, img.g, img.b);
        grid[key].push_back(img);
        totalImages++;
    }
    
    std::vector<Image> findSimilar(const Image& query, double threshold) const override {
        std::vector<Image> results;
        uint64_t queryKey = getHashKey(query.r, query.g, query.b);
        
        for (int dr = -1; dr <= 1; dr++) {
            for (int dg = -1; dg <= 1; dg++) {
                for (int db = -1; db <= 1; db++) {
                    int cellR = std::min(std::max((int)(query.r / CELL_SIZE) + dr, 0), GRID_SIZE - 1);
                    int cellG = std::min(std::max((int)(query.g / CELL_SIZE) + dg, 0), GRID_SIZE - 1);
                    int cellB = std::min(std::max((int)(query.b / CELL_SIZE) + db, 0), GRID_SIZE - 1);
                    
                    uint64_t neighborKey = ((uint64_t)cellR << 32) | ((uint64_t)cellG << 16) | (uint64_t)cellB;
                    auto it = grid.find(neighborKey);
                    if (it != grid.end()) {
                        for (const auto& img : it->second) {
                            if (query.distanceTo(img) <= threshold) {
                                results.push_back(img);
                            }
                        }
                    }
                }
            }
        }
        return results;
    }
    
    size_t size() const override { return totalImages; }
    std::string getName() const override { return "Hash Search"; }
};

// ============================================================================
// 3. OCTREE 
// ============================================================================
struct OctreeNode {
    double minR, maxR, minG, maxG, minB, maxB;
    std::vector<Image> images;
    std::array<std::unique_ptr<OctreeNode>, 8> children;
    bool isLeaf;
    
    OctreeNode(double _minR, double _maxR, double _minG, double _maxG, double _minB, double _maxB)
        : minR(_minR), maxR(_maxR), minG(_minG), maxG(_maxG), minB(_minB), maxB(_maxB), isLeaf(true) {}
        
    void createChildren() {
        if (!isLeaf) return;
        isLeaf = false;
        
        double midR = (minR + maxR) / 2;
        double midG = (minG + maxG) / 2;
        double midB = (minB + maxB) / 2;
        
        children[0] = std::make_unique<OctreeNode>(minR, midR, minG, midG, minB, midB);
        children[1] = std::make_unique<OctreeNode>(midR, maxR, minG, midG, minB, midB);
        children[2] = std::make_unique<OctreeNode>(minR, midR, maxR, maxG, minB, midB);
        children[3] = std::make_unique<OctreeNode>(midR, maxR, maxR, maxG, minB, midB);
        children[4] = std::make_unique<OctreeNode>(minR, midR, minG, midG, midB, maxB);
        children[5] = std::make_unique<OctreeNode>(midR, maxR, minG, midG, midB, maxB);
        children[6] = std::make_unique<OctreeNode>(minR, midR, maxR, maxG, midB, maxB);
        children[7] = std::make_unique<OctreeNode>(midR, maxR, maxR, maxG, midB, maxB);
    }
    
    int getChildIndex(const Image& img) const {
        double midR = (minR + maxR) / 2;
        double midG = (minG + maxG) / 2;
        double midB = (minB + maxB) / 2;
        
        int index = 0;
        if (img.r >= midR) index |= 1;
        if (img.g >= midG) index |= 2;
        if (img.b >= midB) index |= 4;
        return index;
    }
};

class OctreeSearch : public ImageDatabase {
private:
    std::unique_ptr<OctreeNode> root;
    size_t totalImages = 0;
    static constexpr int maxImagesPerNode = 20;

    void insertRecursive(OctreeNode* node, const Image& img, int depth = 0) {
        if (node->isLeaf) {
            node->images.push_back(img);
            if (static_cast<int>(node->images.size()) > maxImagesPerNode && depth < 15) {
                node->createChildren();
                for (const auto& existingImg : node->images) {
                    int childIdx = node->getChildIndex(existingImg);
                    insertRecursive(node->children[childIdx].get(), existingImg, depth + 1);
                }
                node->images.clear();
            }
        } else {
            int childIdx = node->getChildIndex(img);
            insertRecursive(node->children[childIdx].get(), img, depth + 1);
        }
    }
    
    void searchRecursive(OctreeNode* node, const Image& query, double threshold, std::vector<Image>& results) const {
        if (!node) return;
        
        if (node->isLeaf) {
            for (const auto& img : node->images) {
                if (query.distanceTo(img) <= threshold) {
                    results.push_back(img);
                }
            }
        } else {
            for (const auto& child : node->children) {
                if (child) {
                    searchRecursive(child.get(), query, threshold, results);
                }
            }
        }
    }

public:
    OctreeSearch() : root(std::make_unique<OctreeNode>(0, 255, 0, 255, 0, 255)) {}
    
    void insert(const Image& img) override {
        insertRecursive(root.get(), img);
        totalImages++;
    }
    
    std::vector<Image> findSimilar(const Image& query, double threshold) const override {
        std::vector<Image> results;
        searchRecursive(root.get(), query, threshold, results);
        return results;
    }
    
    size_t size() const override { return totalImages; }
    std::string getName() const override { return "Octree Search"; }
};

// ============================================================================
// 4. QUADTREE
// ============================================================================
struct QuadtreeNode {
    double minR, maxR, minG, maxG;
    std::vector<Image> images;
    std::array<std::unique_ptr<QuadtreeNode>, 4> children;
    bool isLeaf;
    
    QuadtreeNode(double _minR, double _maxR, double _minG, double _maxG)
        : minR(_minR), maxR(_maxR), minG(_minG), maxG(_maxG), isLeaf(true) {}
        
    void createChildren() {
        if (!isLeaf) return;
        isLeaf = false;
        
        double midR = (minR + maxR) / 2;
        double midG = (minG + maxG) / 2;
        
        children[0] = std::make_unique<QuadtreeNode>(minR, midR, minG, midG);
        children[1] = std::make_unique<QuadtreeNode>(midR, maxR, minG, midG);
        children[2] = std::make_unique<QuadtreeNode>(minR, midR, midG, maxG);
        children[3] = std::make_unique<QuadtreeNode>(midR, maxR, midG, maxG);
    }
    
    int getChildIndex(const Image& img) const {
        double midR = (minR + maxR) / 2;
        double midG = (minG + maxG) / 2;
        
        int index = 0;
        if (img.r >= midR) index |= 1;
        if (img.g >= midG) index |= 2;
        return index;
    }
};

class QuadtreeSearch : public ImageDatabase {
private:
    std::unique_ptr<QuadtreeNode> root;
    size_t totalImages = 0;
    static constexpr int maxImagesPerNode = 20;

    void insertRecursive(QuadtreeNode* node, const Image& img, int depth = 0) {
        if (node->isLeaf) {
            node->images.push_back(img);
            if (static_cast<int>(node->images.size()) > maxImagesPerNode && depth < 15) {
                node->createChildren();
                for (const auto& existingImg : node->images) {
                    int childIdx = node->getChildIndex(existingImg);
                    insertRecursive(node->children[childIdx].get(), existingImg, depth + 1);
                }
                node->images.clear();
            }
        } else {
            int childIdx = node->getChildIndex(img);
            insertRecursive(node->children[childIdx].get(), img, depth + 1);
        }
    }
    
    void searchRecursive(QuadtreeNode* node, const Image& query, double threshold, std::vector<Image>& results) const {
        if (!node) return;
        
        if (node->isLeaf) {
            for (const auto& img : node->images) {
                if (query.distanceTo(img) <= threshold) {
                    results.push_back(img);
                }
            }
        } else {
            for (const auto& child : node->children) {
                if (child) {
                    searchRecursive(child.get(), query, threshold, results);
                }
            }
        }
    }

public:
    QuadtreeSearch() : root(std::make_unique<QuadtreeNode>(0, 255, 0, 255)) {}
    
    void insert(const Image& img) override {
        insertRecursive(root.get(), img);
        totalImages++;
    }
    
    std::vector<Image> findSimilar(const Image& query, double threshold) const override {
        std::vector<Image> results;
        searchRecursive(root.get(), query, threshold, results);
        return results;
    }
    
    size_t size() const override { return totalImages; }
    std::string getName() const override { return "Quadtree Search"; }
};

// ============================================================================
// GERADOR DE DADOS
// ============================================================================
std::vector<Image> generateSyntheticDataset(int count) {
    std::vector<Image> images;
    images.reserve(count);
    
    // SEED FIXA para consistência entre execuções
    std::mt19937 gen(42);
    std::uniform_real_distribution<> colorDist(0.0, 255.0);
    
    for (int i = 0; i < count; ++i) {
        double r = colorDist(gen);
        double g = colorDist(gen);
        double b = colorDist(gen);
        images.emplace_back(i, "synthetic_" + std::to_string(i) + ".jpg", r, g, b);
    }
    
    return images;
}

// ============================================================================
// SISTEMA DE BENCHMARK
// ============================================================================
struct BenchmarkResult {
    std::string structureName;
    int datasetSize;
    double insertTime;
    double searchTime;
    int resultsFound;
};

BenchmarkResult benchmarkStructure(std::unique_ptr<ImageDatabase> db, 
                                   const std::vector<Image>& dataset,
                                   const Image& query, 
                                   double threshold) {
    BenchmarkResult result;
    result.structureName = db->getName();
    result.datasetSize = dataset.size();
    
    // Teste de Inserção
    auto startInsert = std::chrono::high_resolution_clock::now();
    for (const auto& img : dataset) {
        db->insert(img);
    }
    auto endInsert = std::chrono::high_resolution_clock::now();
    result.insertTime = std::chrono::duration<double, std::milli>(endInsert - startInsert).count();
    
    // Teste de Busca
    auto startSearch = std::chrono::high_resolution_clock::now();
    auto results = db->findSimilar(query, threshold);
    auto endSearch = std::chrono::high_resolution_clock::now();
    result.searchTime = std::chrono::duration<double, std::milli>(endSearch - startSearch).count();
    result.resultsFound = results.size();
    
    return result;
}

// ============================================================================
// MAIN - BENCHMARK 100M APENAS
// ============================================================================
int main() {
    std::cout << "=============================================================================\n";
    std::cout << "BENCHMARK 100M APENAS - PAA Assignment 1\n";
    std::cout << "=============================================================================\n\n";
    
    const int scale = 100000000;  // 100M imagens
    const Image queryPoint(999999, "query.jpg", 128, 128, 128);
    const double threshold = 50.0;
    
    std::cout << "Configuracao do Benchmark:\n";
    std::cout << "   Dataset: 100M imagens sinteticas\n";
    std::cout << "   Query Point: RGB(" << queryPoint.r << ", " << queryPoint.g << ", " << queryPoint.b << ")\n";
    std::cout << "   Threshold: " << threshold << "\n";
    std::cout << "   Dados: SEED FIXA (42)\n";
    std::cout << "   Ordem: Quadtree -> Octree -> Hash -> Linear\n\n";
    
    std::vector<BenchmarkResult> results;
    
    // ORDEM ESPECÍFICA: Quadtree, Octree, Hash, Linear
    std::vector<std::pair<std::string, std::function<std::unique_ptr<ImageDatabase>()>>> structures = {
        {"QuadtreeSearch", []() { return std::make_unique<QuadtreeSearch>(); }},
        {"OctreeSearch", []() { return std::make_unique<OctreeSearch>(); }},
        {"HashSearch", []() { return std::make_unique<HashSearch>(); }},
        {"LinearSearch", []() { return std::make_unique<LinearSearch>(); }}
    };
    
    for (size_t i = 0; i < structures.size(); i++) {
        std::cout << "\n=============================================================================\n";
        std::cout << "[" << (i+1) << "/4] TESTANDO: " << structures[i].first << " com 100M imagens\n";
        std::cout << "=============================================================================\n";
        std::cout << "Gerando dataset de 100M imagens... ";
        
        auto dataset = generateSyntheticDataset(scale);
        std::cout << "OK (" << dataset.size() << " imagens)\n";
        
        std::cout << "Executando benchmark para " << structures[i].first << "...\n";
        auto structure = structures[i].second();
        
        auto result = benchmarkStructure(std::move(structure), dataset, queryPoint, threshold);
        results.push_back(result);
        
        std::cout << "RESULTADO: Insert: " << result.insertTime << "ms, Search: " 
                  << result.searchTime << "ms, Found: " << result.resultsFound << "\n";
    }
    
    // Tabela final
    std::cout << "\n=============================================================================\n";
    std::cout << "RESULTADOS FINAIS - 100M IMAGENS\n";
    std::cout << "=============================================================================\n\n";
    
    printf("%-15s %-12s %-12s %-8s\n", "Estrutura", "Insert(ms)", "Search(ms)", "Found");
    std::cout << "-------------------------------------------------------\n";
    
    for (const auto& result : results) {
        printf("%-15s %-12.3f %-12.3f %-8d\n", 
               result.structureName.c_str(), result.insertTime, 
               result.searchTime, result.resultsFound);
    }
    
    std::cout << "\n=============================================================================\n";
    std::cout << "Benchmark 100M Concluido!\n";
    std::cout << "=============================================================================\n";
    
    return 0;
}