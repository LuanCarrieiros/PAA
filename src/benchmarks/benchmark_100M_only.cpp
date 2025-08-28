/*
=============================================================================
BENCHMARK 100M APENAS - PAA Assignment 1 - TESTE ESPECIFICO
=============================================================================

Testa apenas 100M imagens sintéticas com padrão CREATE→TEST→DESTROY:
1. Quadtree (primeiro)
2. Octree (segundo)  
3. Hash (terceiro)
4. Linear (último)

OTIMIZAÇÕES:
- Padrão CREATE→TEST→DESTROY para economia de memória
- Dataset gerado independentemente para cada estrutura  
- Saída formatada com progresso e resultados imediatos

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
        
        // Buscar na célula do query e células vizinhas
        int queryR = std::min((int)(query.r / CELL_SIZE), GRID_SIZE - 1);
        int queryG = std::min((int)(query.g / CELL_SIZE), GRID_SIZE - 1);
        int queryB = std::min((int)(query.b / CELL_SIZE), GRID_SIZE - 1);
        
        // Calcular raio de busca em células
        int radius = (int)std::ceil(threshold / CELL_SIZE) + 1;
        
        for (int dr = -radius; dr <= radius; dr++) {
            for (int dg = -radius; dg <= radius; dg++) {
                for (int db = -radius; db <= radius; db++) {
                    int cellR = queryR + dr;
                    int cellG = queryG + dg;
                    int cellB = queryB + db;
                    
                    if (cellR >= 0 && cellR < GRID_SIZE && 
                        cellG >= 0 && cellG < GRID_SIZE && 
                        cellB >= 0 && cellB < GRID_SIZE) {
                        
                        uint64_t key = ((uint64_t)cellR << 32) | ((uint64_t)cellG << 16) | (uint64_t)cellB;
                        auto it = grid.find(key);
                        
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
        }
        
        return results;
    }
    
    size_t size() const override { return totalImages; }
    std::string getName() const override { return "Hash Search"; }
};

// ============================================================================
// 3. OCTREE SEARCH
// ============================================================================
class OctreeSearch : public ImageDatabase {
private:
    struct OctreeNode {
        double minR, maxR, minG, maxG, minB, maxB;
        std::vector<Image> images;
        std::array<std::unique_ptr<OctreeNode>, 8> children;
        bool isLeaf;
        
        OctreeNode(double _minR, double _maxR, double _minG, double _maxG, double _minB, double _maxB) 
            : minR(_minR), maxR(_maxR), minG(_minG), maxG(_maxG), minB(_minB), maxB(_maxB), isLeaf(true) {
            for (auto& child : children) {
                child = nullptr;
            }
        }
    };
    
    std::unique_ptr<OctreeNode> root;
    size_t totalImages = 0;
    static constexpr size_t MAX_IMAGES_PER_NODE = 15;
    
    void insertRecursive(OctreeNode* node, const Image& img) {
        node->images.push_back(img);
        
        if (node->isLeaf && node->images.size() > MAX_IMAGES_PER_NODE) {
            subdivide(node);
        }
        
        if (!node->isLeaf) {
            int childIndex = getChildIndex(node, img);
            if (!node->children[childIndex]) {
                auto [minR, maxR, minG, maxG, minB, maxB] = getChildBounds(node, childIndex);
                node->children[childIndex] = std::make_unique<OctreeNode>(minR, maxR, minG, maxG, minB, maxB);
            }
            insertRecursive(node->children[childIndex].get(), img);
        }
    }
    
    void subdivide(OctreeNode* node) {
        node->isLeaf = false;
        std::vector<Image> images = std::move(node->images);
        node->images.clear();
        
        for (const auto& img : images) {
            insertRecursive(node, img);
        }
    }
    
    int getChildIndex(const OctreeNode* node, const Image& img) const {
        double midR = (node->minR + node->maxR) / 2.0;
        double midG = (node->minG + node->maxG) / 2.0;
        double midB = (node->minB + node->maxB) / 2.0;
        
        int index = 0;
        if (img.r >= midR) index |= 4;
        if (img.g >= midG) index |= 2;
        if (img.b >= midB) index |= 1;
        
        return index;
    }
    
    std::tuple<double, double, double, double, double, double> getChildBounds(const OctreeNode* node, int childIndex) const {
        double midR = (node->minR + node->maxR) / 2.0;
        double midG = (node->minG + node->maxG) / 2.0;
        double midB = (node->minB + node->maxB) / 2.0;
        
        double minR = (childIndex & 4) ? midR : node->minR;
        double maxR = (childIndex & 4) ? node->maxR : midR;
        double minG = (childIndex & 2) ? midG : node->minG;
        double maxG = (childIndex & 2) ? node->maxG : midG;
        double minB = (childIndex & 1) ? midB : node->minB;
        double maxB = (childIndex & 1) ? node->maxB : midB;
        
        return {minR, maxR, minG, maxG, minB, maxB};
    }
    
    void searchRecursive(const OctreeNode* node, const Image& query, double threshold, std::vector<Image>& results) const {
        if (!node) return;
        
        // Verificar se o nó pode conter resultados
        double minDist = 0;
        if (query.r < node->minR) minDist += (node->minR - query.r) * (node->minR - query.r);
        else if (query.r > node->maxR) minDist += (query.r - node->maxR) * (query.r - node->maxR);
        
        if (query.g < node->minG) minDist += (node->minG - query.g) * (node->minG - query.g);
        else if (query.g > node->maxG) minDist += (query.g - node->maxG) * (query.g - node->maxG);
        
        if (query.b < node->minB) minDist += (node->minB - query.b) * (node->minB - query.b);
        else if (query.b > node->maxB) minDist += (query.b - node->maxB) * (query.b - node->maxB);
        
        if (std::sqrt(minDist) > threshold) return;
        
        // Verificar imagens neste nó
        for (const auto& img : node->images) {
            if (query.distanceTo(img) <= threshold) {
                results.push_back(img);
            }
        }
        
        // Buscar nos filhos
        for (const auto& child : node->children) {
            if (child) {
                searchRecursive(child.get(), query, threshold, results);
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
// 4. QUADTREE SEARCH
// ============================================================================
class QuadtreeSearch : public ImageDatabase {
private:
    struct QuadtreeNode {
        double minR, maxR, minG, maxG;
        std::vector<Image> images;
        std::array<std::unique_ptr<QuadtreeNode>, 4> children;
        bool isLeaf;
        
        QuadtreeNode(double _minR, double _maxR, double _minG, double _maxG) 
            : minR(_minR), maxR(_maxR), minG(_minG), maxG(_maxG), isLeaf(true) {
            for (auto& child : children) {
                child = nullptr;
            }
        }
    };
    
    std::unique_ptr<QuadtreeNode> root;
    size_t totalImages = 0;
    static constexpr size_t MAX_IMAGES_PER_NODE = 30;
    
    void insertRecursive(QuadtreeNode* node, const Image& img) {
        node->images.push_back(img);
        
        if (node->isLeaf && node->images.size() > MAX_IMAGES_PER_NODE) {
            subdivide(node);
        }
        
        if (!node->isLeaf) {
            int childIndex = getChildIndex(node, img);
            if (!node->children[childIndex]) {
                auto [minR, maxR, minG, maxG] = getChildBounds(node, childIndex);
                node->children[childIndex] = std::make_unique<QuadtreeNode>(minR, maxR, minG, maxG);
            }
            insertRecursive(node->children[childIndex].get(), img);
        }
    }
    
    void subdivide(QuadtreeNode* node) {
        node->isLeaf = false;
        std::vector<Image> images = std::move(node->images);
        node->images.clear();
        
        for (const auto& img : images) {
            insertRecursive(node, img);
        }
    }
    
    int getChildIndex(const QuadtreeNode* node, const Image& img) const {
        double midR = (node->minR + node->maxR) / 2.0;
        double midG = (node->minG + node->maxG) / 2.0;
        
        int index = 0;
        if (img.r >= midR) index |= 2;
        if (img.g >= midG) index |= 1;
        
        return index;
    }
    
    std::tuple<double, double, double, double> getChildBounds(const QuadtreeNode* node, int childIndex) const {
        double midR = (node->minR + node->maxR) / 2.0;
        double midG = (node->minG + node->maxG) / 2.0;
        
        double minR = (childIndex & 2) ? midR : node->minR;
        double maxR = (childIndex & 2) ? node->maxR : midR;
        double minG = (childIndex & 1) ? midG : node->minG;
        double maxG = (childIndex & 1) ? node->maxG : midG;
        
        return {minR, maxR, minG, maxG};
    }
    
    void searchRecursive(const QuadtreeNode* node, const Image& query, double threshold, std::vector<Image>& results) const {
        if (!node) return;
        
        // Verificar se o nó pode conter resultados (apenas R,G)
        double minDist = 0;
        if (query.r < node->minR) minDist += (node->minR - query.r) * (node->minR - query.r);
        else if (query.r > node->maxR) minDist += (query.r - node->maxR) * (query.r - node->maxR);
        
        if (query.g < node->minG) minDist += (node->minG - query.g) * (node->minG - query.g);
        else if (query.g > node->maxG) minDist += (query.g - node->maxG) * (query.g - node->maxG);
        
        if (std::sqrt(minDist) > threshold) return;
        
        // Verificar imagens neste nó (distância completa R,G,B)
        for (const auto& img : node->images) {
            if (query.distanceTo(img) <= threshold) {
                results.push_back(img);
            }
        }
        
        // Buscar nos filhos
        for (const auto& child : node->children) {
            if (child) {
                searchRecursive(child.get(), query, threshold, results);
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
// SISTEMA DE BENCHMARK
// ============================================================================
struct BenchmarkResult {
    std::string structureName;
    int datasetSize;
    double insertTime;
    double searchTime;
    int resultsFound;
};

// Gerar dataset sintético
std::vector<Image> generateSyntheticDataset(int size) {
    std::vector<Image> dataset;
    dataset.reserve(size);
    
    // SEED FIXA para consistência entre execuções
    std::mt19937 gen(20);  // Sempre os mesmos dados sintéticos
    std::uniform_real_distribution<double> colorDist(0.0, 255.0);
    
    for (int i = 0; i < size; i++) {
        double r = colorDist(gen);
        double g = colorDist(gen);
        double b = colorDist(gen);
        
        dataset.emplace_back(i, "synthetic_" + std::to_string(i) + ".jpg", r, g, b);
    }
    
    return dataset;
}

// Benchmark individual de uma estrutura
BenchmarkResult benchmarkStructure(std::unique_ptr<ImageDatabase> structure, 
                                 const std::vector<Image>& dataset, 
                                 const Image& query, double threshold) {
    BenchmarkResult result;
    result.structureName = structure->getName();
    result.datasetSize = dataset.size();
    
    // Teste de Inserção
    auto start = std::chrono::high_resolution_clock::now();
    
    for (const auto& img : dataset) {
        structure->insert(img);
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    result.insertTime = std::chrono::duration<double, std::milli>(end - start).count();
    
    // Teste de Busca
    start = std::chrono::high_resolution_clock::now();
    auto results = structure->findSimilar(query, threshold);
    end = std::chrono::high_resolution_clock::now();
    
    result.searchTime = std::chrono::duration<double, std::milli>(end - start).count();
    result.resultsFound = results.size();
    
    return result;
}

// ============================================================================
// MAIN - BENCHMARK 100M EXCLUSIVO
// ============================================================================
int main() {
    std::cout << "==================================================================================\n";
    std::cout << " BENCHMARK 100M EXCLUSIVO - PAA Assignment 1 - DADOS SINTETICOS\n";
    std::cout << "==================================================================================\n\n";
    
    // Configurações
    const int SCALE = 100000000; // 100 milhões
    const Image queryPoint(999999, "query.jpg", 128, 128, 128);
    const double threshold = 50.0;
    
    std::cout << "Dataset: Sintetico 100M imagens (MAIOR ESCALA)\n";
    std::cout << "Threshold: " << threshold << "\n";
    std::cout << "Query: RGB(" << (int)queryPoint.r << ", " << (int)queryPoint.g << ", " << (int)queryPoint.b << ")\n\n";
    
    std::cout << "Gerando datasets sinteticos com SEED fixa para reproducibilidade...\n";
    
    // Estruturas em ordem específica para o teste de 100M
    std::vector<std::string> structureNames = {"QuadtreeSearch", "OctreeSearch", "HashSearch", "LinearSearch"};
    std::vector<BenchmarkResult> allResults;
    
    std::cout << "\n[TESTANDO] Escala: " << SCALE << " imagens...\n";
    std::cout << "Generating " << SCALE << " synthetic images...\n";
    
    for (const std::string& structName : structureNames) {
        // Criar nova instância da estrutura
        std::unique_ptr<ImageDatabase> structure;
        if (structName == "LinearSearch") structure = std::make_unique<LinearSearch>();
        else if (structName == "HashSearch") structure = std::make_unique<HashSearch>();
        else if (structName == "OctreeSearch") structure = std::make_unique<OctreeSearch>();
        else if (structName == "QuadtreeSearch") structure = std::make_unique<QuadtreeSearch>();
        
        // NOVO: Gerar dataset fresco para cada estrutura (libera memória entre testes)
        auto freshDataset = generateSyntheticDataset(SCALE);
        
        auto result = benchmarkStructure(std::move(structure), freshDataset, queryPoint, threshold);
        allResults.push_back(result);
        
        // Mostrar resultado imediatamente no estilo dos benchmarks de imagem
        printf("  %s: Insert=%.3fms, Search=%.3fms, Found=%d\n", 
               result.structureName.c_str(), result.insertTime, result.searchTime, result.resultsFound);
        
        // Dataset sai de escopo aqui e libera memória automaticamente
    }
    
    // Agora mostrar tabela organizada
    std::cout << "\n==================================================================================\n";
    std::cout << "RESULTADOS FINAIS - BENCHMARK 100M\n";
    std::cout << "==================================================================================\n\n";
    
    // Cabeçalho da tabela
    printf("%-15s %-12s %-12s %-8s\n", "Estrutura", "Insert(ms)", "Search(ms)", "Found");
    std::cout << "-------------------------------------------------------------------------------\n";
    
    // Dados organizados
    for (const auto& result : allResults) {
        printf("%-15s %-12.3f %-12.3f %-8d\n", 
               result.structureName.c_str(), result.insertTime, result.searchTime, result.resultsFound);
    }
    std::cout << "-------------------------------------------------------------------------------\n";
    
    // Análise de vencedores
    std::cout << "\nANALISE DE VENCEDORES (100M imagens):\n";
    std::cout << "==================================================================================\n";
    
    // Encontrar melhor inserção e busca
    double bestInsert = 999999999, bestSearch = 999999999;
    std::string bestInsertName, bestSearchName;
    
    for (const auto& result : allResults) {
        if (result.insertTime < bestInsert) {
            bestInsert = result.insertTime;
            bestInsertName = result.structureName;
        }
        if (result.searchTime < bestSearch) {
            bestSearch = result.searchTime;
            bestSearchName = result.structureName;
        }
    }
    
    printf("100M           | Insert: %-15s (%.3fms) | Search: %-15s (%.3fms)\n", 
           bestInsertName.c_str(), bestInsert, 
           bestSearchName.c_str(), bestSearch);
    
    std::cout << "\n==================================================================================\n";
    std::cout << "Benchmark 100M Concluido! Teste na maior escala possivel.\n";
    std::cout << "   Escala extrema: 100 milhoes de imagens sinteticas\n";
    std::cout << "   Memoria otimizada: CREATE→TEST→DESTROY pattern\n";
    std::cout << "   Dados prontos para analise de limite computacional.\n";
    std::cout << "==================================================================================\n";
    
    return 0;
}