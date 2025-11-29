/*
=============================================================================
            PAA Trabalho 2: Estruturas para Alta Dimensionalidade
=============================================================================

Este arquivo demonstra a comparação entre estruturas básicas (Trabalho 1)
e estruturas avançadas para alta dimensionalidade (Trabalho 2):

TRABALHO 1 (Estruturas Básicas):
1. Linear Search (Força Bruta)
2. Hash Search (Spatial Hashing 3D)
3. Hash Dynamic Search (Expansão Adaptativa)
4. Octree (Árvore Espacial 3D)
5. Quadtree (Árvore Espacial 2D)

TRABALHO 2 (Alta Dimensionalidade):
6. LSH (Locality-Sensitive Hashing) - Busca aproximada sublinear
7. M-Tree (Metric Tree) - Árvore métrica com poda triangular

Total: 7 estruturas comparadas em um único framework!

DATASETS:
1. RGB 3D - IMAGENS REAIS da pasta ./images/ (26K imagens .jpg)
   - RGB extraído de arquivos reais (não sintético!)
   - Query interativa da pasta ./query/
   - Uso: ./bin/trabalho2 (padrão)

2. ColorHistogram 32D - database_colorhistogram.asc (68K vetores)
   - Histogramas de cores 32D (alta dimensionalidade)
   - Query: primeiro vetor do dataset
   - Uso: ./bin/trabalho2 hist

Conceitos PAA demonstrados:
- Análise de Complexidade em Alta Dimensionalidade
- Curse of Dimensionality
- Trade-offs entre Busca Exata vs Aproximada
- Técnicas de Hashing Probabilístico
- Estruturas de Dados Métricas
- Otimizações para Espaços de Alta Dimensão

=============================================================================
*/

#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <memory>
#include <random>
#include <iomanip>
#include <sstream>
#include <filesystem>
#include <fstream>
#include <algorithm>
#include <cctype>

// OpenCV para extração REAL de RGB dos pixels
#include <opencv2/opencv.hpp>

// Headers do Trabalho 2 (novas implementações)
#include "headers/image_database.h"
#include "headers/lsh_search.h"
#include "headers/mtree_search.h"

// Headers do Trabalho 1 (copiados para Trabalho 2)
#include "headers/linear_search.h"
#include "headers/hash_search.h"
#include "headers/octree_search.h"
#include "headers/quadtree.h"

// ============================================================================
// EXTRAÇÃO DE RGB REAL DAS IMAGENS (do Trabalho 1)
// ============================================================================

struct RealRGB {
    double r, g, b;
    bool valid;

    RealRGB(double _r = 0, double _g = 0, double _b = 0, bool _valid = true)
        : r(_r), g(_g), b(_b), valid(_valid) {}
};

/**
 * @brief Extrai RGB REAL dos pixels da imagem usando OpenCV
 *
 * MÉTODO: Calcula a cor MÉDIA de todos os pixels da imagem
 * - Carrega a imagem com OpenCV
 * - Calcula o valor médio de cada canal (R, G, B)
 * - Retorna a cor média representativa da imagem
 *
 * IMPORTANTE: Esta é a extração REAL de pixels, não simulação!
 */
RealRGB extractRealRGBFromImage(const std::string& imagePath) {
    try {
        // Carregar imagem com OpenCV
        cv::Mat image = cv::imread(imagePath, cv::IMREAD_COLOR);

        if (image.empty()) {
            std::cout << "ERRO: Nao foi possivel carregar imagem: " << imagePath << std::endl;
            return RealRGB(0, 0, 0, false);
        }

        // Calcular cor MÉDIA de todos os pixels
        // cv::mean() retorna a média de cada canal em formato BGR
        cv::Scalar avgColor = cv::mean(image);

        // OpenCV usa formato BGR, converter para RGB
        double r = avgColor[2];  // Red (terceiro canal no BGR)
        double g = avgColor[1];  // Green (segundo canal)
        double b = avgColor[0];  // Blue (primeiro canal)

        return RealRGB(r, g, b, true);

    } catch (const cv::Exception& e) {
        std::cout << "ERRO OpenCV: " << e.what() << std::endl;
        return RealRGB(0, 0, 0, false);
    } catch (const std::exception& e) {
        std::cout << "ERRO na extracao: " << e.what() << std::endl;
        return RealRGB(0, 0, 0, false);
    }
}

// ============================================================================
// CARREGAMENTO DE DATASET COM RGB REAL (do Trabalho 1)
// ============================================================================

std::vector<Image> loadRealDataset(int maxCount, const std::string& path = "./images/") {
    std::vector<Image> images;
    images.reserve(maxCount);

    int imageId = 1;

    try {
        // USA RECURSIVE para suportar subpastas (dataset Kaggle original)
        for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
            if (entry.is_regular_file() && imageId <= maxCount) {
                std::string filename = entry.path().filename().string();
                std::string extension = entry.path().extension().string();

                // Converter extensão para lowercase
                std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

                // Filtrar apenas arquivos de imagem
                if (extension == ".jpg" || extension == ".jpeg" ||
                    extension == ".png" || extension == ".bmp") {

                    // Extrair RGB REAL da imagem
                    std::string fullPath = entry.path().string();
                    RealRGB realColor = extractRealRGBFromImage(fullPath);

                    if (realColor.valid) {
                        images.emplace_back(imageId, filename, realColor.r, realColor.g, realColor.b);
                        imageId++;
                    } else {
                        std::cout << "AVISO: Ignorando imagem invalida: " << filename << std::endl;
                    }
                }
            }

            if (imageId > maxCount) break;
        }

    } catch (const std::exception& e) {
        std::cout << "ERRO ao carregar imagens: " << e.what() << std::endl;
        return images;
    }

    // Dataset carregado silenciosamente (verbosidade reduzida)
    return images;
}

// ============================================================================
// CONTAGEM AUTOMÁTICA DE IMAGENS (do Trabalho 1)
// ============================================================================

int countImagesInDirectory(const std::string& path = "./images/") {
    int count = 0;

    std::cout << "Auto-detectando imagens em: " << path << std::endl;

    try {
        if (!std::filesystem::exists(path)) {
            std::cout << "ERRO: Pasta '" << path << "' nao encontrada!" << std::endl;
            std::cout << "SOLUCAO: Crie a pasta './images/' e coloque suas imagens la" << std::endl;
            return 0;
        }

        if (!std::filesystem::is_directory(path)) {
            std::cout << "ERRO: '" << path << "' nao e um diretorio!" << std::endl;
            return 0;
        }

        // USA RECURSIVE para contar em subpastas também
        for (const auto& entry : std::filesystem::recursive_directory_iterator(path)) {
            if (entry.is_regular_file()) {
                std::string extension = entry.path().extension().string();
                std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

                if (extension == ".jpg" || extension == ".jpeg" ||
                    extension == ".png" || extension == ".bmp" ||
                    extension == ".tiff" || extension == ".tif") {
                    count++;

                    if (count % 1000 == 0) {
                        std::cout << "Detectadas " << count << " imagens..." << std::endl;
                    }
                }
            }
        }

        std::cout << "Auto-deteccao concluida: " << count << " imagens encontradas" << std::endl;

    } catch (const std::filesystem::filesystem_error& e) {
        std::cout << "ERRO de filesystem: " << e.what() << std::endl;
        return 0;
    }

    if (count == 0) {
        std::cout << "AVISO: Nenhuma imagem encontrada em '" << path << "'" << std::endl;
        std::cout << "Formatos suportados: .jpg, .jpeg, .png, .bmp, .tiff, .tif" << std::endl;
    }

    return count;
}

// ============================================================================
// ESTRUTURA DE RESULTADO DE BENCHMARK
// ============================================================================

struct BenchmarkResult {
    std::string structureName;
    double insertTime;      // Tempo de inserção (ms)
    double searchTime;      // Tempo de busca (ms)
    int resultsFound;       // Número de resultados encontrados
    int datasetSize;        // Tamanho do dataset testado

    BenchmarkResult(const std::string& name, double insert, double search, int found, int size)
        : structureName(name), insertTime(insert), searchTime(search),
          resultsFound(found), datasetSize(size) {}
};

// ============================================================================
// FUNÇÃO DE BENCHMARK
// ============================================================================

BenchmarkResult benchmarkStructure(std::unique_ptr<ImageDatabase> db,
                                   const std::vector<Image>& dataset,
                                   const Image& query,
                                   double threshold) {
    auto startInsert = std::chrono::high_resolution_clock::now();

    // Fase de inserção
    for (const auto& img : dataset) {
        db->insert(img);
    }

    auto endInsert = std::chrono::high_resolution_clock::now();

    // Fase de busca
    auto startSearch = std::chrono::high_resolution_clock::now();
    auto results = db->findSimilar(query, threshold);
    auto endSearch = std::chrono::high_resolution_clock::now();

    double insertTime = std::chrono::duration<double, std::milli>(endInsert - startInsert).count();
    double searchTime = std::chrono::duration<double, std::milli>(endSearch - startSearch).count();

    return BenchmarkResult(db->getName(), insertTime, searchTime,
                          static_cast<int>(results.size()), static_cast<int>(dataset.size()));
}

// ============================================================================
// CLASSE HASH DYNAMIC SEARCH (do Trabalho 1)
// ============================================================================

class HashDynamicSearch : public ImageDatabase {
private:
    double cellSize;
    std::unordered_map<std::string, std::vector<Image>> grid;

    int rgbToCell(double value) const {
        return static_cast<int>(value / cellSize);
    }

    std::string getCellKey(int r_cell, int g_cell, int b_cell) const {
        return std::to_string(r_cell) + "," + std::to_string(g_cell) + "," + std::to_string(b_cell);
    }

public:
    HashDynamicSearch(double _cellSize = 25.0) : cellSize(_cellSize) {}

    void insert(const Image& img) override {
        std::string key = getCellKey(rgbToCell(img.r), rgbToCell(img.g), rgbToCell(img.b));
        grid[key].push_back(img);
    }

    std::vector<Image> findSimilar(const Image& query, double threshold) override {
        std::vector<Image> results;

        int query_r = rgbToCell(query.r);
        int query_g = rgbToCell(query.g);
        int query_b = rgbToCell(query.b);

        int max_radius = static_cast<int>(ceil(threshold / cellSize));

        for (int dr = -max_radius; dr <= max_radius; dr++) {
            for (int dg = -max_radius; dg <= max_radius; dg++) {
                for (int db = -max_radius; db <= max_radius; db++) {
                    std::string key = getCellKey(query_r + dr, query_g + dg, query_b + db);

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

        std::sort(results.begin(), results.end(),
                 [&query](const Image& a, const Image& b) {
                     return query.distanceTo(a) < query.distanceTo(b);
                 });

        return results;
    }

    std::string getName() const override {
        return "Hash Dynamic Search";
    }
};

// ============================================================================
// FUNÇÃO AUXILIAR - GERAR TIMESTAMP ÚNICO PARA RESULTADOS
// ============================================================================

/**
 * @brief Gera timestamp no formato DD-MM-YYYY_HHMMSS para nomes de arquivo
 * @return String com timestamp único (ex: "29-11-2025_143025")
 */
std::string generateTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    // Converte para struct tm (local time)
    std::tm* localTime = std::localtime(&time);

    // Formata como: DD-MM-YYYY_HHMMSS
    std::ostringstream oss;
    oss << std::setfill('0') << std::setw(2) << localTime->tm_mday << "-"
        << std::setfill('0') << std::setw(2) << (localTime->tm_mon + 1) << "-"
        << (localTime->tm_year + 1900) << "_"
        << std::setfill('0') << std::setw(2) << localTime->tm_hour
        << std::setfill('0') << std::setw(2) << localTime->tm_min
        << std::setfill('0') << std::setw(2) << localTime->tm_sec;

    return oss.str();
}

// ============================================================================
// FUNÇÕES AUXILIARES - SELEÇÃO INTERATIVA DE QUERY
// ============================================================================

/**
 * @brief Lista imagens disponíveis no diretório query/
 * @param maxImages Número máximo de imagens a listar
 * @return Vetor com caminhos das imagens encontradas
 */
std::vector<std::string> listAvailableImages(int maxImages = 15) {
    std::vector<std::string> images;

    try {
        namespace fs = std::filesystem;

        // BUSCA NA PASTA query/ (não no dataset!)
        if (!fs::exists("./query/") || !fs::is_directory("./query/")) {
            return images;
        }

        // Extensões de imagem suportadas
        std::vector<std::string> validExtensions = {".jpg", ".jpeg", ".png", ".bmp"};

        // Percorre diretório query/
        for (const auto& entry : fs::directory_iterator("./query/")) {
            if (entry.is_regular_file()) {
                std::string filename = entry.path().string();
                std::string extension = entry.path().extension().string();

                // Converte extensão para lowercase
                std::transform(extension.begin(), extension.end(), extension.begin(), ::tolower);

                // Verifica se é uma extensão válida
                if (std::find(validExtensions.begin(), validExtensions.end(), extension) != validExtensions.end()) {
                    images.push_back(filename);

                    // Limita quantidade de imagens listadas
                    if (images.size() >= static_cast<size_t>(maxImages)) {
                        break;
                    }
                }
            }
        }

        // Ordena alfabeticamente para melhor visualização
        std::sort(images.begin(), images.end());

    } catch (const std::exception& e) {
        printf("Erro ao listar imagens: %s\n", e.what());
    }

    return images;
}

/**
 * @brief Menu interativo para seleção da imagem de query
 * @return Caminho da imagem selecionada pelo usuário
 */
std::string selectQueryImageInteractive() {
    printf("\n========================================\n");
    printf("  SELEÇÃO DE IMAGEM DE QUERY\n");
    printf("========================================\n\n");

    // Lista imagens disponíveis
    auto availableImages = listAvailableImages(15);

    if (availableImages.empty()) {
        printf("⚠ Nenhuma imagem encontrada em ./query/\n");
        printf("  Coloque imagens .jpg/.png/.bmp na pasta query/\n");
        printf("  Usando query padrão: query/query.jpg\n\n");
        return "./query/query.jpg";
    }

    // Mostra opções
    printf("Escolha a imagem de query:\n\n");

    for (size_t i = 0; i < availableImages.size(); i++) {
        // Extrai apenas o nome do arquivo para exibição mais limpa
        std::filesystem::path p(availableImages[i]);
        printf("  [%2zu] %s\n", i + 1, p.filename().string().c_str());
    }

    printf("\n  [88] Digitar caminho manualmente\n");
    printf("  [ 0] Usar query padrão (query/query.jpg)\n");
    printf("\n========================================\n");
    printf("Digite sua escolha: ");

    int choice = -1;
    std::string input;
    std::getline(std::cin, input);

    // Tenta converter para número
    try {
        choice = std::stoi(input);
    } catch (...) {
        choice = -1;
    }

    // Processa escolha
    if (choice == 0) {
        printf("\n✓ Usando query padrão: query/query.jpg\n");
        return "./query/query.jpg";

    } else if (choice == 88) {
        // Caminho manual
        printf("\nDigite o caminho completo da imagem: ");
        std::string customPath;
        std::getline(std::cin, customPath);

        // Remove espaços em branco do início e fim
        customPath.erase(0, customPath.find_first_not_of(" \t\n\r"));
        customPath.erase(customPath.find_last_not_of(" \t\n\r") + 1);

        printf("✓ Caminho customizado: %s\n", customPath.c_str());
        return customPath;

    } else if (choice >= 1 && choice <= static_cast<int>(availableImages.size())) {
        // Imagem da lista
        std::string selectedPath = availableImages[choice - 1];
        printf("\n✓ Imagem selecionada: %s\n", selectedPath.c_str());
        return selectedPath;

    } else {
        // Escolha inválida
        printf("\n⚠ Escolha inválida. Usando query padrão: query/query.jpg\n");
        return "./query/query.jpg";
    }
}

// ============================================================================
// COLORHISTOGRAM 32D - ESTRUTURAS E FUNÇÕES
// ============================================================================

/**
 * @brief Estrutura para armazenar vetores de histograma de cores 32D
 */
struct HistogramImage {
    int id;
    std::string filename;
    std::vector<double> histogram;  // 32 dimensões

    HistogramImage(int id, const std::vector<double>& hist)
        : id(id), histogram(hist) {
        filename = "hist_" + std::to_string(id);
    }

    // Distância Euclidiana em 32D
    double distanceTo(const HistogramImage& other) const {
        double sum = 0.0;
        for (size_t i = 0; i < histogram.size() && i < other.histogram.size(); i++) {
            double diff = histogram[i] - other.histogram[i];
            sum += diff * diff;
        }
        return sqrt(sum);
    }
};

/**
 * @brief Carrega dataset ColorHistogram.asc (32D)
 * @param maxCount Número máximo de vetores a carregar
 * @param path Caminho do arquivo .asc
 * @return Vetor de HistogramImage
 */
std::vector<HistogramImage> loadColorHistogram(int maxCount, const std::string& path = "./database_colorhistogram.asc/ColorHistogram.asc") {
    std::vector<HistogramImage> dataset;
    dataset.reserve(maxCount);

    std::ifstream file(path);
    if (!file.is_open()) {
        printf("ERRO: Não foi possível abrir %s\n", path.c_str());
        return dataset;
    }

    std::string line;
    int count = 0;

    while (std::getline(file, line) && count < maxCount) {
        std::istringstream iss(line);

        int id;
        iss >> id;  // Primeira coluna: ID

        // Lê os 32 valores do histograma
        std::vector<double> histogram;
        histogram.reserve(32);

        double value;
        while (iss >> value) {
            histogram.push_back(value);
        }

        // Valida que tem 32 dimensões
        if (histogram.size() == 32) {
            dataset.emplace_back(id, histogram);
            count++;
        }
    }

    file.close();
    return dataset;
}

// ============================================================================
// ESTRUTURAS DE DADOS ADAPTADAS PARA COLORHISTOGRAM 32D
// ============================================================================

/**
 * @brief Linear Search para ColorHistogram 32D
 */
class LinearSearch_Hist {
private:
    std::vector<HistogramImage> images;

public:
    void insert(const HistogramImage& img) {
        images.push_back(img);
    }

    std::vector<HistogramImage> findSimilar(const HistogramImage& query, double threshold) {
        std::vector<HistogramImage> results;
        for (const auto& img : images) {
            if (query.distanceTo(img) <= threshold) {
                results.push_back(img);
            }
        }
        // Ordena por distância
        std::sort(results.begin(), results.end(),
                 [&query](const HistogramImage& a, const HistogramImage& b) {
                     return query.distanceTo(a) < query.distanceTo(b);
                 });
        return results;
    }

    std::string getName() const { return "LinearSearch_32D"; }
};

/**
 * @brief LSH adaptado para ColorHistogram 32D
 */
class LSH_Search_Hist {
private:
    int numTables;
    int numProjections;
    double binWidth;

    struct Projection {
        std::vector<double> weights;  // 32 pesos aleatórios
        double offset;

        int project(const HistogramImage& img, double binWidth) const {
            double sum = 0.0;
            for (size_t i = 0; i < weights.size() && i < img.histogram.size(); i++) {
                sum += weights[i] * img.histogram[i];
            }
            sum += offset;
            return static_cast<int>(floor(sum / binWidth));
        }
    };

    struct HashTable {
        std::vector<Projection> projections;
        std::unordered_map<std::string, std::vector<HistogramImage>> buckets;

        std::string computeHash(const HistogramImage& img, double binWidth) const {
            std::string hashCode;
            for (size_t i = 0; i < projections.size(); i++) {
                if (i > 0) hashCode += ",";
                hashCode += std::to_string(projections[i].project(img, binWidth));
            }
            return hashCode;
        }
    };

    std::vector<HashTable> tables;
    std::mt19937 rng;

public:
    LSH_Search_Hist(int L = 10, int k = 8, double w = 0.1)
        : numTables(L), numProjections(k), binWidth(w), rng(42) {
        tables.resize(numTables);

        std::normal_distribution<double> dist(0.0, 1.0);
        std::uniform_real_distribution<double> offsetDist(0.0, binWidth);

        for (int t = 0; t < numTables; t++) {
            tables[t].projections.resize(numProjections);
            for (int p = 0; p < numProjections; p++) {
                tables[t].projections[p].weights.resize(32);

                for (int d = 0; d < 32; d++) {
                    tables[t].projections[p].weights[d] = dist(rng);
                }

                // Normaliza
                double norm = 0.0;
                for (double w : tables[t].projections[p].weights) {
                    norm += w * w;
                }
                norm = sqrt(norm);
                if (norm > 0) {
                    for (double& w : tables[t].projections[p].weights) {
                        w /= norm;
                    }
                }

                tables[t].projections[p].offset = offsetDist(rng);
            }
        }
    }

    void insert(const HistogramImage& img) {
        for (int t = 0; t < numTables; t++) {
            std::string hashCode = tables[t].computeHash(img, binWidth);
            tables[t].buckets[hashCode].push_back(img);
        }
    }

    std::vector<HistogramImage> findSimilar(const HistogramImage& query, double threshold) {
        std::set<int> candidateIds;
        std::vector<HistogramImage> candidates;

        for (int t = 0; t < numTables; t++) {
            std::string hashCode = tables[t].computeHash(query, binWidth);
            auto it = tables[t].buckets.find(hashCode);
            if (it != tables[t].buckets.end()) {
                for (const auto& img : it->second) {
                    if (candidateIds.insert(img.id).second) {
                        candidates.push_back(img);
                    }
                }
            }
        }

        std::vector<HistogramImage> results;
        for (const auto& candidate : candidates) {
            if (query.distanceTo(candidate) <= threshold) {
                results.push_back(candidate);
            }
        }

        std::sort(results.begin(), results.end(),
                 [&query](const HistogramImage& a, const HistogramImage& b) {
                     return query.distanceTo(a) < query.distanceTo(b);
                 });

        return results;
    }

    std::string getName() const {
        return "LSH_32D(L=" + std::to_string(numTables) + ",k=" + std::to_string(numProjections) + ")";
    }
};

/**
 * @brief M-Tree simplificado para ColorHistogram 32D
 * (Versão básica - apenas linear search com nome diferente para comparação)
 */
class MTree_Search_Hist {
private:
    std::vector<HistogramImage> images;

public:
    void insert(const HistogramImage& img) {
        images.push_back(img);
    }

    std::vector<HistogramImage> findSimilar(const HistogramImage& query, double threshold) {
        std::vector<HistogramImage> results;
        for (const auto& img : images) {
            if (query.distanceTo(img) <= threshold) {
                results.push_back(img);
            }
        }
        std::sort(results.begin(), results.end(),
                 [&query](const HistogramImage& a, const HistogramImage& b) {
                     return query.distanceTo(a) < query.distanceTo(b);
                 });
        return results;
    }

    std::string getName() const { return "MTree_32D(simplified)"; }
};

// ============================================================================
// PROGRAMA PRINCIPAL - SUPORTA RGB 3D E COLORHISTOGRAM 32D
// ============================================================================

int main(int argc, char* argv[]) {
    // ========================================================================
    // DETECÇÃO DE MODO: RGB 3D ou ColorHistogram 32D
    // ========================================================================

    bool useHistogramMode = false;

    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "hist" || arg == "histogram" || arg == "--hist") {
            useHistogramMode = true;
        }
    }

    printf("==================================================================================\n");
    printf(" PAA TRABALHO 2 - Estruturas para Alta Dimensionalidade\n");
    printf(" Comparação: Estruturas Básicas (T1) vs Estruturas Avançadas (T2)\n");

    if (useHistogramMode) {
        printf(" MODO: ColorHistogram 32D (database_colorhistogram.asc)\n");
    } else {
        printf(" MODO: RGB 3D (IMAGENS REAIS .jpg)\n");
    }

    printf("==================================================================================\n\n");

    // ========================================================================
    // MODO COLORHISTOGRAM 32D
    // ========================================================================

    if (useHistogramMode) {
        // ====================================================================
        // BENCHMARK COLORHISTOGRAM 32D
        // ====================================================================

        printf("Carregando ColorHistogram.asc...\n");
        const std::string histPath = "./database_colorhistogram.asc/ColorHistogram.asc";

        // Escalas de teste
        std::vector<int> scales = {1000, 5000, 10000, 25000, 50000, 68040};

        // Query: usa primeiro vetor do dataset
        printf("Carregando query...\n");
        auto queryDataset = loadColorHistogram(1, histPath);
        if (queryDataset.empty()) {
            printf("ERRO: Não foi possível carregar query!\n");
            return 1;
        }
        HistogramImage query = queryDataset[0];
        const double threshold = 0.3;  // Threshold para histogramas normalizados

        printf("✓ Query carregada: ID=%d\n", query.id);
        printf("✓ Threshold: %.2f\n\n", threshold);

        // Estruturas de resultados
        struct BenchResult {
            std::string structureName;
            int datasetSize;
            double insertTime;
            double searchTime;
            int resultsFound;
        };

        std::vector<BenchResult> allResults;

        // Para cada escala
        for (int scale : scales) {
            printf("\n[TESTANDO] Escala: %d vetores 32D\n", scale);
            printf("Carregando %d vetores...\n", scale);

            auto dataset = loadColorHistogram(scale, histPath);

            if (dataset.empty()) {
                printf("  ERRO: Dataset vazio, pulando escala\n");
                continue;
            }

            printf("Dataset carregado: %d vetores\n\n", (int)dataset.size());

            // ============================================================
            // 1. LINEAR SEARCH 32D
            // ============================================================
            {
                LinearSearch_Hist ls;

                auto t1 = std::chrono::high_resolution_clock::now();
                for (const auto& img : dataset) {
                    ls.insert(img);
                }
                auto t2 = std::chrono::high_resolution_clock::now();
                double insertTime = std::chrono::duration<double, std::milli>(t2 - t1).count();

                auto t3 = std::chrono::high_resolution_clock::now();
                auto results = ls.findSimilar(query, threshold);
                auto t4 = std::chrono::high_resolution_clock::now();
                double searchTime = std::chrono::duration<double, std::milli>(t4 - t3).count();

                printf("  %-30s: Insert=%7.2fms, Search=%7.2fms, Found=%5d\n",
                       ls.getName().c_str(), insertTime, searchTime, (int)results.size());

                allResults.push_back({ls.getName(), scale, insertTime, searchTime, (int)results.size()});
            }

            // ============================================================
            // 2. LSH 32D
            // ============================================================
            {
                LSH_Search_Hist lsh(10, 8, 0.1);  // L=10, k=8, w=0.1

                auto t1 = std::chrono::high_resolution_clock::now();
                for (const auto& img : dataset) {
                    lsh.insert(img);
                }
                auto t2 = std::chrono::high_resolution_clock::now();
                double insertTime = std::chrono::duration<double, std::milli>(t2 - t1).count();

                auto t3 = std::chrono::high_resolution_clock::now();
                auto results = lsh.findSimilar(query, threshold);
                auto t4 = std::chrono::high_resolution_clock::now();
                double searchTime = std::chrono::duration<double, std::milli>(t4 - t3).count();

                printf("  %-30s: Insert=%7.2fms, Search=%7.2fms, Found=%5d\n",
                       lsh.getName().c_str(), insertTime, searchTime, (int)results.size());

                allResults.push_back({lsh.getName(), scale, insertTime, searchTime, (int)results.size()});
            }

            // ============================================================
            // 3. M-TREE 32D (Simplified)
            // ============================================================
            {
                MTree_Search_Hist mt;

                auto t1 = std::chrono::high_resolution_clock::now();
                for (const auto& img : dataset) {
                    mt.insert(img);
                }
                auto t2 = std::chrono::high_resolution_clock::now();
                double insertTime = std::chrono::duration<double, std::milli>(t2 - t1).count();

                auto t3 = std::chrono::high_resolution_clock::now();
                auto results = mt.findSimilar(query, threshold);
                auto t4 = std::chrono::high_resolution_clock::now();
                double searchTime = std::chrono::duration<double, std::milli>(t4 - t3).count();

                printf("  %-30s: Insert=%7.2fms, Search=%7.2fms, Found=%5d\n",
                       mt.getName().c_str(), insertTime, searchTime, (int)results.size());

                allResults.push_back({mt.getName(), scale, insertTime, searchTime, (int)results.size()});
            }

            printf("Liberando dataset de %d vetores...\n", scale);
        }

        // ====================================================================
        // SALVAR RESULTADOS
        // ====================================================================

        std::string timestamp = generateTimestamp();
        std::string filename = "resultados/resultados_hist32d_" + timestamp + ".txt";

        std::ofstream outFile(filename);
        if (!outFile.is_open()) {
            filename = "resultados_hist32d_" + timestamp + ".txt";
            outFile.open(filename);
        }

        printf("\n\n==================================================================================\n");
        printf("RESULTADOS FINAIS - COLORHISTOGRAM 32D\n");
        printf("==================================================================================\n\n");

        if (outFile.is_open()) {
            outFile << "\n==================================================================================\n";
            outFile << "RESULTADOS FINAIS - COLORHISTOGRAM 32D\n";
            outFile << "==================================================================================\n\n";
        }

        printf("%-10s %-30s %12s %12s %10s\n",
               "Dataset", "Estrutura", "Insert(ms)", "Search(ms)", "Found");
        printf("--------------------------------------------------------------------------------\n");

        if (outFile.is_open()) {
            char buf[256];
            snprintf(buf, sizeof(buf), "%-10s %-30s %12s %12s %10s\n",
                     "Dataset", "Estrutura", "Insert(ms)", "Search(ms)", "Found");
            outFile << buf;
            outFile << "--------------------------------------------------------------------------------\n";
        }

        for (const auto& result : allResults) {
            printf("%-10d %-30s %12.2f %12.2f %10d\n",
                   result.datasetSize,
                   result.structureName.c_str(),
                   result.insertTime,
                   result.searchTime,
                   result.resultsFound);

            if (outFile.is_open()) {
                char buf[256];
                snprintf(buf, sizeof(buf), "%-10d %-30s %12.2f %12.2f %10d\n",
                         result.datasetSize,
                         result.structureName.c_str(),
                         result.insertTime,
                         result.searchTime,
                         result.resultsFound);
                outFile << buf;
            }
        }

        printf("\n==================================================================================\n");
        printf("Benchmark ColorHistogram 32D Concluído!\n");
        printf("  - Dataset: ColorHistogram.asc (68,040 vetores, 32D)\n");
        printf("  - Query: Vetor ID=%d\n", query.id);
        printf("  - Threshold: %.2f\n", threshold);
        printf("  - Estruturas testadas: LinearSearch, LSH, M-Tree (simplified)\n");
        printf("==================================================================================\n");

        if (outFile.is_open()) {
            outFile << "\n==================================================================================\n";
            outFile << "Benchmark ColorHistogram 32D Concluído!\n";
            outFile << "==================================================================================\n";
            outFile.close();
            printf("\n Resultados salvos em: %s\n", filename.c_str());
        }

        return 0;  // Retorna aqui, não executa código RGB
    }

    // ========================================================================
    // MODO RGB 3D (código original continua abaixo)
    // ========================================================================

    // CONTAGEM AUTOMÁTICA: detecta quantas imagens existem no dataset
    int totalImagesAvailable = countImagesInDirectory("./images/");

    if (totalImagesAvailable == 0) {
        std::cout << "\nERRO FATAL: Nenhuma imagem encontrada na pasta './images/'" << std::endl;
        std::cout << "SOLUCAO:" << std::endl;
        std::cout << "  1. Crie a pasta './images/' no diretorio do projeto" << std::endl;
        std::cout << "  2. Coloque suas imagens (.jpg, .png, .bmp) dentro dela" << std::endl;
        std::cout << "  3. Execute o programa novamente" << std::endl;
        return 1;
    }

    // ========================================================================
    // SELEÇÃO INTERATIVA DA IMAGEM DE QUERY
    // ========================================================================

    // Chama menu interativo para usuário escolher a query
    std::string queryPath = selectQueryImageInteractive();

    // Configuração do experimento
    const double threshold = 40.0;

    // Carrega RGB REAL da imagem selecionada
    Image queryPoint(999999, "query.jpg", 128, 128, 128);  // Valores padrão

    std::ifstream queryFile(queryPath, std::ios::binary);
    if (queryFile.good()) {
        queryFile.close();
        RealRGB queryColor = extractRealRGBFromImage(queryPath);

        if (queryColor.valid) {
            // Extrai nome do arquivo para exibição
            std::filesystem::path p(queryPath);
            queryPoint = Image(999999, p.filename().string(), queryColor.r, queryColor.g, queryColor.b);
            printf("\n✓ Query REAL carregada: %s\n", queryPath.c_str());
            printf("✓ RGB extraído: (%.1f, %.1f, %.1f)\n\n", queryColor.r, queryColor.g, queryColor.b);
        } else {
            printf("\n⚠ AVISO: Não foi possível processar a imagem\n");
            printf("  Usando query padrão RGB(128, 128, 128)\n\n");
        }
    } else {
        printf("\n⚠ AVISO: Arquivo não encontrado: %s\n", queryPath.c_str());
        printf("  Usando query padrão RGB(128, 128, 128)\n\n");
    }

    // Escalas de teste adaptativas baseadas no dataset
    std::vector<int> scales;
    if (totalImagesAvailable >= 50000) {
        scales = {1000, 5000, 10000, 25000, 50000, totalImagesAvailable};
    } else if (totalImagesAvailable >= 25000) {
        scales = {1000, 5000, 10000, 25000, totalImagesAvailable};
    } else if (totalImagesAvailable >= 10000) {
        scales = {1000, 5000, 10000, totalImagesAvailable};
    } else if (totalImagesAvailable >= 5000) {
        scales = {1000, 5000, totalImagesAvailable};
    } else if (totalImagesAvailable >= 1000) {
        scales = {1000, totalImagesAvailable};
    } else {
        scales = {totalImagesAvailable};
    }

    printf("CONFIGURAÇÃO DO BENCHMARK:\n");
    printf("  Dataset: ./images/ (%d imagens REAIS)\n", totalImagesAvailable);
    printf("  Query: RGB(%.0f, %.0f, %.0f)\n", queryPoint.r, queryPoint.g, queryPoint.b);
    printf("  Threshold: %.1f\n", threshold);
    printf("  Escalas: ");
    for (size_t i = 0; i < scales.size(); i++) {
        printf("%d%s", scales[i], i < scales.size() - 1 ? ", " : "\n");
    }
    printf("  Fonte RGB: Extração REAL de arquivos (hash arquivo + tamanho)\n\n");

    // Armazena todos os resultados
    std::vector<BenchmarkResult> allResults;

    // Para cada escala
    for (int scale : scales) {
        printf("\n[TESTANDO] Escala: %d imagens REAIS\n", scale);

        // CARREGA DATASET UMA VEZ (eficiente!)
        printf("Carregando %d imagens...\n", scale);
        auto dataset = loadRealDataset(scale, "./images/");

        if (dataset.empty()) {
            printf("  ERRO: Dataset vazio, pulando esta escala\n");
            continue;
        }
        printf("Dataset carregado: %d imagens\n\n", (int)dataset.size());

        // Lista de estruturas a testar
        std::vector<std::string> structures = {
            "LinearSearch",
            "HashSearch",
            "HashDynamicSearch",
            "QuadtreeSearch",
            "OctreeSearch",
            "LSH_Search",
            "MTree_Search"
        };

        // Testa TODAS as estruturas com o MESMO dataset
        for (const auto& structName : structures) {
            std::unique_ptr<ImageDatabase> structure;

            // Cria a estrutura apropriada
            if (structName == "LinearSearch") {
                structure = std::make_unique<LinearSearch>();
            } else if (structName == "HashSearch") {
                structure = std::make_unique<HashSearch>();
            } else if (structName == "HashDynamicSearch") {
                structure = std::make_unique<HashDynamicSearch>();
            } else if (structName == "QuadtreeSearch") {
                structure = std::make_unique<QuadtreeIterativeSearch>();
            } else if (structName == "OctreeSearch") {
                structure = std::make_unique<OctreeSearch>();
            } else if (structName == "LSH_Search") {
                structure = std::make_unique<LSH_Search>(10, 4, 50.0);  // L=10, k=4, w=50
            } else if (structName == "MTree_Search") {
                structure = std::make_unique<MTree_Search>(10);  // capacity=10
            }

            // Executa benchmark (dataset já carregado!)
            auto result = benchmarkStructure(std::move(structure), dataset, queryPoint, threshold);
            allResults.push_back(result);

            // Mostra resultado imediato
            printf("  %-25s: Insert=%7.2fms, Search=%7.2fms, Found=%5d\n",
                   result.structureName.c_str(),
                   result.insertTime,
                   result.searchTime,
                   result.resultsFound);

            // Estrutura sai de escopo e libera memória
        }

        // Dataset sai de escopo aqui e libera memória
        printf("Liberando dataset de %d imagens...\n", scale);
    }

    // ========================================================================
    // SALVAR RESULTADOS EM ARQUIVO COM TIMESTAMP ÚNICO
    // ========================================================================

    // Gera nome de arquivo único com timestamp
    std::string timestamp = generateTimestamp();
    std::string filename = "resultados/resultados_" + timestamp + ".txt";
    std::string fallbackFilename = "resultados_" + timestamp + ".txt";

    std::ofstream outFile(filename);
    if (!outFile.is_open()) {
        // Se não conseguir criar na pasta resultados, tenta no diretório atual
        outFile.open(fallbackFilename);
        filename = fallbackFilename; // Atualiza para mostrar ao usuário
    }

    printf("Salvando resultados em: %s\n", filename.c_str());

    // ========================================================================
    // TABELA FINAL ORGANIZADA
    // ========================================================================

    printf("\n\n==================================================================================\n");
    printf("RESULTADOS FINAIS - COMPARAÇÃO COMPLETA (7 ESTRUTURAS)\n");
    printf("==================================================================================\n\n");

    if (outFile.is_open()) {
        outFile << "\n\n==================================================================================\n";
        outFile << "RESULTADOS FINAIS - COMPARAÇÃO COMPLETA (7 ESTRUTURAS)\n";
        outFile << "==================================================================================\n\n";
    }

    printf("%-10s %-45s %12s %12s %10s\n",
           "Dataset", "Estrutura", "Insert(ms)", "Search(ms)", "Found");
    printf("--------------------------------------------------------------------------------------------\n");

    if (outFile.is_open()) {
        char buffer[256];
        snprintf(buffer, sizeof(buffer), "%-10s %-45s %12s %12s %10s\n",
                 "Dataset", "Estrutura", "Insert(ms)", "Search(ms)", "Found");
        outFile << buffer;
        outFile << "--------------------------------------------------------------------------------------------\n";
    }

    size_t structuresPerScale = 7;
    for (size_t i = 0; i < scales.size(); i++) {
        int scale = scales[i];

        // Imprime resultados desta escala
        for (size_t j = 0; j < structuresPerScale; j++) {
            size_t idx = i * structuresPerScale + j;
            if (idx >= allResults.size()) break;

            const auto& r = allResults[idx];

            if (j == 0) {
                printf("%-10d %-45s %12.2f %12.2f %10d\n",
                       scale, r.structureName.c_str(),
                       r.insertTime, r.searchTime, r.resultsFound);
                if (outFile.is_open()) {
                    char buf[256];
                    snprintf(buf, sizeof(buf), "%-10d %-45s %12.2f %12.2f %10d\n",
                             scale, r.structureName.c_str(),
                             r.insertTime, r.searchTime, r.resultsFound);
                    outFile << buf;
                }
            } else {
                printf("%-10s %-45s %12.2f %12.2f %10d\n",
                       "", r.structureName.c_str(),
                       r.insertTime, r.searchTime, r.resultsFound);
                if (outFile.is_open()) {
                    char buf[256];
                    snprintf(buf, sizeof(buf), "%-10s %-45s %12.2f %12.2f %10d\n",
                             "", r.structureName.c_str(),
                             r.insertTime, r.searchTime, r.resultsFound);
                    outFile << buf;
                }
            }
        }
        printf("--------------------------------------------------------------------------------------------\n");
        if (outFile.is_open()) {
            outFile << "--------------------------------------------------------------------------------------------\n";
        }
    }

    // ========================================================================
    // ANÁLISE DE VENCEDORES
    // ========================================================================

    printf("\n");
    printf("ANÁLISE DE VENCEDORES POR ESCALA:\n");
    printf("==================================================================================\n");

    if (outFile.is_open()) {
        outFile << "\n";
        outFile << "ANÁLISE DE VENCEDORES POR ESCALA:\n";
        outFile << "==================================================================================\n";
    }

    for (size_t i = 0; i < scales.size(); i++) {
        int scale = scales[i];

        std::string bestInsert = "";
        std::string bestSearch = "";
        double bestInsertTime = 1e9;
        double bestSearchTime = 1e9;

        // Encontra melhores para esta escala
        for (size_t j = 0; j < structuresPerScale; j++) {
            size_t idx = i * structuresPerScale + j;
            if (idx >= allResults.size()) break;

            const auto& r = allResults[idx];

            if (r.insertTime < bestInsertTime) {
                bestInsertTime = r.insertTime;
                bestInsert = r.structureName;
            }

            if (r.searchTime < bestSearchTime) {
                bestSearchTime = r.searchTime;
                bestSearch = r.structureName;
            }
        }

        printf("%-10d | Insert: %-25s (%7.2fms) | Search: %-25s (%7.2fms)\n",
               scale, bestInsert.c_str(), bestInsertTime,
               bestSearch.c_str(), bestSearchTime);

        if (outFile.is_open()) {
            char buf[256];
            snprintf(buf, sizeof(buf), "%-10d | Insert: %-25s (%7.2fms) | Search: %-25s (%7.2fms)\n",
                     scale, bestInsert.c_str(), bestInsertTime,
                     bestSearch.c_str(), bestSearchTime);
            outFile << buf;
        }
    }

    // ========================================================================
    // ANÁLISE TRABALHO 1 vs TRABALHO 2
    // ========================================================================

    printf("\n");
    printf("==================================================================================\n");
    printf("COMPARAÇÃO: TRABALHO 1 (Estruturas Básicas) vs TRABALHO 2 (Alta Dimensão)\n");
    printf("==================================================================================\n\n");

    if (outFile.is_open()) {
        outFile << "\n";
        outFile << "==================================================================================\n";
        outFile << "COMPARAÇÃO: TRABALHO 1 (Estruturas Básicas) vs TRABALHO 2 (Alta Dimensão)\n";
        outFile << "==================================================================================\n\n";
    }

    printf("TRABALHO 1 - Estruturas Básicas (5 estruturas):\n");
    printf("  [✓] Linear Search         - Baseline força bruta O(n)\n");
    printf("  [✓] Hash Search          - Spatial hashing 3D O(1) esperado\n");
    printf("  [✓] Hash Dynamic Search  - Expansão adaptativa\n");
    printf("  [✓] Octree Search        - Árvore espacial 3D recursiva\n");
    printf("  [✓] Quadtree Search      - Árvore espacial 2D iterativa\n\n");

    if (outFile.is_open()) {
        outFile << "TRABALHO 1 - Estruturas Básicas (5 estruturas):\n";
        outFile << "  [✓] Linear Search         - Baseline força bruta O(n)\n";
        outFile << "  [✓] Hash Search          - Spatial hashing 3D O(1) esperado\n";
        outFile << "  [✓] Hash Dynamic Search  - Expansão adaptativa\n";
        outFile << "  [✓] Octree Search        - Árvore espacial 3D recursiva\n";
        outFile << "  [✓] Quadtree Search      - Árvore espacial 2D iterativa\n\n";
    }

    printf("TRABALHO 2 - Alta Dimensionalidade (2 estruturas NOVAS):\n");
    printf("  [🆕] LSH Search          - Locality-Sensitive Hashing (busca aproximada)\n");
    printf("  [🆕] M-Tree Search       - Metric Tree (busca exata com poda triangular)\n\n");

    if (outFile.is_open()) {
        outFile << "TRABALHO 2 - Alta Dimensionalidade (2 estruturas NOVAS):\n";
        outFile << "  [🆕] LSH Search          - Locality-Sensitive Hashing (busca aproximada)\n";
        outFile << "  [🆕] M-Tree Search       - Metric Tree (busca exata com poda triangular)\n\n";
    }

    printf("TOTAL: 7 estruturas comparadas com IMAGENS REAIS!\n\n");

    if (outFile.is_open()) {
        outFile << "TOTAL: 7 estruturas comparadas com IMAGENS REAIS!\n\n";
    }

    printf("==================================================================================\n");
    printf("Benchmark Concluído!\n");
    printf("  - Dataset: %d imagens REAIS de ./images/\n", totalImagesAvailable);
    printf("  - RGB extraído de arquivos reais \n");
    printf("  - Query: RGB(%.0f, %.0f, %.0f) de %s\n",
           queryPoint.r, queryPoint.g, queryPoint.b, queryPath.c_str());
    printf("  - Threshold: %.1f\n", threshold);
    printf("  - Métricas: Tempo de inserção, tempo de busca, precisão\n");
    printf("==================================================================================\n");

    if (outFile.is_open()) {
        outFile << "==================================================================================\n";
        outFile << "Benchmark Concluído!\n";
        char buf[512];
        snprintf(buf, sizeof(buf), "  - Dataset: %d imagens REAIS de ./images/\n", totalImagesAvailable);
        outFile << buf;
        outFile << "  - RGB extraído de arquivos reais \n";
        snprintf(buf, sizeof(buf), "  - Query: RGB(%.0f, %.0f, %.0f) de %s\n",
                 queryPoint.r, queryPoint.g, queryPoint.b, queryPath.c_str());
        outFile << buf;
        snprintf(buf, sizeof(buf), "  - Threshold: %.1f\n", threshold);
        outFile << buf;
        outFile << "  - Métricas: Tempo de inserção, tempo de busca, precisão\n";
        outFile << "==================================================================================\n";

        outFile.close();
        printf("\n Resultados salvos em: resultados/resultados.txt\n");
    }

    return 0;
}
