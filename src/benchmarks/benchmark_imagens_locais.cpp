#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <string>
#include <fstream>
#include <random>
#include <algorithm>
#include <functional>
#include <array>
#include <unordered_map>
#include <filesystem>
#include <iomanip>

#define STB_IMAGE_IMPLEMENTATION
#include "../headers/stb_image.h"

struct Image {
    int r, g, b;
    std::string filename;
    std::string category;
    
    Image(int r, int g, int b, const std::string& filename = "", const std::string& category = "") 
        : r(r), g(g), b(b), filename(filename), category(category) {}
    
    double distanceTo(const Image& other) const {
        double dr = r - other.r;
        double dg = g - other.g;
        double db = b - other.b;
        return std::sqrt(dr*dr + dg*dg + db*db);
    }
};

class ImageDatabase {
public:
    virtual ~ImageDatabase() = default;
    virtual void insert(const Image& image) = 0;
    virtual std::vector<Image> search(const Image& query, double threshold) = 0;
    virtual void clear() = 0;
    virtual size_t size() const = 0;
    virtual std::string getName() const = 0;
};

class LinearSearch : public ImageDatabase {
private:
    std::vector<Image> images;

public:
    void insert(const Image& image) override {
        images.push_back(image);
    }
    
    std::vector<Image> search(const Image& query, double threshold) override {
        std::vector<Image> results;
        for (const auto& img : images) {
            if (img.distanceTo(query) <= threshold) {
                results.push_back(img);
            }
        }
        return results;
    }
    
    void clear() override { images.clear(); }
    size_t size() const override { return images.size(); }
    std::string getName() const override { return "Linear Search"; }
};

class HashSearch : public ImageDatabase {
private:
    static const int GRID_SIZE = 32;
    std::unordered_map<int, std::vector<Image>> hashGrid;
    
    int hashFunction(int r, int g, int b) {
        int cellR = r / GRID_SIZE;
        int cellG = g / GRID_SIZE;
        int cellB = b / GRID_SIZE;
        return cellR * 64 * 64 + cellG * 64 + cellB;
    }

public:
    void insert(const Image& image) override {
        int hash = hashFunction(image.r, image.g, image.b);
        hashGrid[hash].push_back(image);
    }
    
    std::vector<Image> search(const Image& query, double threshold) override {
        std::vector<Image> results;
        int queryHash = hashFunction(query.r, query.g, query.b);
        
        for (int dr = -1; dr <= 1; dr++) {
            for (int dg = -1; dg <= 1; dg++) {
                for (int db = -1; db <= 1; db++) {
                    int cellR = query.r / GRID_SIZE + dr;
                    int cellG = query.g / GRID_SIZE + dg;
                    int cellB = query.b / GRID_SIZE + db;
                    
                    if (cellR >= 0 && cellG >= 0 && cellB >= 0) {
                        int neighborHash = cellR * 64 * 64 + cellG * 64 + cellB;
                        auto it = hashGrid.find(neighborHash);
                        if (it != hashGrid.end()) {
                            for (const auto& img : it->second) {
                                if (img.distanceTo(query) <= threshold) {
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
    
    void clear() override { hashGrid.clear(); }
    size_t size() const override {
        size_t total = 0;
        for (const auto& bucket : hashGrid) {
            total += bucket.second.size();
        }
        return total;
    }
    std::string getName() const override { return "Hash Search"; }
};

struct OctreeNode {
    Image* image;
    OctreeNode* children[8];
    int minR, maxR, minG, maxG, minB, maxB;
    int depth;
    
    OctreeNode(int minR, int maxR, int minG, int maxG, int minB, int maxB, int depth = 0) 
        : image(nullptr), minR(minR), maxR(maxR), minG(minG), maxG(maxG), minB(minB), maxB(maxB), depth(depth) {
        for (int i = 0; i < 8; i++) {
            children[i] = nullptr;
        }
    }
    
    ~OctreeNode() {
        for (int i = 0; i < 8; i++) {
            delete children[i];
        }
    }
    
    int getChildIndex(const Image& img) {
        int midR = (minR + maxR) / 2;
        int midG = (minG + maxG) / 2;
        int midB = (minB + maxB) / 2;
        
        int index = 0;
        if (img.r > midR) index |= 4;
        if (img.g > midG) index |= 2;
        if (img.b > midB) index |= 1;
        return index;
    }
};

class OctreeSearch : public ImageDatabase {
private:
    OctreeNode* root;
    size_t imageCount;
    std::vector<Image> imageStorage;
    
    void insertRecursive(OctreeNode* node, Image* img) {
        // THEORETICAL: Ideal implementation (may cause memory issues with large datasets)
        if (node->maxR - node->minR <= 1 && node->maxG - node->minG <= 1 && node->maxB - node->minB <= 1) {
            if (!node->image) {
                node->image = img;
            }
            return;
        }
        
        int childIndex = node->getChildIndex(*img);
        if (!node->children[childIndex]) {
            int midR = (node->minR + node->maxR) / 2;
            int midG = (node->minG + node->maxG) / 2;
            int midB = (node->minB + node->maxB) / 2;
            
            int newMinR = (childIndex & 4) ? midR + 1 : node->minR;
            int newMaxR = (childIndex & 4) ? node->maxR : midR;
            int newMinG = (childIndex & 2) ? midG + 1 : node->minG;
            int newMaxG = (childIndex & 2) ? node->maxG : midG;
            int newMinB = (childIndex & 1) ? midB + 1 : node->minB;
            int newMaxB = (childIndex & 1) ? node->maxB : midB;
            
            node->children[childIndex] = new OctreeNode(newMinR, newMaxR, newMinG, newMaxG, newMinB, newMaxB);
        }
        
        insertRecursive(node->children[childIndex], img);
    }
    
    void searchRecursive(OctreeNode* node, const Image& query, double threshold, std::vector<Image>& results) {
        if (!node) return;
        
        if (node->image && node->image->distanceTo(query) <= threshold) {
            results.push_back(*node->image);
        }
        
        for (int i = 0; i < 8; i++) {
            if (node->children[i]) {
                searchRecursive(node->children[i], query, threshold, results);
            }
        }
    }

public:
    OctreeSearch() : root(new OctreeNode(0, 255, 0, 255, 0, 255)), imageCount(0) {}
    
    ~OctreeSearch() {
        delete root;
    }
    
    void insert(const Image& image) override {
        imageStorage.push_back(image);
        insertRecursive(root, &imageStorage.back());
        imageCount++;
    }
    
    std::vector<Image> search(const Image& query, double threshold) override {
        std::vector<Image> results;
        searchRecursive(root, query, threshold, results);
        return results;
    }
    
    void clear() override {
        delete root;
        root = new OctreeNode(0, 255, 0, 255, 0, 255);
        imageStorage.clear();
        imageCount = 0;
    }
    
    size_t size() const override { return imageCount; }
    std::string getName() const override { return "Octree Search"; }
};

struct QuadtreeNode {
    Image* image;
    QuadtreeNode* children[4];
    int minR, maxR, minG, maxG;
    int depth;
    
    QuadtreeNode(int minR, int maxR, int minG, int maxG, int depth = 0) 
        : image(nullptr), minR(minR), maxR(maxR), minG(minG), maxG(maxG), depth(depth) {
        for (int i = 0; i < 4; i++) {
            children[i] = nullptr;
        }
    }
    
    ~QuadtreeNode() {
        for (int i = 0; i < 4; i++) {
            delete children[i];
        }
    }
    
    int getChildIndex(const Image& img) {
        int midR = (minR + maxR) / 2;
        int midG = (minG + maxG) / 2;
        
        int index = 0;
        if (img.r > midR) index |= 2;
        if (img.g > midG) index |= 1;
        return index;
    }
};

class QuadtreeSearch : public ImageDatabase {
private:
    QuadtreeNode* root;
    size_t imageCount;
    std::vector<Image> imageStorage;
    
    void insertRecursive(QuadtreeNode* node, Image* img) {
        // THEORETICAL: Ideal implementation (may cause memory issues with large datasets)
        if ((node->maxR - node->minR <= 1 && node->maxG - node->minG <= 1) ||
            (node->maxR <= node->minR || node->maxG <= node->minG)) {
            if (!node->image) {
                node->image = img;
            }
            return;
        }
        
        int childIndex = node->getChildIndex(*img);
        if (!node->children[childIndex]) {
            int midR = (node->minR + node->maxR) / 2;
            int midG = (node->minG + node->maxG) / 2;
            
            int newMinR = (childIndex & 2) ? midR + 1 : node->minR;  // Back to +1 for precision
            int newMaxR = (childIndex & 2) ? node->maxR : midR;
            int newMinG = (childIndex & 1) ? midG + 1 : node->minG;  // Back to +1 for precision
            int newMaxG = (childIndex & 1) ? node->maxG : midG;
            
            // Safety check to avoid invalid ranges
            if (newMinR >= newMaxR || newMinG >= newMaxG) {
                if (!node->image) {
                    node->image = img;
                }
                return;
            }
            
            node->children[childIndex] = new QuadtreeNode(newMinR, newMaxR, newMinG, newMaxG);
        }
        
        insertRecursive(node->children[childIndex], img);
    }
    
    void searchRecursive(QuadtreeNode* node, const Image& query, double threshold, std::vector<Image>& results) {
        if (!node) return;
        
        if (node->image && node->image->distanceTo(query) <= threshold) {
            results.push_back(*node->image);
        }
        
        for (int i = 0; i < 4; i++) {
            if (node->children[i]) {
                searchRecursive(node->children[i], query, threshold, results);
            }
        }
    }

public:
    QuadtreeSearch() : root(new QuadtreeNode(0, 255, 0, 255)), imageCount(0) {}
    
    ~QuadtreeSearch() {
        delete root;
    }
    
    void insert(const Image& image) override {
        imageStorage.push_back(image);
        insertRecursive(root, &imageStorage.back());
        imageCount++;
    }
    
    std::vector<Image> search(const Image& query, double threshold) override {
        std::vector<Image> results;
        searchRecursive(root, query, threshold, results);
        return results;
    }
    
    void clear() override {
        delete root;
        root = new QuadtreeNode(0, 255, 0, 255);
        imageStorage.clear();
        imageCount = 0;
    }
    
    size_t size() const override { return imageCount; }
    std::string getName() const override { return "Quadtree Search"; }
};

// Dynamic Hash Search com busca por expansão de cubo  
class HashSearchDynamic : public ImageDatabase {
private:
    static const int CELL_SIZE = 32;
    std::unordered_map<std::string, std::vector<Image>> hashGrid;
    
    int rgbToCell(int value) {
        return value / CELL_SIZE;
    }
    
    std::string getCellKey(int r_cell, int g_cell, int b_cell) {
        // Simple hash key to avoid string creation issues
        static char buffer[64];
        snprintf(buffer, sizeof(buffer), "%d,%d,%d", r_cell, g_cell, b_cell);
        return std::string(buffer);
    }
    
    void searchCubeAtRadius(int center_r, int center_g, int center_b, int radius, 
                           const Image& query, double threshold, std::vector<Image>& results) {
        if (radius == 0) {
            searchSingleCell(center_r, center_g, center_b, query, threshold, results);
            return;
        }
        
        for (int dr = -radius; dr <= radius; dr++) {
            for (int dg = -radius; dg <= radius; dg++) {
                for (int db = -radius; db <= radius; db++) {
                    if (abs(dr) == radius || abs(dg) == radius || abs(db) == radius) {
                        searchSingleCell(center_r + dr, center_g + dg, center_b + db, 
                                       query, threshold, results);
                    }
                }
            }
        }
    }
    
    void searchSingleCell(int r_cell, int g_cell, int b_cell, 
                         const Image& query, double threshold, std::vector<Image>& results) {
        std::string key = getCellKey(r_cell, g_cell, b_cell);
        
        auto it = hashGrid.find(key);
        if (it != hashGrid.end()) {
            for (const auto& img : it->second) {
                double distance = query.distanceTo(img);
                if (distance <= threshold) {
                    results.push_back(img);
                }
            }
        }
    }

public:
    void insert(const Image& image) override {
        int r_cell = rgbToCell(image.r);
        int g_cell = rgbToCell(image.g);
        int b_cell = rgbToCell(image.b);
        std::string key = getCellKey(r_cell, g_cell, b_cell);
        hashGrid[key].push_back(image);
    }
    
    std::vector<Image> search(const Image& query, double threshold) override {
        std::vector<Image> results;
        
        int query_r = rgbToCell(query.r);
        int query_g = rgbToCell(query.g);
        int query_b = rgbToCell(query.b);
        
        int max_radius = static_cast<int>(ceil(threshold / CELL_SIZE));
        
        // Dynamic search: expand from query cell outward
        for (int radius = 0; radius <= max_radius; radius++) {
            searchCubeAtRadius(query_r, query_g, query_b, radius, query, threshold, results);
        }
        
        std::sort(results.begin(), results.end(), 
                 [&query](const Image& a, const Image& b) {
                     return query.distanceTo(a) < query.distanceTo(b);
                 });
        
        return results;
    }
    
    void clear() override { hashGrid.clear(); }
    size_t size() const override {
        size_t total = 0;
        for (const auto& bucket : hashGrid) {
            total += bucket.second.size();
        }
        return total;
    }
    std::string getName() const override { return "Hash Dynamic Search"; }
};

class RealImageLoader {
private:
    std::string baseDir;
    std::vector<std::string> categories;
    std::mt19937 rng;

public:
    RealImageLoader(const std::string& dir) : baseDir(dir), rng(42) {
        // No categories needed - just load all images
    }
    
    Image extractRGBFromFile(const std::string& filepath, const std::string& category) {
        int width, height, channels;
        unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 3);
        
        if (!data) {
            std::cerr << "Error loading image: " << filepath << std::endl;
            return Image(128, 128, 128, filepath, category);
        }
        
        long long totalR = 0, totalG = 0, totalB = 0;
        int pixelCount = width * height;
        
        for (int i = 0; i < pixelCount * 3; i += 3) {
            totalR += data[i];
            totalG += data[i + 1];
            totalB += data[i + 2];
        }
        
        stbi_image_free(data);
        
        int avgR = static_cast<int>(totalR / pixelCount);
        int avgG = static_cast<int>(totalG / pixelCount);
        int avgB = static_cast<int>(totalB / pixelCount);
        
        return Image(avgR, avgG, avgB, filepath, category);
    }
    
    std::vector<Image> loadImages(int maxImages = -1) {
        std::vector<Image> images;
        std::vector<std::string> allFiles;
        
        // Load all .jpg and .png files
        namespace fs = std::filesystem;
        for (const auto& entry : fs::directory_iterator(baseDir)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename.find(".jpg") != std::string::npos || filename.find(".png") != std::string::npos) {
                    // Extract category from filename (e.g., "car_0001.jpg" -> "car")
                    std::string category = "image";
                    size_t underscorePos = filename.find('_');
                    if (underscorePos != std::string::npos) {
                        category = filename.substr(0, underscorePos);
                    }
                    allFiles.push_back(entry.path().string() + ":" + category);
                }
            }
        }
        
        if (maxImages > 0 && allFiles.size() > static_cast<size_t>(maxImages)) {
            std::shuffle(allFiles.begin(), allFiles.end(), rng);
            allFiles.resize(maxImages);
        }
        
        std::cout << "Loading " << allFiles.size() << " images..." << std::endl;
        
        for (size_t i = 0; i < allFiles.size(); i++) {
            if (i % 500 == 0) {
                std::cout << "Progress: " << i << "/" << allFiles.size() << std::endl;
            }
            
            size_t colonPos = allFiles[i].find(':');
            std::string filepath = allFiles[i].substr(0, colonPos);
            std::string category = allFiles[i].substr(colonPos + 1);
            
            Image img = extractRGBFromFile(filepath, category);
            images.push_back(img);
        }
        
        std::cout << "Loaded " << images.size() << " images successfully!" << std::endl;
        return images;
    }
    
    Image getRandomQueryImage() {
        std::vector<std::string> allFiles;
        namespace fs = std::filesystem;
        for (const auto& entry : fs::directory_iterator(baseDir)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename.find(".jpg") != std::string::npos || filename.find(".png") != std::string::npos) {
                    allFiles.push_back(entry.path().string());
                }
            }
        }
        
        if (allFiles.empty()) {
            return Image(128, 128, 128, "fallback", "image");
        }
        
        std::uniform_int_distribution<int> fileDist(0, allFiles.size() - 1);
        std::string selectedFile = allFiles[fileDist(rng)];
        
        // Extract category from filename
        std::string category = "image";
        std::string filename = selectedFile.substr(selectedFile.find_last_of("/\\") + 1);
        size_t underscorePos = filename.find('_');
        if (underscorePos != std::string::npos) {
            category = filename.substr(0, underscorePos);
        }
        
        std::cout << "Query image: " << selectedFile << " (category: " << category << ")" << std::endl;
        return extractRGBFromFile(selectedFile, category);
    }
};

struct BenchmarkResult {
    std::string structure_name;
    double insert_time_ms;
    double search_time_ms;
    int images_found;
    int dataset_size;
};

BenchmarkResult benchmarkStructure(ImageDatabase* db, const std::vector<Image>& images, const Image& query, double threshold) {
    db->clear();
    
    auto start = std::chrono::high_resolution_clock::now();
    for (const auto& img : images) {
        db->insert(img);
    }
    auto end = std::chrono::high_resolution_clock::now();
    double insert_time = std::chrono::duration<double, std::milli>(end - start).count();
    
    start = std::chrono::high_resolution_clock::now();
    std::vector<Image> results = db->search(query, threshold);
    end = std::chrono::high_resolution_clock::now();
    double search_time = std::chrono::duration<double, std::milli>(end - start).count();
    
    return {db->getName(), insert_time, search_time, static_cast<int>(results.size()), static_cast<int>(images.size())};
}

int main() {
    const std::string imageDir = "./images/";
    const double THRESHOLD = 50.0;
    
    // Count total images in directory first
    namespace fs = std::filesystem;
    int totalImages = 0;
    for (const auto& entry : fs::directory_iterator(imageDir)) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            if (filename.find(".jpg") != std::string::npos || filename.find(".png") != std::string::npos) {
                totalImages++;
            }
        }
    }
    
    std::cout << "Total de imagens encontradas: " << totalImages << std::endl;
    
    // Test with all scales now that Quadtree works
    std::vector<int> testSizes;
    if (totalImages >= 50) testSizes.push_back(50);
    if (totalImages >= 100) testSizes.push_back(100);
    if (totalImages >= 300) testSizes.push_back(300);
    if (totalImages >= 500) testSizes.push_back(500);
    if (totalImages >= 1000) testSizes.push_back(1000);
    if (totalImages >= 2000) testSizes.push_back(2000);
    if (totalImages >= 5000) testSizes.push_back(5000);
    if (totalImages >= 8000) testSizes.push_back(std::min(totalImages, 8000));
    // Always add total images if different from last added size
    if (totalImages > 5000 && (testSizes.empty() || totalImages != testSizes.back())) {
        testSizes.push_back(totalImages);
    }
    
    std::cout << "==================================================================================" << std::endl;
    std::cout << " BENCHMARK IMAGENS LOCAIS - PAA Assignment 1 - DADOS REAIS" << std::endl;
    std::cout << "==================================================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Dataset: images/ (" << totalImages << " imagens)" << std::endl;
    std::cout << "Threshold: 50.0" << std::endl;
    std::cout << "Query: Imagem aleatoria (RGB medio extraido da foto)" << std::endl;
    std::cout << std::endl;
    
    RealImageLoader loader(imageDir);
    
    std::cout << "Carregando dataset de forma eficiente..." << std::endl;
    
    Image queryImage = loader.getRandomQueryImage();
    std::cout << "Query RGB: (" << queryImage.r << ", " << queryImage.g << ", " << queryImage.b << ")" << std::endl;
    std::cout << std::endl;
    
    // Test only stable structures (trees removed due to memory constraints)
    std::vector<std::string> structureNames = {
        "LinearSearch", "HashSearch", "HashSearchDynamic"
    };
    
    std::vector<BenchmarkResult> allResults;
    
    for (int testSize : testSizes) {
        std::cout << "[TESTANDO] Escala: " << testSize << " imagens..." << std::endl;
        
        std::vector<Image> testDataset = loader.loadImages(testSize);
        
        // Test each structure individually: CREATE -> TEST -> DESTROY
        for (const std::string& structName : structureNames) {
            std::unique_ptr<ImageDatabase> structure;
            
            // Create only one structure at a time
            if (structName == "LinearSearch") {
                structure = std::make_unique<LinearSearch>();
            } else if (structName == "HashSearch") {
                structure = std::make_unique<HashSearch>();
            } else if (structName == "HashSearchDynamic") {
                structure = std::make_unique<HashSearchDynamic>();
            }
            
            // Test this structure
            BenchmarkResult result = benchmarkStructure(structure.get(), testDataset, queryImage, THRESHOLD);
            allResults.push_back(result);
            
            std::cout << "  " << result.structure_name << ": "
                     << "Insert=" << std::fixed << std::setprecision(3) << result.insert_time_ms << "ms, "
                     << "Search=" << result.search_time_ms << "ms, "
                     << "Found=" << result.images_found << std::endl;
            
            // Structure automatically destroyed when unique_ptr goes out of scope
        }
        std::cout << std::endl;
    }
    
    std::cout << "==================================================================================" << std::endl;
    std::cout << "RESULTADOS FINAIS - TABELA ORGANIZADA" << std::endl;
    std::cout << "==================================================================================" << std::endl;
    std::cout << std::endl;
    
    std::cout << "Dataset        Estrutura               Insert(ms)       Search(ms)       Found       " << std::endl;
    std::cout << "-------------------------------------------------------------------------------" << std::endl;
    
    for (int testSize : testSizes) {
        std::cout << std::left << std::setw(15) << testSize;
        
        for (int i = 0; i < 3; i++) {
            auto it = std::find_if(allResults.begin(), allResults.end(),
                [testSize, i](const BenchmarkResult& r) {
                    return r.dataset_size == testSize && 
                           ((i == 0 && r.structure_name == "Linear Search") ||
                            (i == 1 && r.structure_name == "Hash Search") ||
                            (i == 2 && r.structure_name == "Hash Dynamic Search"));
                });
            
            if (it != allResults.end()) {
                if (i == 0) {
                    std::cout << std::left << std::setw(25) << it->structure_name;
                } else {
                    std::cout << std::string(15, ' ') << std::left << std::setw(25) << it->structure_name;
                }
                std::cout << std::left << std::setw(17) << std::fixed << std::setprecision(3) << it->insert_time_ms
                         << std::left << std::setw(17) << it->search_time_ms
                         << std::left << std::setw(12) << it->images_found << std::endl;
            }
        }
        std::cout << "-------------------------------------------------------------------------------" << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "ANALISE DE VENCEDORES POR ESCALA:" << std::endl;
    std::cout << "==================================================================================" << std::endl;
    
    for (int testSize : testSizes) {
        BenchmarkResult bestInsert = {"", 1e9, 0, 0, 0};
        BenchmarkResult bestSearch = {"", 0, 1e9, 0, 0};
        
        for (const auto& result : allResults) {
            if (result.dataset_size == testSize) {
                if (result.insert_time_ms < bestInsert.insert_time_ms) {
                    bestInsert = result;
                }
                if (result.search_time_ms < bestSearch.search_time_ms) {
                    bestSearch = result;
                }
            }
        }
        
        std::cout << std::left << std::setw(15) << testSize
                 << "| Insert: " << std::left << std::setw(20) << bestInsert.structure_name
                 << "(" << std::fixed << std::setprecision(3) << bestInsert.insert_time_ms << "ms)"
                 << " | Search: " << std::left << std::setw(20) << bestSearch.structure_name
                 << "(" << bestSearch.search_time_ms << "ms)" << std::endl;
    }
    
    std::cout << std::endl;
    std::cout << "==================================================================================" << std::endl;
    std::cout << "Benchmark Concluido! Analise com imagens reais." << std::endl;
    std::cout << "   Query: Imagem aleatoria" << std::endl;
    std::cout << "   RGB extraido: (" << queryImage.r << ", " << queryImage.g << ", " << queryImage.b << ")" << std::endl;
    std::cout << "   Dados prontos para analise comparativa." << std::endl;
    std::cout << "==================================================================================" << std::endl;
    
    return 0;
}