#ifndef HASH_SEARCH_H
#define HASH_SEARCH_H

#include <unordered_map>
#include <vector>
#include <string>
#include <algorithm>
#include <iostream>
#include <cmath>

// Hash Table baseada em grid 3D do espaço RGB
class HashSearch : public ImageDatabase {
private:
    // Tamanho da célula do grid (quanto menor, mais preciso mas mais células)
    double cellSize;
    
    // Hash table: key = "r,g,b" da célula, value = lista de imagens nessa célula
    std::unordered_map<std::string, std::vector<Image>> grid;
    
    // Converte coordenada RGB para coordenada da célula
    int rgbToCell(double value) const {
        return static_cast<int>(value / cellSize);
    }
    
    // Gera chave da célula para uma imagem
    std::string getCellKey(const Image& img) const {
        int r_cell = rgbToCell(img.r);
        int g_cell = rgbToCell(img.g);
        int b_cell = rgbToCell(img.b);
        return std::to_string(r_cell) + "," + std::to_string(g_cell) + "," + std::to_string(b_cell);
    }
    
    // Gera chave da célula para coordenadas específicas
    std::string getCellKey(int r_cell, int g_cell, int b_cell) const {
        return std::to_string(r_cell) + "," + std::to_string(g_cell) + "," + std::to_string(b_cell);
    }
    
    // Search cells at specific radius from center (dynamic expanding cube)
    void searchCubeAtRadius(int center_r, int center_g, int center_b, int radius, 
                           const Image& query, double threshold, std::vector<Image>& results) const {
        if (radius == 0) {
            // Search only center cell for radius 0
            searchSingleCell(center_r, center_g, center_b, query, threshold, results);
            return;
        }
        
        // Search cells that are exactly at 'radius' distance (cube boundary)
        for (int dr = -radius; dr <= radius; dr++) {
            for (int dg = -radius; dg <= radius; dg++) {
                for (int db = -radius; db <= radius; db++) {
                    // Only search cells at the boundary of current radius (Manhattan distance)
                    if (abs(dr) + abs(dg) + abs(db) == radius || 
                        (abs(dr) == radius || abs(dg) == radius || abs(db) == radius)) {
                        searchSingleCell(center_r + dr, center_g + dg, center_b + db, 
                                       query, threshold, results);
                    }
                }
            }
        }
    }
    
    // Search a single cell and add matching images to results
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
    // Construtor - cellSize determina granularidade do hash
    HashSearch(double _cellSize = 30.0) : cellSize(_cellSize) {}
    
    void insert(const Image& img) override {
        std::string key = getCellKey(img);
        grid[key].push_back(img);
    }
    
    std::vector<Image> findSimilar(const Image& query, double threshold) override {
        return findSimilarDynamic(query, threshold, -1); // -1 = no result limit
    }
    
    // Dynamic search with expanding cube and early termination
    std::vector<Image> findSimilarDynamic(const Image& query, double threshold, int maxResults = -1) {
        std::vector<Image> results;
        
        // Determinar células base da consulta
        int query_r = rgbToCell(query.r);
        int query_g = rgbToCell(query.g);
        int query_b = rgbToCell(query.b);
        
        // Calcular quantas células precisamos verificar baseado no threshold
        int max_radius = static_cast<int>(ceil(threshold / cellSize));
        
        // Dynamic search: expand from query cell outward
        for (int radius = 0; radius <= max_radius; radius++) {
            size_t results_before = results.size();
            
            // Search cells at current radius
            searchCubeAtRadius(query_r, query_g, query_b, radius, query, threshold, results);
            
            // Early termination: if we found enough results, stop
            if (maxResults > 0 && static_cast<int>(results.size()) >= maxResults) {
                break;
            }
            
            // Early termination: if no new results found at this radius and we already have some
            if (radius > 0 && results.size() == results_before && !results.empty()) {
                // Optional: could break here for aggressive early termination
                // break;
            }
        }
        
        // Ordena por distância
        std::sort(results.begin(), results.end(), 
                 [&query](const Image& a, const Image& b) {
                     return query.distanceTo(a) < query.distanceTo(b);
                 });
        
        // Limit results if requested
        if (maxResults > 0 && static_cast<int>(results.size()) > maxResults) {
            results.erase(results.begin() + maxResults, results.end());
        }
        
        return results;
    }
    
    std::string getName() const override {
        return "Hash Search (Dynamic, cell=" + std::to_string(cellSize) + ")";
    }
    
    // Public interface for dynamic search with configurable parameters
    std::vector<Image> findSimilarWithLimit(const Image& query, double threshold, int maxResults) {
        return findSimilarDynamic(query, threshold, maxResults);
    }
    
    void clear() {
        grid.clear();
    }
    
    // Métodos para análise
    size_t getNumCells() const {
        return grid.size();
    }
    
    double getAverageCellSize() const {
        if (grid.empty()) return 0;
        
        size_t totalImages = 0;
        for (const auto& pair : grid) {
            totalImages += pair.second.size();
        }
        return static_cast<double>(totalImages) / grid.size();
    }
    
    void printStats() const {
        std::cout << "Hash Table Stats:" << std::endl;
        std::cout << "  Células ativas: " << getNumCells() << std::endl;
        std::cout << "  Tamanho médio da célula: " << getAverageCellSize() << " imagens" << std::endl;
        std::cout << "  Cell size: " << cellSize << std::endl;
        
        // Distribuição de tamanhos
        std::vector<int> cellSizes;
        for (const auto& pair : grid) {
            cellSizes.push_back(pair.second.size());
        }
        if (!cellSizes.empty()) {
            std::sort(cellSizes.begin(), cellSizes.end());
            std::cout << "  Menor célula: " << cellSizes.front() << " imagens" << std::endl;
            std::cout << "  Maior célula: " << cellSizes.back() << " imagens" << std::endl;
            std::cout << "  Mediana: " << cellSizes[cellSizes.size()/2] << " imagens" << std::endl;
        }
    }
};

#endif