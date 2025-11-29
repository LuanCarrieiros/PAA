#ifndef LSH_SEARCH_IMPROVED_H
#define LSH_SEARCH_IMPROVED_H

#include "image_database.h"
#include "config.h"
#include <unordered_map>
#include <vector>
#include <random>
#include <algorithm>
#include <cmath>
#include <string>
#include <set>

/**
 * @brief LSH com código refatorado e helper functions
 *
 * Versão melhorada do LSH_Search com:
 * - Funções auxiliares extraídas
 * - Código mais legível e modular
 * - Uso de Config centralizado
 * - Melhor separação de responsabilidades
 */
class LSH_Search_Improved : public ImageDatabase {
private:
    // Parâmetros do LSH
    int numTables;
    int numProjections;
    double binWidth;
    int totalImages;

    // Estrutura de uma projeção aleatória
    struct Projection {
        double r, g, b;
        double offset;

        Projection() : r(0), g(0), b(0), offset(0) {}

        // Projeta e quantiza uma imagem
        int project(const Image& img, double binWidth) const {
            double projection = computeDotProduct(img) + offset;
            return quantize(projection, binWidth);
        }

    private:
        // Helper: calcula produto escalar
        double computeDotProduct(const Image& img) const {
            return r * img.r + g * img.g + b * img.b;
        }

        // Helper: quantiza projeção em bins
        static int quantize(double value, double binWidth) {
            return static_cast<int>(floor(value / binWidth));
        }
    };

    // Tabela hash individual
    struct HashTable {
        std::vector<Projection> projections;
        std::unordered_map<std::string, std::vector<Image>> buckets;

        // Computa hash code para uma imagem
        std::string computeHash(const Image& img, double binWidth) const {
            return buildHashCode(img, binWidth);
        }

        // Obtém bucket para um hash code
        const std::vector<Image>* getBucket(const std::string& hashCode) const {
            auto it = buckets.find(hashCode);
            return (it != buckets.end()) ? &(it->second) : nullptr;
        }

        // Insere imagem em um bucket
        void insertIntoBucket(const std::string& hashCode, const Image& img) {
            buckets[hashCode].push_back(img);
        }

    private:
        // Helper: constrói string do hash code
        std::string buildHashCode(const Image& img, double binWidth) const {
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

    // ==========================================================================
    // HELPER FUNCTIONS - Inicialização
    // ==========================================================================

    /**
     * @brief Inicializa todas as tabelas hash
     */
    void initializeTables() {
        std::normal_distribution<double> dist(0.0, 1.0);
        std::uniform_real_distribution<double> offsetDist(0.0, binWidth);

        for (int t = 0; t < numTables; t++) {
            initializeTableProjections(t, dist, offsetDist);
        }
    }

    /**
     * @brief Inicializa projeções de uma tabela específica
     */
    void initializeTableProjections(
        int tableIndex,
        std::normal_distribution<double>& dist,
        std::uniform_real_distribution<double>& offsetDist
    ) {
        tables[tableIndex].projections.resize(numProjections);

        for (int p = 0; p < numProjections; p++) {
            generateRandomProjection(tableIndex, p, dist, offsetDist);
        }
    }

    /**
     * @brief Gera uma projeção aleatória
     */
    void generateRandomProjection(
        int tableIndex,
        int projIndex,
        std::normal_distribution<double>& dist,
        std::uniform_real_distribution<double>& offsetDist
    ) {
        auto& proj = tables[tableIndex].projections[projIndex];

        // Gera vetor gaussiano
        proj.r = dist(rng);
        proj.g = dist(rng);
        proj.b = dist(rng);

        // Normaliza
        normalizeProjection(proj);

        // Adiciona offset
        proj.offset = offsetDist(rng);
    }

    /**
     * @brief Normaliza um vetor de projeção
     */
    static void normalizeProjection(Projection& proj) {
        double norm = computeNorm(proj);
        if (norm > 0) {
            proj.r /= norm;
            proj.g /= norm;
            proj.b /= norm;
        }
    }

    /**
     * @brief Calcula norma euclidiana de uma projeção
     */
    static double computeNorm(const Projection& proj) {
        return sqrt(proj.r * proj.r + proj.g * proj.g + proj.b * proj.b);
    }

    // ==========================================================================
    // HELPER FUNCTIONS - Inserção
    // ==========================================================================

    /**
     * @brief Insere imagem em uma tabela específica
     */
    void insertIntoTable(int tableIndex, const Image& img) {
        std::string hashCode = tables[tableIndex].computeHash(img, binWidth);
        tables[tableIndex].insertIntoBucket(hashCode, img);
    }

    // ==========================================================================
    // HELPER FUNCTIONS - Busca
    // ==========================================================================

    /**
     * @brief Coleta candidatos de todas as tabelas
     */
    std::vector<Image> collectCandidates(const Image& query) const {
        std::set<int> candidateIds;
        std::vector<Image> candidates;

        for (int t = 0; t < numTables; t++) {
            addCandidatesFromTable(t, query, candidateIds, candidates);
        }

        return candidates;
    }

    /**
     * @brief Adiciona candidatos de uma tabela específica
     */
    void addCandidatesFromTable(
        int tableIndex,
        const Image& query,
        std::set<int>& candidateIds,
        std::vector<Image>& candidates
    ) const {
        std::string hashCode = tables[tableIndex].computeHash(query, binWidth);
        const auto* bucket = tables[tableIndex].getBucket(hashCode);

        if (bucket != nullptr) {
            addImagesFromBucket(*bucket, candidateIds, candidates);
        }
    }

    /**
     * @brief Adiciona imagens de um bucket aos candidatos
     */
    static void addImagesFromBucket(
        const std::vector<Image>& bucket,
        std::set<int>& candidateIds,
        std::vector<Image>& candidates
    ) {
        for (const auto& img : bucket) {
            // Evita duplicatas
            if (candidateIds.insert(img.id).second) {
                candidates.push_back(img);
            }
        }
    }

    /**
     * @brief Filtra candidatos por threshold
     */
    std::vector<Image> filterByThreshold(
        const std::vector<Image>& candidates,
        const Image& query,
        double threshold
    ) const {
        std::vector<Image> results;
        results.reserve(candidates.size());

        for (const auto& candidate : candidates) {
            if (isWithinThreshold(candidate, query, threshold)) {
                results.push_back(candidate);
            }
        }

        return results;
    }

    /**
     * @brief Verifica se imagem está dentro do threshold
     */
    static bool isWithinThreshold(
        const Image& img,
        const Image& query,
        double threshold
    ) {
        return query.distanceTo(img) <= threshold;
    }

    /**
     * @brief Ordena resultados por distância
     */
    void sortByDistance(std::vector<Image>& results, const Image& query) const {
        std::sort(results.begin(), results.end(),
            [&query](const Image& a, const Image& b) {
                return query.distanceTo(a) < query.distanceTo(b);
            }
        );
    }

    // ==========================================================================
    // HELPER FUNCTIONS - k-NN
    // ==========================================================================

    /**
     * @brief Seleciona k vizinhos mais próximos
     */
    std::vector<Image> selectKNearest(
        const std::vector<Image>& candidates,
        const Image& query,
        int k
    ) const {
        // Calcula distâncias
        std::vector<std::pair<double, Image>> distances;
        distances.reserve(candidates.size());

        for (const auto& candidate : candidates) {
            double dist = query.distanceTo(candidate);
            distances.push_back({dist, candidate});
        }

        // Ordena por distância
        std::sort(distances.begin(), distances.end(),
            [](const auto& a, const auto& b) {
                return a.first < b.first;
            }
        );

        // Retorna k primeiros
        std::vector<Image> results;
        int numResults = std::min(k, static_cast<int>(distances.size()));
        results.reserve(numResults);

        for (int i = 0; i < numResults; i++) {
            results.push_back(distances[i].second);
        }

        return results;
    }

public:
    /**
     * @brief Construtor usando configurações centralizadas
     */
    LSH_Search_Improved()
        : LSH_Search_Improved(
            Config::LSH::DEFAULT_NUM_TABLES,
            Config::LSH::DEFAULT_NUM_PROJECTIONS,
            Config::LSH::DEFAULT_BIN_WIDTH,
            Config::LSH::DEFAULT_SEED
        ) {}

    /**
     * @brief Construtor com parâmetros personalizados
     */
    LSH_Search_Improved(int L, int k, double w, int seed = 42)
        : numTables(L), numProjections(k), binWidth(w),
          totalImages(0), rng(seed) {
        tables.resize(numTables);
        initializeTables();
    }

    // ==========================================================================
    // INTERFACE PÚBLICA
    // ==========================================================================

    void insert(const Image& img) override {
        for (int t = 0; t < numTables; t++) {
            insertIntoTable(t, img);
        }
        totalImages++;
    }

    std::vector<Image> findSimilar(const Image& query, double threshold) override {
        // Coleta candidatos
        std::vector<Image> candidates = collectCandidates(query);

        // Filtra por threshold
        std::vector<Image> results = filterByThreshold(candidates, query, threshold);

        // Ordena por distância
        sortByDistance(results, query);

        return results;
    }

    /**
     * @brief Busca k-NN (k vizinhos mais próximos)
     */
    std::vector<Image> findKNearest(const Image& query, int k) {
        std::vector<Image> candidates = collectCandidates(query);
        return selectKNearest(candidates, query, k);
    }

    std::string getName() const override {
        return "LSH Search (Improved) (L=" + std::to_string(numTables) +
               ", k=" + std::to_string(numProjections) +
               ", w=" + std::to_string(binWidth) + ")";
    }

    /**
     * @brief Estatísticas da estrutura
     */
    void printStats() const {
        printf("\n=== LSH Statistics (Improved) ===\n");
        printf("Tables (L): %d\n", numTables);
        printf("Projections per table (k): %d\n", numProjections);
        printf("Bin width (w): %.2f\n", binWidth);
        printf("Total images: %d\n", totalImages);

        printBucketStats();
    }

private:
    /**
     * @brief Imprime estatísticas dos buckets
     */
    void printBucketStats() const {
        size_t totalBuckets = 0;
        size_t maxBucketSize = 0;
        size_t totalCollisions = 0;

        for (const auto& table : tables) {
            updateBucketStats(table, totalBuckets, maxBucketSize, totalCollisions);
        }

        printBucketSummary(totalBuckets, maxBucketSize, totalCollisions);
    }

    /**
     * @brief Atualiza estatísticas dos buckets de uma tabela
     */
    static void updateBucketStats(
        const HashTable& table,
        size_t& totalBuckets,
        size_t& maxBucketSize,
        size_t& totalCollisions
    ) {
        totalBuckets += table.buckets.size();

        for (const auto& bucket : table.buckets) {
            size_t bucketSize = bucket.second.size();
            maxBucketSize = std::max(maxBucketSize, bucketSize);
            if (bucketSize > 1) {
                totalCollisions += bucketSize;
            }
        }
    }

    /**
     * @brief Imprime resumo das estatísticas
     */
    void printBucketSummary(
        size_t totalBuckets,
        size_t maxBucketSize,
        size_t totalCollisions
    ) const {
        printf("Total buckets: %zu\n", totalBuckets);
        printf("Max bucket size: %zu\n", maxBucketSize);
        printf("Images with collisions: %zu\n", totalCollisions);

        if (totalBuckets > 0) {
            double avgBucketSize = static_cast<double>(totalImages * numTables) / totalBuckets;
            double loadFactor = 100.0 * totalImages * numTables / totalBuckets;
            printf("Average bucket size: %.2f\n", avgBucketSize);
            printf("Load factor: %.2f%%\n", loadFactor);
        }
    }
};

#endif // LSH_SEARCH_IMPROVED_H
