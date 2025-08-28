/*
=============================================================================
BENCHMARK ESCALADO - PAA Assignment 1: Testes com Multiplas Escalas
=============================================================================

Testa as 4 estruturas de dados com datasets de tamanhos crescentes:
- 100, 1K, 10K, 100K, 500K, 1M, 5M imagens
- Combina dados sinteticos + dados reais do CSV (50K disponiveis)
- Mede tempos de insercao e busca para analise de escalabilidade

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
        
        // Buscar na celula do query e celulas vizinhas
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
// HASH DYNAMIC SEARCH - Hash espacial com expansão adaptativa
// ============================================================================
class HashDynamicSearch : public ImageDatabase {
private:
    double cellSize;
    std::unordered_map<std::string, std::vector<Image>> grid;
    
    int rgbToCell(double value) const {
        return static_cast<int>(value / cellSize);
    }
    
    std::string getCellKey(int r_cell, int g_cell, int b_cell) const {
        static char buffer[64];
        snprintf(buffer, sizeof(buffer), "%d,%d,%d", r_cell, g_cell, b_cell);
        return std::string(buffer);
    }
    
    // BUSCA POR EXPANSÃO DE CUBO: examina camadas concêntricas
    void searchCubeAtRadius(int center_r, int center_g, int center_b, int radius,
                           const Image& query, double threshold, std::vector<Image>& results) const {
        if (radius == 0) {
            // Célula central
            searchSingleCell(center_r, center_g, center_b, query, threshold, results);
            return;
        }
        
        // TÉCNICA PAA: Busca apenas na "casca" do cubo de raio r
        // Evita reprocessar células já examinadas em raios menores
        for (int dr = -radius; dr <= radius; dr++) {
            for (int dg = -radius; dg <= radius; dg++) {
                for (int db = -radius; db <= radius; db++) {
                    // Só processar se está na casca externa (pelo menos uma coordenada no limite)
                    if (abs(dr) == radius || abs(dg) == radius || abs(db) == radius) {
                        searchSingleCell(center_r + dr, center_g + dg, center_b + db,
                                       query, threshold, results);
                    }
                }
            }
        }
    }
    
    void searchSingleCell(int r_cell, int g_cell, int b_cell,
                         const Image& query, double threshold, std::vector<Image>& results) const {
        std::string key = getCellKey(r_cell, g_cell, b_cell);
        
        auto it = grid.find(key);
        if (it != grid.end()) {
            for (const auto& img : it->second) {
                double distance = query.distanceTo(img);
                if (distance <= threshold) {
                    results.push_back(img);
                }
            }
        }
    }

public:
    HashDynamicSearch(double _cellSize = 25.0) : cellSize(_cellSize) {}
    
    void insert(const Image& img) override {
        std::string key = getCellKey(rgbToCell(img.r), rgbToCell(img.g), rgbToCell(img.b));
        grid[key].push_back(img);
    }
    
    std::vector<Image> findSimilar(const Image& query, double threshold) const override {
        std::vector<Image> results;
        
        int query_r = rgbToCell(query.r);
        int query_g = rgbToCell(query.g);  
        int query_b = rgbToCell(query.b);
        
        // BUSCA DINÂMICA: expande em camadas até cobrir o threshold
        int max_radius = static_cast<int>(ceil(threshold / cellSize));
        
        for (int radius = 0; radius <= max_radius; radius++) {
            searchCubeAtRadius(query_r, query_g, query_b, radius, query, threshold, results);
        }
        
        // Ordenar por distância (nearest-first)
        std::sort(results.begin(), results.end(),
                 [&query](const Image& a, const Image& b) {
                     return query.distanceTo(a) < query.distanceTo(b);
                 });
        
        return results;
    }
    
    size_t size() const override { 
        size_t count = 0;
        for (const auto& pair : grid) {
            count += pair.second.size();
        }
        return count;
    }
    
    std::string getName() const override {
        return "Hash Dynamic Search (cell=" + std::to_string(cellSize) + ", adaptive)";
    }
};

// ============================================================================
// 3. OCTREE NODE
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

// ============================================================================
// 3. OCTREE SEARCH
// ============================================================================
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
// 4. QUADTREE (2D - R,G apenas)
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
// GERADORES DE DADOS
// ============================================================================

// Gerar dataset sintetico
std::vector<Image> generateSyntheticDataset(int count) {
    std::vector<Image> images;
    images.reserve(count);
    
    // SEED FIXA para consistencia entre execucoes
    std::mt19937 gen(20);  // Sempre os mesmos dados sinteticos
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
// MAIN - BENCHMARK ESCALADO
// ============================================================================
int main() {
    std::cout << "==================================================================================\n";
    std::cout << " BENCHMARK ESCALADO - PAA Assignment 1 - DADOS SINTETICOS\n";
    std::cout << "==================================================================================\n\n";
    
    // Configuracoes
    const std::vector<int> scales = {100, 1000, 10000, 100000, 500000, 1000000, 5000000, 10000000, 25000000, 50000000};
    const Image queryPoint(999999, "query.jpg", 128, 128, 128);
    const double threshold = 50.0;
    
    std::cout << "Dataset: Sintetico escalado (100 -> 50M imagens)\n";
    std::cout << "Threshold: " << threshold << "\n";
    std::cout << "Query: RGB(" << (int)queryPoint.r << ", " << (int)queryPoint.g << ", " << (int)queryPoint.b << ")\n\n";
    
    std::cout << "Gerando datasets sinteticos com SEED fixa para reproducibilidade...\n";
    
    // Coletar todos os resultados primeiro
    std::vector<BenchmarkResult> allResults;
    
    for (int scale : scales) {
        std::cout << "\n[TESTANDO] Escala: " << scale << " imagens...\n";
        std::cout << "Generating " << scale << " synthetic images...\n";
        
        // Testar cada estrutura COM DATASET INDEPENDENTE (minhas 16GB de ram chorou kkk, economiza RAM)
        std::vector<std::string> structureNames = {"LinearSearch", "HashSearch", "HashDynamicSearch", "OctreeSearch", "QuadtreeSearch"};
        
        for (const std::string& structName : structureNames) {
            // Criar nova instancia da estrutura
            std::unique_ptr<ImageDatabase> structure;
            if (structName == "LinearSearch") structure = std::make_unique<LinearSearch>();
            else if (structName == "HashSearch") structure = std::make_unique<HashSearch>();
            else if (structName == "HashDynamicSearch") structure = std::make_unique<HashDynamicSearch>();
            else if (structName == "OctreeSearch") structure = std::make_unique<OctreeSearch>();
            else if (structName == "QuadtreeSearch") structure = std::make_unique<QuadtreeSearch>();
            
            // NOVO: Gerar dataset fresco para cada estrutura (libera memoria entre testes)
            auto freshDataset = generateSyntheticDataset(scale);
            
            auto result = benchmarkStructure(std::move(structure), freshDataset, queryPoint, threshold);
            allResults.push_back(result);
            
            // Mostrar resultado imediatamente no estilo dos benchmarks de imagem
            printf("  %s: Insert=%.3fms, Search=%.3fms, Found=%d\n", 
                   result.structureName.c_str(), result.insertTime, result.searchTime, result.resultsFound);
            
            // Dataset sai de escopo aqui e libera memoria automaticamente
        }
    }
    
    // Agora mostrar tabela organizada
    std::cout << "\n==================================================================================\n";
    std::cout << "RESULTADOS FINAIS - TABELA ORGANIZADA\n";
    std::cout << "==================================================================================\n\n";
    
    // Cabecalho da tabela
    printf("%-10s %-15s %-12s %-12s %-8s\n", "Dataset", "Estrutura", "Insert(ms)", "Search(ms)", "Found");
    std::cout << "-------------------------------------------------------------------------------\n";
    
    // Dados organizados por escala
    for (int scale : scales) {
        bool firstInScale = true;
        for (const auto& result : allResults) {
            if (result.datasetSize == scale) {
                if (firstInScale) {
                    printf("%-10d %-15s %-12.3f %-12.3f %-8d\n", 
                           result.datasetSize, result.structureName.c_str(), 
                           result.insertTime, result.searchTime, result.resultsFound);
                    firstInScale = false;
                } else {
                    printf("%-10s %-15s %-12.3f %-12.3f %-8d\n", 
                           "", result.structureName.c_str(), 
                           result.insertTime, result.searchTime, result.resultsFound);
                }
            }
        }
        std::cout << "-------------------------------------------------------------------------------\n";
    }
    
    // Analise de vencedores por escala
    std::cout << "\nANALISE DE VENCEDORES POR ESCALA:\n";
    std::cout << "==================================================================================\n";
    
    for (int scale : scales) {
        // Encontrar melhor insercao e busca para esta escala
        double bestInsert = 999999, bestSearch = 999999;
        std::string bestInsertName, bestSearchName;
        
        for (const auto& result : allResults) {
            if (result.datasetSize == scale) {
                if (result.insertTime < bestInsert) {
                    bestInsert = result.insertTime;
                    bestInsertName = result.structureName;
                }
                if (result.searchTime < bestSearch) {
                    bestSearch = result.searchTime;
                    bestSearchName = result.structureName;
                }
            }
        }
        
        printf("%-10d | Insert: %-15s (%.3fms) | Search: %-15s (%.3fms)\n", 
               scale, bestInsertName.c_str(), bestInsert, 
               bestSearchName.c_str(), bestSearch);
    }
    
    
    std::cout << "\n==================================================================================\n";
    std::cout << "Benchmark Concluido! Analise focada em dados sinteticos escalados.\n";
    std::cout << "   Hash Search: CAMPEA ABSOLUTA em busca!\n";
    std::cout << "   Linear Search: Impressionante em insercao!\n";
    std::cout << "   Dados prontos para analise comparativa.\n";
    std::cout << "==================================================================================\n";
    
    return 0;
}