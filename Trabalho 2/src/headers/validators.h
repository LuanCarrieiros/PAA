#ifndef VALIDATORS_H
#define VALIDATORS_H

#include "image_database.h"
#include "config.h"
#include <vector>
#include <set>
#include <algorithm>
#include <cmath>
#include <iostream>
#include <iomanip>

/**
 * @brief Ferramentas para validação e análise de resultados
 *
 * Fornece métodos para:
 * - Validar corretude dos resultados
 * - Calcular métricas de qualidade (recall, precision)
 * - Comparar diferentes estruturas
 * - Gerar relatórios de análise
 */
class ResultValidator {
public:
    /**
     * @brief Resultado de uma validação
     */
    struct ValidationResult {
        bool isValid;           // Resultados são válidos?
        double recall;          // Quantos resultados corretos foram encontrados
        double precision;       // Quantos resultados retornados são corretos
        double f1Score;         // Média harmônica de precision e recall
        int truePositives;      // Resultados corretos encontrados
        int falsePositives;     // Resultados incorretos encontrados
        int falseNegatives;     // Resultados corretos não encontrados
        int totalExpected;      // Total de resultados esperados
        int totalFound;         // Total de resultados encontrados

        ValidationResult()
            : isValid(false), recall(0.0), precision(0.0), f1Score(0.0),
              truePositives(0), falsePositives(0), falseNegatives(0),
              totalExpected(0), totalFound(0) {}

        void print() const {
            std::cout << "  ✓ Recall:     " << std::fixed << std::setprecision(4)
                      << (recall * 100) << "%\n";
            std::cout << "  ✓ Precision:  " << std::fixed << std::setprecision(4)
                      << (precision * 100) << "%\n";
            std::cout << "  ✓ F1-Score:   " << std::fixed << std::setprecision(4)
                      << (f1Score * 100) << "%\n";
            std::cout << "  ✓ TP/FP/FN:   " << truePositives << "/"
                      << falsePositives << "/" << falseNegatives << "\n";
        }
    };

    /**
     * @brief Verifica se dois resultados são equivalentes
     *
     * Compara se dois conjuntos de resultados contêm os mesmos IDs,
     * independente da ordem.
     *
     * @param groundTruth Resultado esperado (ground truth)
     * @param testResult Resultado a ser testado
     * @param tolerance Tolerância para diferenças (não usado para IDs)
     * @return true se são equivalentes, false caso contrário
     */
    static bool areEquivalent(
        const std::vector<Image>& groundTruth,
        const std::vector<Image>& testResult,
        double tolerance = Config::Validation::DISTANCE_TOLERANCE
    ) {
        (void)tolerance;  // Não usado para comparação de IDs

        if (groundTruth.size() != testResult.size()) return false;

        // Extrai IDs e ordena
        std::vector<int> truthIds = extractIds(groundTruth);
        std::vector<int> testIds = extractIds(testResult);

        std::sort(truthIds.begin(), truthIds.end());
        std::sort(testIds.begin(), testIds.end());

        return truthIds == testIds;
    }

    /**
     * @brief Calcula recall (quantos resultados corretos foram encontrados)
     *
     * Recall = TP / (TP + FN)
     * Onde TP = true positives, FN = false negatives
     *
     * @param groundTruth Resultado esperado
     * @param testResult Resultado obtido
     * @return Recall no intervalo [0.0, 1.0]
     */
    static double calculateRecall(
        const std::vector<Image>& groundTruth,
        const std::vector<Image>& testResult
    ) {
        if (groundTruth.empty()) return 1.0;

        std::set<int> truthSet = extractIdSet(groundTruth);
        std::set<int> testSet = extractIdSet(testResult);

        int truePositives = countIntersection(truthSet, testSet);

        return static_cast<double>(truePositives) / groundTruth.size();
    }

    /**
     * @brief Calcula precision (quantos resultados retornados são corretos)
     *
     * Precision = TP / (TP + FP)
     * Onde TP = true positives, FP = false positives
     *
     * @param groundTruth Resultado esperado
     * @param testResult Resultado obtido
     * @return Precision no intervalo [0.0, 1.0]
     */
    static double calculatePrecision(
        const std::vector<Image>& groundTruth,
        const std::vector<Image>& testResult
    ) {
        if (testResult.empty()) return 1.0;

        std::set<int> truthSet = extractIdSet(groundTruth);
        std::set<int> testSet = extractIdSet(testResult);

        int truePositives = countIntersection(truthSet, testSet);

        return static_cast<double>(truePositives) / testResult.size();
    }

    /**
     * @brief Calcula F1-Score (média harmônica de precision e recall)
     *
     * F1 = 2 * (precision * recall) / (precision + recall)
     *
     * @param groundTruth Resultado esperado
     * @param testResult Resultado obtido
     * @return F1-Score no intervalo [0.0, 1.0]
     */
    static double calculateF1Score(
        const std::vector<Image>& groundTruth,
        const std::vector<Image>& testResult
    ) {
        double precision = calculatePrecision(groundTruth, testResult);
        double recall = calculateRecall(groundTruth, testResult);

        if (precision + recall == 0.0) return 0.0;

        return 2.0 * (precision * recall) / (precision + recall);
    }

    /**
     * @brief Valida completamente um resultado
     *
     * Calcula todas as métricas de validação.
     *
     * @param groundTruth Resultado esperado
     * @param testResult Resultado obtido
     * @param structureName Nome da estrutura (para logging)
     * @return ValidationResult com todas as métricas
     */
    static ValidationResult validate(
        const std::vector<Image>& groundTruth,
        const std::vector<Image>& testResult,
        const std::string& structureName = ""
    ) {
        ValidationResult result;

        std::set<int> truthSet = extractIdSet(groundTruth);
        std::set<int> testSet = extractIdSet(testResult);

        // Calcula métricas
        result.truePositives = countIntersection(truthSet, testSet);
        result.falsePositives = testSet.size() - result.truePositives;
        result.falseNegatives = truthSet.size() - result.truePositives;
        result.totalExpected = groundTruth.size();
        result.totalFound = testResult.size();

        // Calcula ratios
        if (result.totalExpected > 0) {
            result.recall = static_cast<double>(result.truePositives) / result.totalExpected;
        } else {
            result.recall = 1.0;
        }

        if (result.totalFound > 0) {
            result.precision = static_cast<double>(result.truePositives) / result.totalFound;
        } else {
            result.precision = 1.0;
        }

        result.f1Score = calculateF1Score(groundTruth, testResult);

        // Valida se está dentro dos critérios aceitáveis
        result.isValid = (result.recall >= 0.9 && result.precision >= 0.9);

        // Log se necessário
        if (Config::Output::VERBOSITY_LEVEL >= 2 && !structureName.empty()) {
            std::cout << "\n[VALIDATION] " << structureName << ":\n";
            result.print();
        }

        return result;
    }

    /**
     * @brief Verifica se os resultados estão ordenados por distância
     *
     * @param results Vetor de imagens
     * @param query Imagem de consulta
     * @return true se estão ordenados, false caso contrário
     */
    static bool areResultsSorted(
        const std::vector<Image>& results,
        const Image& query
    ) {
        for (size_t i = 1; i < results.size(); i++) {
            double dist1 = query.distanceTo(results[i-1]);
            double dist2 = query.distanceTo(results[i]);
            if (dist1 > dist2 + Config::Validation::DISTANCE_TOLERANCE) {
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Verifica se todos os resultados estão dentro do threshold
     *
     * @param results Vetor de imagens
     * @param query Imagem de consulta
     * @param threshold Threshold de distância
     * @return true se todos estão dentro do threshold, false caso contrário
     */
    static bool areResultsWithinThreshold(
        const std::vector<Image>& results,
        const Image& query,
        double threshold
    ) {
        for (const auto& img : results) {
            double dist = query.distanceTo(img);
            if (dist > threshold + Config::Validation::DISTANCE_TOLERANCE) {
                if (Config::Output::VERBOSITY_LEVEL >= 2) {
                    std::cout << "[WARNING] Image " << img.id
                              << " exceeds threshold: " << dist
                              << " > " << threshold << "\n";
                }
                return false;
            }
        }
        return true;
    }

    /**
     * @brief Gera relatório de comparação entre estruturas
     */
    static void printComparisonReport(
        const std::string& structure1Name,
        const std::vector<Image>& results1,
        const std::string& structure2Name,
        const std::vector<Image>& results2
    ) {
        std::cout << "\n========================================\n";
        std::cout << "COMPARISON REPORT\n";
        std::cout << "========================================\n";
        std::cout << structure1Name << " vs " << structure2Name << "\n\n";

        std::set<int> set1 = extractIdSet(results1);
        std::set<int> set2 = extractIdSet(results2);

        int common = countIntersection(set1, set2);
        int onlyIn1 = set1.size() - common;
        int onlyIn2 = set2.size() - common;

        std::cout << "Results in " << structure1Name << ": " << set1.size() << "\n";
        std::cout << "Results in " << structure2Name << ": " << set2.size() << "\n";
        std::cout << "Common results: " << common << "\n";
        std::cout << "Only in " << structure1Name << ": " << onlyIn1 << "\n";
        std::cout << "Only in " << structure2Name << ": " << onlyIn2 << "\n";

        double jaccard = (set1.size() + set2.size() > 0) ?
            static_cast<double>(common) / (set1.size() + set2.size() - common) : 1.0;
        std::cout << "Jaccard similarity: " << std::fixed << std::setprecision(4)
                  << (jaccard * 100) << "%\n";
        std::cout << "========================================\n\n";
    }

private:
    /**
     * @brief Extrai IDs de um vetor de imagens
     */
    static std::vector<int> extractIds(const std::vector<Image>& images) {
        std::vector<int> ids;
        ids.reserve(images.size());
        for (const auto& img : images) {
            ids.push_back(img.id);
        }
        return ids;
    }

    /**
     * @brief Extrai IDs para um set (remove duplicatas)
     */
    static std::set<int> extractIdSet(const std::vector<Image>& images) {
        std::set<int> ids;
        for (const auto& img : images) {
            ids.insert(img.id);
        }
        return ids;
    }

    /**
     * @brief Conta interseção entre dois sets
     */
    static int countIntersection(const std::set<int>& set1, const std::set<int>& set2) {
        int count = 0;
        for (int id : set1) {
            if (set2.count(id)) count++;
        }
        return count;
    }
};

#endif // VALIDATORS_H
