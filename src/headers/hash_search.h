#ifndef HASH_SEARCH_H
#define HASH_SEARCH_H

#include <unordered_map>
#include <vector>
#include <string>

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
    
public:
    // Construtor - cellSize determina granularidade do hash
    HashSearch(double _cellSize = 30.0) : cellSize(_cellSize) {}
    
    void insert(const Image& img) override {
        std::string key = getCellKey(img);
        grid[key].push_back(img);
    }
    
    std::vector<Image> findSimilar(const Image& query, double threshold) override {
        std::vector<Image> results;
        
        // Determinar células base da consulta
        int query_r = rgbToCell(query.r);
        int query_g = rgbToCell(query.g);
        int query_b = rgbToCell(query.b);
        
        // Calcular quantas células precisamos verificar baseado no threshold
        int cell_radius = static_cast<int>(ceil(threshold / cellSize));
        
        // Buscar em células vizinhas (cubo 3D)
        for (int dr = -cell_radius; dr <= cell_radius; dr++) {
            for (int dg = -cell_radius; dg <= cell_radius; dg++) {
                for (int db = -cell_radius; db <= cell_radius; db++) {
                    std::string key = getCellKey(query_r + dr, query_g + dg, query_b + db);
                    
                    // Se a célula existe, verificar todas as imagens nela
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
            }
        }
        
        // Ordena por distância
        std::sort(results.begin(), results.end(), 
                 [&query](const Image& a, const Image& b) {
                     return query.distanceTo(a) < query.distanceTo(b);
                 });
        
        return results;
    }
    
    std::string getName() const override {
        return "Hash Search (Grid-based, cell=" + std::to_string(cellSize) + ")";
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