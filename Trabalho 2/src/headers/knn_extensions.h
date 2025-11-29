#ifndef KNN_EXTENSIONS_H
#define KNN_EXTENSIONS_H

#include "image_database.h"
#include "config.h"
#include <vector>
#include <algorithm>
#include <queue>
#include <functional>

/**
 * @brief Interface estendida com suporte a k-NN query
 *
 * Adiciona método findKNearest() além do findSimilar() existente.
 * Permite busca dos k vizinhos mais próximos, comum na literatura.
 */
class ImageDatabaseKNN : public ImageDatabase {
public:
    /**
     * @brief Busca k vizinhos mais próximos
     *
     * @param query Imagem de consulta
     * @param k Número de vizinhos a retornar
     * @return Vetor com k imagens mais próximas, ordenadas por distância
     */
    virtual std::vector<Image> findKNearest(const Image& query, int k) = 0;

    /**
     * @brief Implementação padrão de k-NN usando findSimilar
     *
     * Se uma estrutura não implementar k-NN nativamente,
     * pode usar este helper que converte range query em k-NN.
     */
    std::vector<Image> findKNearestFromRange(
        const Image& query,
        int k,
        double initialThreshold = 100.0
    ) {
        // Busca com threshold inicial
        std::vector<Image> candidates = findSimilar(query, initialThreshold);

        // Se não encontrou k resultados, expande threshold
        while (candidates.size() < static_cast<size_t>(k) && initialThreshold < 500.0) {
            initialThreshold *= 2.0;
            candidates = findSimilar(query, initialThreshold);
        }

        // Calcula distâncias
        std::vector<std::pair<double, Image>> distances;
        distances.reserve(candidates.size());

        for (const auto& candidate : candidates) {
            double dist = query.distanceTo(candidate);
            distances.push_back({dist, candidate});
        }

        // Ordena por distância
        std::partial_sort(distances.begin(),
                         distances.begin() + std::min(k, static_cast<int>(distances.size())),
                         distances.end(),
                         [](const auto& a, const auto& b) {
                             return a.first < b.first;
                         });

        // Retorna k primeiros
        std::vector<Image> results;
        int numResults = std::min(k, static_cast<int>(distances.size()));
        results.reserve(numResults);

        for (int i = 0; i < numResults; i++) {
            results.push_back(distances[i].second);
        }

        return results;
    }
};

/**
 * @brief Heap-based k-NN query
 *
 * Implementação eficiente de k-NN usando max-heap.
 * Mantém apenas k elementos durante a busca, evitando ordenar todos.
 */
class KNNHeap {
private:
    struct DistanceImage {
        double distance;
        Image image;

        bool operator<(const DistanceImage& other) const {
            return distance < other.distance;  // Min-heap
        }

        bool operator>(const DistanceImage& other) const {
            return distance > other.distance;  // Max-heap
        }
    };

    int k;
    std::priority_queue<DistanceImage, std::vector<DistanceImage>, std::less<DistanceImage>> heap;

public:
    explicit KNNHeap(int k_) : k(k_) {}

    /**
     * @brief Adiciona candidato ao heap
     */
    void add(const Image& img, double distance) {
        if (heap.size() < static_cast<size_t>(k)) {
            // Heap não está cheio, adiciona
            heap.push({distance, img});
        } else if (distance < heap.top().distance) {
            // Candidato é melhor que o pior no heap
            heap.pop();
            heap.push({distance, img});
        }
    }

    /**
     * @brief Retorna k vizinhos mais próximos
     */
    std::vector<Image> getResults() {
        std::vector<DistanceImage> temp;
        temp.reserve(heap.size());

        // Extrai elementos do heap
        while (!heap.empty()) {
            temp.push_back(heap.top());
            heap.pop();
        }

        // Ordena em ordem crescente
        std::sort(temp.begin(), temp.end());

        // Extrai apenas as imagens
        std::vector<Image> results;
        results.reserve(temp.size());
        for (const auto& di : temp) {
            results.push_back(di.image);
        }

        return results;
    }

    /**
     * @brief Retorna distância do k-ésimo elemento (threshold atual)
     */
    double getCurrentThreshold() const {
        return heap.empty() ? std::numeric_limits<double>::max() : heap.top().distance;
    }

    /**
     * @brief Verifica se heap está cheio
     */
    bool isFull() const {
        return heap.size() >= static_cast<size_t>(k);
    }
};

/**
 * @brief Wrappers para adicionar k-NN às estruturas existentes
 */

// Exemplo de como usar para Linear Search
inline std::vector<Image> linearSearchKNN(
    const std::vector<Image>& dataset,
    const Image& query,
    int k
) {
    KNNHeap heap(k);

    for (const auto& img : dataset) {
        double dist = query.distanceTo(img);
        heap.add(img, dist);
    }

    return heap.getResults();
}

/**
 * @brief Comparador de benchmarks: Range Query vs k-NN
 */
class QueryComparator {
public:
    struct ComparisonResult {
        std::vector<Image> rangeResults;
        std::vector<Image> knnResults;
        double rangeTime;
        double knnTime;
        int rangeCount;
        int knnCount;
        double overlap;  // Percentual de sobreposição

        void print() const {
            std::cout << "\n========================================\n";
            std::cout << "RANGE QUERY vs k-NN COMPARISON\n";
            std::cout << "========================================\n";
            std::cout << "Range Query Results: " << rangeCount << " (" << rangeTime << " ms)\n";
            std::cout << "k-NN Results: " << knnCount << " (" << knnTime << " ms)\n";
            std::cout << "Overlap: " << std::fixed << std::setprecision(2)
                      << (overlap * 100) << "%\n";
            std::cout << "========================================\n";
        }
    };

    /**
     * @brief Compara resultados de range query e k-NN
     */
    static ComparisonResult compare(
        const std::vector<Image>& rangeResults,
        const std::vector<Image>& knnResults,
        double rangeTime,
        double knnTime
    ) {
        ComparisonResult result;
        result.rangeResults = rangeResults;
        result.knnResults = knnResults;
        result.rangeTime = rangeTime;
        result.knnTime = knnTime;
        result.rangeCount = rangeResults.size();
        result.knnCount = knnResults.size();

        // Calcula sobreposição
        std::set<int> rangeIds, knnIds;
        for (const auto& img : rangeResults) rangeIds.insert(img.id);
        for (const auto& img : knnResults) knnIds.insert(img.id);

        int common = 0;
        for (int id : rangeIds) {
            if (knnIds.count(id)) common++;
        }

        int total = rangeIds.size() + knnIds.size() - common;
        result.overlap = (total > 0) ? static_cast<double>(common) / total : 1.0;

        return result;
    }
};

#endif // KNN_EXTENSIONS_H
