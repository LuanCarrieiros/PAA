/*
=============================================================================
BENCHMARK RECURSIVO vs ITERATIVO - PAA Assignment 1
=============================================================================

Testa apenas estruturas de arvores espaciais em 4 versoes:
1. Quadtree Recursivo (insercao + busca)
2. Quadtree Iterativo (insercao + busca)  
3. Octree Recursivo (insercao + busca)
4. Octree Iterativo (insercao + busca)

Escalas: 100 → 50M imagens sinteticas
Objetivo: Provar que Recursao > Iteracao para arvores espaciais

=============================================================================
*/

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <cmath>
#include <random>
#include <algorithm>
#include <memory>
#include <array>
#include <stack>
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
// 1. QUADTREE RECURSIVO
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

class QuadtreeRecursivo : public ImageDatabase {
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
    QuadtreeRecursivo() : root(std::make_unique<QuadtreeNode>(0, 255, 0, 255)) {}
    
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
    std::string getName() const override { return "Quadtree Recursivo"; }
};

// ============================================================================
// 2. QUADTREE ITERATIVO
// ============================================================================
struct QuadStackItem {
    QuadtreeNode* node;
    Image img;
    int depth;
};

class QuadtreeIterativo : public ImageDatabase {
private:
    std::unique_ptr<QuadtreeNode> root;
    size_t totalImages = 0;
    static constexpr int maxImagesPerNode = 20;

    void insertIterative(const Image& img) {
        std::stack<QuadStackItem> stack;
        stack.push({root.get(), img, 0});
        
        while (!stack.empty()) {
            auto item = stack.top();
            stack.pop();
            
            if (item.node->isLeaf) {
                item.node->images.push_back(item.img);
                if (static_cast<int>(item.node->images.size()) > maxImagesPerNode && item.depth < 15) {
                    item.node->createChildren();
                    // Redistribuir todas as imagens
                    for (const auto& existingImg : item.node->images) {
                        int childIdx = item.node->getChildIndex(existingImg);
                        stack.push({item.node->children[childIdx].get(), existingImg, item.depth + 1});
                    }
                    item.node->images.clear();
                }
            } else {
                int childIdx = item.node->getChildIndex(item.img);
                stack.push({item.node->children[childIdx].get(), item.img, item.depth + 1});
            }
        }
    }
    
    void searchIterative(const Image& query, double threshold, std::vector<Image>& results) const {
        std::stack<QuadtreeNode*> stack;
        stack.push(root.get());
        
        while (!stack.empty()) {
            QuadtreeNode* node = stack.top();
            stack.pop();
            
            if (!node) continue;
            
            if (node->isLeaf) {
                for (const auto& img : node->images) {
                    if (query.distanceTo(img) <= threshold) {
                        results.push_back(img);
                    }
                }
            } else {
                for (const auto& child : node->children) {
                    if (child) {
                        stack.push(child.get());
                    }
                }
            }
        }
    }

public:
    QuadtreeIterativo() : root(std::make_unique<QuadtreeNode>(0, 255, 0, 255)) {}
    
    void insert(const Image& img) override {
        insertIterative(img);
        totalImages++;
    }
    
    std::vector<Image> findSimilar(const Image& query, double threshold) const override {
        std::vector<Image> results;
        searchIterative(query, threshold, results);
        return results;
    }
    
    size_t size() const override { return totalImages; }
    std::string getName() const override { return "Quadtree Iterativo"; }
};

// ============================================================================
// 3. OCTREE RECURSIVO
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

class OctreeRecursivo : public ImageDatabase {
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
    OctreeRecursivo() : root(std::make_unique<OctreeNode>(0, 255, 0, 255, 0, 255)) {}
    
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
    std::string getName() const override { return "Octree Recursivo"; }
};

// ============================================================================
// 4. OCTREE ITERATIVO
// ============================================================================
struct OctStackItem {
    OctreeNode* node;
    Image img;
    int depth;
};

class OctreeIterativo : public ImageDatabase {
private:
    std::unique_ptr<OctreeNode> root;
    size_t totalImages = 0;
    static constexpr int maxImagesPerNode = 20;

    void insertIterative(const Image& img) {
        std::stack<OctStackItem> stack;
        stack.push({root.get(), img, 0});
        
        while (!stack.empty()) {
            auto item = stack.top();
            stack.pop();
            
            if (item.node->isLeaf) {
                item.node->images.push_back(item.img);
                if (static_cast<int>(item.node->images.size()) > maxImagesPerNode && item.depth < 15) {
                    item.node->createChildren();
                    for (const auto& existingImg : item.node->images) {
                        int childIdx = item.node->getChildIndex(existingImg);
                        stack.push({item.node->children[childIdx].get(), existingImg, item.depth + 1});
                    }
                    item.node->images.clear();
                }
            } else {
                int childIdx = item.node->getChildIndex(item.img);
                stack.push({item.node->children[childIdx].get(), item.img, item.depth + 1});
            }
        }
    }
    
    void searchIterative(const Image& query, double threshold, std::vector<Image>& results) const {
        std::stack<OctreeNode*> stack;
        stack.push(root.get());
        
        while (!stack.empty()) {
            OctreeNode* node = stack.top();
            stack.pop();
            
            if (!node) continue;
            
            if (node->isLeaf) {
                for (const auto& img : node->images) {
                    if (query.distanceTo(img) <= threshold) {
                        results.push_back(img);
                    }
                }
            } else {
                for (const auto& child : node->children) {
                    if (child) {
                        stack.push(child.get());
                    }
                }
            }
        }
    }

public:
    OctreeIterativo() : root(std::make_unique<OctreeNode>(0, 255, 0, 255, 0, 255)) {}
    
    void insert(const Image& img) override {
        insertIterative(img);
        totalImages++;
    }
    
    std::vector<Image> findSimilar(const Image& query, double threshold) const override {
        std::vector<Image> results;
        searchIterative(query, threshold, results);
        return results;
    }
    
    size_t size() const override { return totalImages; }
    std::string getName() const override { return "Octree Iterativo"; }
};

// ============================================================================
// GERADOR DE DADOS
// ============================================================================
std::vector<Image> generateSyntheticDataset(int count) {
    std::vector<Image> images;
    images.reserve(count);
    
    // SEED FIXA para consistencia entre execucoes
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
    
    // Teste de Insercao
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
// MAIN - BENCHMARK RECURSIVO VS ITERATIVO
// ============================================================================
int main() {
    std::cout << "=============================================================================\n";
    std::cout << "BENCHMARK RECURSIVO VS ITERATIVO - PAA Assignment 1\n";
    std::cout << "=============================================================================\n\n";
    
    // Configuracoes
    const std::vector<int> scales = {100, 1000, 10000, 100000, 500000, 1000000, 5000000, 10000000, 25000000, 50000000};
    const Image queryPoint(999999, "query.jpg", 128, 128, 128);
    const double threshold = 50.0;
    
    std::cout << "Configuracao do Benchmark:\n";
    std::cout << "   Query Point: RGB(" << queryPoint.r << ", " << queryPoint.g << ", " << queryPoint.b << ")\n";
    std::cout << "   Threshold: " << threshold << "\n";
    std::cout << "   Escalas: 100 -> 50M imagens (10 escalas)\n";
    std::cout << "   Dados: SEED FIXA (42)\n";
    std::cout << "   Foco: Arvores Espaciais - Recursao vs Iteracao\n\n";
    
    // Coletar todos os resultados
    std::vector<BenchmarkResult> allResults;
    
    for (int scale : scales) {
        std::cout << "\n[TESTANDO] Escala: " << scale << " imagens...";
        
        // Testar 4 estruturas: Quad Rec, Quad Iter, Oct Rec, Oct Iter
        std::vector<std::function<std::unique_ptr<ImageDatabase>()>> structures = {
            []() { return std::make_unique<QuadtreeRecursivo>(); },
            []() { return std::make_unique<QuadtreeIterativo>(); },
            []() { return std::make_unique<OctreeRecursivo>(); },
            []() { return std::make_unique<OctreeIterativo>(); }
        };
        
        for (auto& structureFactory : structures) {
            // Gerar dataset fresco para cada estrutura
            auto dataset = generateSyntheticDataset(scale);
            auto structure = structureFactory();
            
            auto result = benchmarkStructure(std::move(structure), dataset, queryPoint, threshold);
            allResults.push_back(result);
        }
        std::cout << " OK\n";
    }
    
    // Mostrar tabela organizada
    std::cout << "\n=============================================================================\n";
    std::cout << "RESULTADOS FINAIS - RECURSAO VS ITERACAO\n";
    std::cout << "=============================================================================\n\n";
    
    // Cabecalho da tabela
    printf("%-10s %-20s %-12s %-12s %-8s\n", "Dataset", "Estrutura", "Insert(ms)", "Search(ms)", "Found");
    std::cout << "-------------------------------------------------------------------------\n";
    
    // Dados organizados por escala
    for (int scale : scales) {
        bool firstInScale = true;
        for (const auto& result : allResults) {
            if (result.datasetSize == scale) {
                if (firstInScale) {
                    printf("%-10d %-20s %-12.3f %-12.3f %-8d\n", 
                           result.datasetSize, result.structureName.c_str(), 
                           result.insertTime, result.searchTime, result.resultsFound);
                    firstInScale = false;
                } else {
                    printf("%-10s %-20s %-12.3f %-12.3f %-8d\n", 
                           "", result.structureName.c_str(), 
                           result.insertTime, result.searchTime, result.resultsFound);
                }
            }
        }
        std::cout << "-------------------------------------------------------------------------\n";
    }
    
    // Analise de vencedores 
    std::cout << "\nCOMPARACAO RECURSAO VS ITERACAO:\n";
    std::cout << "=============================================================================\n";
    
    for (int scale : scales) {
        std::cout << "\nEscala " << scale << ":\n";
        
        // Encontrar tempos para comparacao
        double quadRecInsert = 0, quadIterInsert = 0, quadRecSearch = 0, quadIterSearch = 0;
        double octRecInsert = 0, octIterInsert = 0, octRecSearch = 0, octIterSearch = 0;
        
        for (const auto& result : allResults) {
            if (result.datasetSize == scale) {
                if (result.structureName == "Quadtree Recursivo") {
                    quadRecInsert = result.insertTime; quadRecSearch = result.searchTime;
                } else if (result.structureName == "Quadtree Iterativo") {
                    quadIterInsert = result.insertTime; quadIterSearch = result.searchTime;
                } else if (result.structureName == "Octree Recursivo") {
                    octRecInsert = result.insertTime; octRecSearch = result.searchTime;
                } else if (result.structureName == "Octree Iterativo") {
                    octIterInsert = result.insertTime; octIterSearch = result.searchTime;
                }
            }
        }
        
        // Comparacoes
        printf("  Quadtree: Rec %.3fms vs Iter %.3fms (Insert) - ", quadRecInsert, quadIterInsert);
        printf("%s vence por %.1f%%\n", 
               quadRecInsert < quadIterInsert ? "RECURSIVO" : "Iterativo",
               100.0 * std::abs(quadRecInsert - quadIterInsert) / std::max(quadRecInsert, quadIterInsert));
               
        printf("  Octree:   Rec %.3fms vs Iter %.3fms (Insert) - ", octRecInsert, octIterInsert);
        printf("%s vence por %.1f%%\n", 
               octRecInsert < octIterInsert ? "RECURSIVO" : "Iterativo",
               100.0 * std::abs(octRecInsert - octIterInsert) / std::max(octRecInsert, octIterInsert));
    }
    
    std::cout << "\n=============================================================================\n";
    std::cout << "Benchmark Recursao vs Iteracao Concluido!\n";
    std::cout << "HIPOTESE: Recursao sempre vence devido a otimizacoes do compilador\n";
    std::cout << "=============================================================================\n";
    
    return 0;
}