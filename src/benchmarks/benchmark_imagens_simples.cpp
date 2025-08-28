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
#include "stb_image.h"

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

class RealImageLoader {
private:
    std::string baseDir;
    std::vector<std::string> categories;
    std::mt19937 rng;

public:
    RealImageLoader(const std::string& dir) : baseDir(dir), rng(42) {
        categories = {"airplane", "car", "cat", "dog", "flower", "fruit", "motorbike", "person"};
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
        
        for (const auto& category : categories) {
            namespace fs = std::filesystem;
            for (const auto& entry : fs::directory_iterator(baseDir)) {
                if (entry.is_regular_file()) {
                    std::string filename = entry.path().filename().string();
                    if (filename.find(category) == 0 && filename.find(".jpg") != std::string::npos) {
                        allFiles.push_back(entry.path().string() + ":" + category);
                    }
                }
            }
        }
        
        if (maxImages > 0 && allFiles.size() > static_cast<size_t>(maxImages)) {
            std::shuffle(allFiles.begin(), allFiles.end(), rng);
            allFiles.resize(maxImages);
        }
        
        std::cout << "Loading " << allFiles.size() << " images..." << std::endl;
        
        for (size_t i = 0; i < allFiles.size(); i++) {
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
        std::uniform_int_distribution<int> categoryDist(0, categories.size() - 1);
        std::string selectedCategory = categories[categoryDist(rng)];
        
        std::vector<std::string> categoryFiles;
        namespace fs = std::filesystem;
        for (const auto& entry : fs::directory_iterator(baseDir)) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                if (filename.find(selectedCategory) == 0 && filename.find(".jpg") != std::string::npos) {
                    categoryFiles.push_back(entry.path().string());
                }
            }
        }
        
        if (categoryFiles.empty()) {
            return Image(128, 128, 128, "fallback", selectedCategory);
        }
        
        std::uniform_int_distribution<int> fileDist(0, categoryFiles.size() - 1);
        std::string selectedFile = categoryFiles[fileDist(rng)];
        
        std::cout << "Query image: " << selectedFile << " (" << selectedCategory << ")" << std::endl;
        return extractRGBFromFile(selectedFile, selectedCategory);
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
    std::cout << "==================================================================================" << std::endl;
    std::cout << " BENCHMARK IMAGENS LOCAIS SIMPLIFICADO - PAA Assignment 1 - DADOS REAIS" << std::endl;
    std::cout << "==================================================================================" << std::endl;
    std::cout << std::endl;
    std::cout << "Dataset: Images/natural_images/ (imagens reais Kaggle)" << std::endl;
    std::cout << "Categorias: airplane, car, cat, dog, flower, fruit, motorbike, person" << std::endl;
    std::cout << "Estruturas: Linear Search, Hash Search" << std::endl;
    std::cout << "Threshold: 50.0" << std::endl;
    std::cout << "Query: Imagem real aleatória (RGB extraído da foto)" << std::endl;
    std::cout << std::endl;
    
    const std::string imageDir = "./Images/natural_images/";
    const double THRESHOLD = 50.0;
    const std::vector<int> testSizes = {50, 100, 200, 500, 1000, 2000};
    
    RealImageLoader loader(imageDir);
    
    Image queryImage = loader.getRandomQueryImage();
    std::cout << "Query RGB: (" << queryImage.r << ", " << queryImage.g << ", " << queryImage.b << ")" << std::endl;
    std::cout << "Query categoria: " << queryImage.category << std::endl;
    std::cout << std::endl;
    
    std::vector<std::unique_ptr<ImageDatabase>> structures;
    structures.push_back(std::make_unique<LinearSearch>());
    structures.push_back(std::make_unique<HashSearch>());
    
    std::vector<BenchmarkResult> allResults;
    
    for (int testSize : testSizes) {
        std::cout << "[TESTANDO] Escala: " << testSize << " imagens..." << std::endl;
        
        std::vector<Image> testDataset = loader.loadImages(testSize);
        
        for (auto& structure : structures) {
            BenchmarkResult result = benchmarkStructure(structure.get(), testDataset, queryImage, THRESHOLD);
            allResults.push_back(result);
            
            std::cout << "  " << result.structure_name << ": "
                     << "Insert=" << std::fixed << std::setprecision(3) << result.insert_time_ms << "ms, "
                     << "Search=" << result.search_time_ms << "ms, "
                     << "Found=" << result.images_found << std::endl;
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
        
        for (int i = 0; i < 2; i++) {
            auto it = std::find_if(allResults.begin(), allResults.end(),
                [testSize, i](const BenchmarkResult& r) {
                    return r.dataset_size == testSize && 
                           ((i == 0 && r.structure_name == "Linear Search") ||
                            (i == 1 && r.structure_name == "Hash Search"));
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
    std::cout << "ANÁLISE DE VENCEDORES POR ESCALA:" << std::endl;
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
    std::cout << "Benchmark Concluído! Análise SIMPLIFICADA com imagens reais do dataset Kaggle." << std::endl;
    std::cout << "   Query: Imagem real aleatória (categoria: " << queryImage.category << ")" << std::endl;
    std::cout << "   RGB extraído: (" << queryImage.r << ", " << queryImage.g << ", " << queryImage.b << ")" << std::endl;
    std::cout << "   APENAS Linear e Hash Search - versão estável para commit." << std::endl;
    std::cout << "==================================================================================" << std::endl;
    
    return 0;
}