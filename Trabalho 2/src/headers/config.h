#ifndef CONFIG_H
#define CONFIG_H

#include <string>
#include <vector>

/**
 * @brief Configurações centralizadas do Trabalho 2
 *
 * Este arquivo centraliza todos os parâmetros configuráveis do projeto,
 * facilitando ajustes e experimentos sem modificar o código principal.
 */
namespace Config {

// =============================================================================
// PARÂMETROS DAS ESTRUTURAS DE DADOS
// =============================================================================

/**
 * @brief Configurações do LSH (Locality-Sensitive Hashing)
 */
namespace LSH {
    // Número de tabelas hash (mais tabelas = maior recall, maior custo)
    constexpr int DEFAULT_NUM_TABLES = 10;

    // Número de projeções por tabela (mais projeções = maior precisão)
    constexpr int DEFAULT_NUM_PROJECTIONS = 4;

    // Largura dos bins de quantização (menor = mais precisão, maior = mais velocidade)
    constexpr double DEFAULT_BIN_WIDTH = 50.0;

    // Seed para reprodutibilidade
    constexpr int DEFAULT_SEED = 42;

    // Número de probes para multiprobe search
    constexpr int DEFAULT_NUM_PROBES = 3;
}

/**
 * @brief Configurações do M-Tree (Metric Tree)
 */
namespace MTree {
    // Capacidade máxima de cada nó antes de fazer split
    constexpr int DEFAULT_CAPACITY = 10;

    // Política de split: "balanced", "min_radius", "min_overlap"
    const std::string SPLIT_POLICY = "balanced";
}

/**
 * @brief Configurações do Hash Search
 */
namespace Hash {
    // Tamanho da célula para spatial hashing
    constexpr double DEFAULT_CELL_SIZE = 30.0;

    // Fator de expansão para Hash Dynamic
    constexpr int DEFAULT_EXPANSION_RADIUS = 1;
}

/**
 * @brief Configurações das árvores espaciais (Octree, Quadtree)
 */
namespace Trees {
    // Máximo de imagens por nó antes de subdividir
    constexpr int OCTREE_MAX_PER_NODE = 10;
    constexpr int QUADTREE_MAX_PER_NODE = 25;

    // Profundidade máxima da árvore (evita recursão infinita)
    constexpr int MAX_DEPTH = 20;
}

// =============================================================================
// PARÂMETROS DE BENCHMARK
// =============================================================================

/**
 * @brief Configurações de teste e benchmark
 */
namespace Benchmark {
    // Threshold padrão para range query (distância Euclidiana em RGB)
    constexpr double DEFAULT_THRESHOLD = 40.0;

    // Número de vizinhos para k-NN query
    constexpr int DEFAULT_K_NEIGHBORS = 5;

    // Escalas de teste (número de imagens)
    const std::vector<int> TEST_SCALES = {1000, 5000, 10000, 25000};

    // Executar teste completo ou rápido
    constexpr bool QUICK_TEST = false;  // true = só 1K imagens

    // Número de repetições para medir tempo médio
    constexpr int NUM_ITERATIONS = 1;

    // Mostrar progresso durante carregamento
    constexpr bool SHOW_PROGRESS = true;

    // Salvar resultados em arquivo
    constexpr bool SAVE_RESULTS = true;
}

// =============================================================================
// CAMINHOS DE ARQUIVOS E DIRETÓRIOS
// =============================================================================

/**
 * @brief Paths para datasets e outputs
 */
namespace Paths {
    // Dataset de imagens
    const std::string IMAGE_DATASET = "./images/";

    // Imagem de query
    const std::string QUERY_IMAGE = "./query/query.jpg";

    // Diretório de resultados
    const std::string RESULTS_DIR = "./resultados/";

    // Arquivo de saída dos resultados
    const std::string RESULTS_FILE = "./resultados/resultados.txt";

    // Arquivo de resultados detalhados (CSV)
    const std::string RESULTS_CSV = "./resultados/resultados.csv";
}

// =============================================================================
// PARÂMETROS DE EXTRAÇÃO DE FEATURES
// =============================================================================

/**
 * @brief Configurações de extração RGB
 */
namespace Features {
    // Método de extração: "mean", "median", "dominant", "center_pixel"
    const std::string EXTRACTION_METHOD = "mean";

    // Redimensionar imagem antes de processar (0 = não redimensionar)
    constexpr int RESIZE_WIDTH = 0;
    constexpr int RESIZE_HEIGHT = 0;

    // Normalizar RGB para [0,1]?
    constexpr bool NORMALIZE_RGB = false;
}

// =============================================================================
// PARÂMETROS DE OUTPUT E LOGGING
// =============================================================================

/**
 * @brief Configurações de output
 */
namespace Output {
    // Verbosidade: 0=silencioso, 1=normal, 2=detalhado, 3=debug
    constexpr int VERBOSITY_LEVEL = 1;

    // Mostrar estatísticas das estruturas após construção
    constexpr bool SHOW_STRUCTURE_STATS = false;

    // Mostrar IDs das imagens encontradas
    constexpr bool SHOW_FOUND_IDS = false;

    // Formato da tabela: "simple", "markdown", "latex"
    const std::string TABLE_FORMAT = "simple";

    // Precisão dos números (casas decimais)
    constexpr int FLOAT_PRECISION = 2;
}

// =============================================================================
// PARÂMETROS DE VALIDAÇÃO
// =============================================================================

/**
 * @brief Configurações de validação e testes
 */
namespace Validation {
    // Executar validação de resultados (compara com ground truth)
    constexpr bool ENABLE_VALIDATION = true;

    // Tolerância para comparação de distâncias
    constexpr double DISTANCE_TOLERANCE = 1e-6;

    // Calcular recall automaticamente
    constexpr bool CALCULATE_RECALL = true;

    // Estrutura baseline para comparação (ground truth)
    const std::string GROUND_TRUTH_STRUCTURE = "Linear Search";
}

// =============================================================================
// OTIMIZAÇÕES E PERFORMANCE
// =============================================================================

/**
 * @brief Flags de otimização
 */
namespace Optimizations {
    // Usar multithreading onde possível
    constexpr bool ENABLE_MULTITHREADING = false;

    // Número de threads (0 = auto-detect)
    constexpr int NUM_THREADS = 0;

    // Pré-alocar memória para vetores
    constexpr bool PREALLOCATE_VECTORS = true;

    // Usar cache para distâncias calculadas
    constexpr bool CACHE_DISTANCES = false;
}

// =============================================================================
// HELPERS
// =============================================================================

/**
 * @brief Funções helper para acessar configurações
 */
inline std::vector<int> getTestScales() {
    if (Benchmark::QUICK_TEST) {
        return {1000};  // Apenas 1K para teste rápido
    }
    return Benchmark::TEST_SCALES;
}

inline std::string getTimestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    char buffer[100];
    strftime(buffer, sizeof(buffer), "%Y%m%d_%H%M%S", localtime(&time));
    return std::string(buffer);
}

} // namespace Config

#endif // CONFIG_H
