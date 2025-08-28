/*
=============================================================================
            PAA Assignment 1: Análise de 4 Estruturas de Dados
=============================================================================

Este arquivo demonstra a implementação e análise comparativa de 4 estruturas
de dados para busca por similaridade em espaços multidimensionais (RGB):

1. BUSCA LINEAR (Força Bruta)
2. HASH TABLE (Spatial Hashing)  
3. OCTREE (Árvore Espacial 3D)
4. QUADTREE (Árvore Espacial 2D)

Conceitos de PAA demonstrados:
- Análise de Complexidade (Big O)
- Trade-offs entre Tempo e Espaço
- Estruturas de Dados Espaciais
- Algoritmos de Busca por Proximidade
- Técnicas de Indexação Multidimensional
- Otimizações Algorítmicas

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
#include <stack>
#include <queue>

// ============================================================================
// REPRESENTAÇÃO DE DADOS - ESPAÇO RGB COMO PROBLEMA MULTIDIMENSIONAL
// ============================================================================

struct Image {
    int id;
    std::string filename;
    double r, g, b;  // Coordenadas no espaço RGB (0-255)
    
    Image(int _id, const std::string& _filename, double _r, double _g, double _b) 
        : id(_id), filename(_filename), r(_r), g(_g), b(_b) {}
    
    // MÉTRICA DE SIMILARIDADE: Distância Euclidiana no Espaço 3D
    /*
    ANÁLISE PAA:
    - Função de distância define a métrica de similaridade
    - Espaço RGB = R³ (3 dimensões)
    - Distância euclidiana é métrica padrão para espaços contínuos
    - Complexidade: O(1) para calcular distância entre 2 pontos
    */
    double distanceTo(const Image& other) const {
        double dr = r - other.r;
        double dg = g - other.g;
        double db = b - other.b;
        return sqrt(dr*dr + dg*dg + db*db);
    }
    
    void print() const {
        std::cout << "Image " << id << " (" << filename << "): "
                  << "RGB(" << r << ", " << g << ", " << b << ")" << std::endl;
    }
};

// ============================================================================
// INTERFACE ABSTRATA - PADRÃO DE DESIGN PARA COMPARAÇÃO JUSTA
// ============================================================================

class ImageDatabase {
public:
    virtual ~ImageDatabase() = default;
    
    // Operações fundamentais para análise de complexidade
    virtual void insert(const Image& img) = 0;
    virtual std::vector<Image> findSimilar(const Image& query, double threshold) = 0;
    virtual std::string getName() const = 0;
};

// ============================================================================
// ESTRUTURA 1: BUSCA LINEAR (BASELINE)
// ============================================================================
/*
ANÁLISE PAA - BUSCA LINEAR:

CARACTERÍSTICAS:
- Estrutura mais simples possível
- Não há pré-processamento dos dados
- Serve como baseline para comparação

COMPLEXIDADES:
- Inserção: O(1) - apenas adiciona ao final
- Busca: O(n) - deve verificar todos os elementos
- Espaço: O(n) - armazena apenas os dados

TRADE-OFFS:
- Vantagem: Implementação trivial, sem overhead
- Desvantagem: Busca lenta para grandes datasets

QUANDO USAR:
- Datasets pequenos (n < 1000)
- Quando implementação simples é prioridade
- Como baseline para avaliar outras estruturas
*/

class LinearSearch : public ImageDatabase {
private:
    std::vector<Image> images;  // Array dinâmico simples
    
public:
    void insert(const Image& img) override {
        // O(1) - inserção no final do array
        images.push_back(img);
    }
    
    std::vector<Image> findSimilar(const Image& query, double threshold) override {
        std::vector<Image> results;
        
        // O(n) - FORÇA BRUTA: examina todos os elementos
        for (const auto& img : images) {
            double distance = query.distanceTo(img);  // O(1)
            if (distance <= threshold) {
                results.push_back(img);
            }
        }
        
        // O(k log k) onde k = número de resultados
        std::sort(results.begin(), results.end(), 
                 [&query](const Image& a, const Image& b) {
                     return query.distanceTo(a) < query.distanceTo(b);
                 });
        
        return results;
    }
    
    std::string getName() const override {
        return "Linear Search (Força Bruta)";
    }
    
    size_t size() const { return images.size(); }
};

// ============================================================================
// ESTRUTURA 2: HASH TABLE com SPATIAL HASHING
// ============================================================================
/*
ANÁLISE PAA - SPATIAL HASHING:

CONCEITO:
- Divide o espaço RGB em células (grid 3D)
- Cada célula é uma "bucket" na hash table
- Imagens similares ficam em células próximas

TÉCNICA DE INDEXAÇÃO:
- Hash function: (r/cellSize, g/cellSize, b/cellSize)
- Collision resolution: chaining (lista em cada célula)
- Spatial locality: células vizinhas contêm pontos próximos

COMPLEXIDADES:
- Inserção: O(1) esperado - hash + insert na lista
- Busca: O(k) onde k = células examinadas × densidade
- Espaço: O(n + m) onde m = número de células ativas

OTIMIZAÇÃO:
- Cell size determina trade-off precisão vs performance
- Muito pequeno: muitas células, overhead alto  
- Muito grande: muitas comparações desnecessárias

QUANDO USAR:
- Datasets médios/grandes (n > 1000)
- Distribuição uniforme dos dados
- Quando busca rápida é prioridade
*/

class HashSearch : public ImageDatabase {
private:
    double cellSize;  // Parâmetro de tunning do algoritmo
    
    // Hash Table: chave = coordenada da célula, valor = lista de imagens
    std::unordered_map<std::string, std::vector<Image>> grid;
    
    // FUNÇÃO HASH: Mapeia coordenada RGB para coordenada de célula
    int rgbToCell(double value) const {
        return static_cast<int>(value / cellSize);
    }
    
    // CHAVE DA HASH TABLE: Serializa coordenadas 3D em string
    std::string getCellKey(const Image& img) const {
        int r_cell = rgbToCell(img.r);
        int g_cell = rgbToCell(img.g);
        int b_cell = rgbToCell(img.b);
        return std::to_string(r_cell) + "," + 
               std::to_string(g_cell) + "," + 
               std::to_string(b_cell);
    }
    
    std::string getCellKey(int r_cell, int g_cell, int b_cell) const {
        return std::to_string(r_cell) + "," + 
               std::to_string(g_cell) + "," + 
               std::to_string(b_cell);
    }
    
public:
    HashSearch(double _cellSize = 30.0) : cellSize(_cellSize) {}
    
    void insert(const Image& img) override {
        // O(1) esperado - hash + insert
        std::string key = getCellKey(img);
        grid[key].push_back(img);
    }
    
    std::vector<Image> findSimilar(const Image& query, double threshold) override {
        std::vector<Image> results;
        
        // OTIMIZAÇÃO ESPACIAL: calcular raio de busca em células
        int query_r = rgbToCell(query.r);
        int query_g = rgbToCell(query.g);
        int query_b = rgbToCell(query.b);
        
        // Quantas células precisamos examinar baseado no threshold?
        int cell_radius = static_cast<int>(ceil(threshold / cellSize));
        
        // BUSCA EM CUBO 3D: examina apenas células relevantes
        for (int dr = -cell_radius; dr <= cell_radius; dr++) {
            for (int dg = -cell_radius; dg <= cell_radius; dg++) {
                for (int db = -cell_radius; db <= cell_radius; db++) {
                    std::string key = getCellKey(query_r + dr, 
                                               query_g + dg, 
                                               query_b + db);
                    
                    auto it = grid.find(key);
                    if (it != grid.end()) {
                        // Examinar todas as imagens nesta célula
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
        
        // Ordenar por distância
        std::sort(results.begin(), results.end(), 
                 [&query](const Image& a, const Image& b) {
                     return query.distanceTo(a) < query.distanceTo(b);
                 });
        
        return results;
    }
    
    std::string getName() const override {
        return "Hash Search (Spatial Grid, cell=" + std::to_string(cellSize) + ")";
    }
    
    // MÉTRICA DE ANÁLISE: distribuição de dados
    size_t getNumCells() const { return grid.size(); }
    
    double getAverageCellSize() const {
        if (grid.empty()) return 0;
        
        size_t totalImages = 0;
        for (const auto& pair : grid) {
            totalImages += pair.second.size();
        }
        return static_cast<double>(totalImages) / grid.size();
    }
    
    void printAnalysis() const {
        std::cout << "  ANÁLISE SPATIAL HASHING:" << std::endl;
        std::cout << "    Células ativas: " << getNumCells() << std::endl;
        std::cout << "    Densidade média: " << getAverageCellSize() << " imagens/célula" << std::endl;
        std::cout << "    Tamanho da célula: " << cellSize << std::endl;
    }
};

// ============================================================================
// ESTRUTURA 3: OCTREE (ÁRVORE ESPACIAL 3D)
// ============================================================================
/*
ANÁLISE PAA - OCTREE:

CONCEITO:
- Árvore de subdivisão espacial para espaço 3D
- Cada nó representa uma região cúbica do espaço RGB
- Divisão adaptativa baseada na densidade de dados

PROPRIEDADES ESTRUTURAIS:
- Cada nó interno tem exatamente 8 filhos (octantes)
- Nós folha contêm as imagens da região
- Profundidade varia conforme distribuição dos dados

ALGORITMO DE CONSTRUÇÃO:
1. Inserir ponto no nó raiz
2. Se nó folha e não cheio: adicionar ponto
3. Se nó folha e cheio: dividir em 8 octantes
4. Redistribuir pontos pelos octantes apropriados
5. Recursivamente inserir novo ponto

COMPLEXIDADES:
- Inserção: O(log n) esperado, O(h) onde h = altura
- Busca: O(log n + k) onde k = resultados
- Espaço: O(n + nós internos)

TÉCNICA DE PODA (PRUNING):
- Calcula distância mínima do query à região do nó
- Se > threshold, poda toda a subárvore
- Evita examinar regiões distantes

QUANDO USAR:
- Datasets grandes (n > 10000)
- Distribuição não-uniforme dos dados
- Busca em alta dimensionalidade (até ~10D)
*/

struct OctreeNode {
    // BOUNDING BOX: região 3D que este nó representa
    double minR, maxR, minG, maxG, minB, maxB;
    
    std::vector<Image> images;  // Imagens nesta região (se folha)
    std::array<std::unique_ptr<OctreeNode>, 8> children;  // 8 octantes
    bool isLeaf;
    
    OctreeNode(double minR, double maxR, double minG, double maxG, 
               double minB, double maxB)
        : minR(minR), maxR(maxR), minG(minG), maxG(maxG), 
          minB(minB), maxB(maxB), isLeaf(true) {
        for (auto& child : children) {
            child = nullptr;
        }
    }
    
    // TESTE DE CONTENÇÃO: ponto está nesta região?
    bool contains(const Image& img) const {
        return img.r >= minR && img.r <= maxR &&
               img.g >= minG && img.g <= maxG &&
               img.b >= minB && img.b <= maxB;
    }
    
    // FUNÇÃO DE INDEXAÇÃO: qual octante contém este ponto?
    /*
    TÉCNICA PAA: Mapeamento bit a bit
    - Bit 2: R >= midR ? 1 : 0
    - Bit 1: G >= midG ? 1 : 0  
    - Bit 0: B >= midB ? 1 : 0
    - Resulta em índice 0-7
    */
    int getChildIndex(const Image& img) const {
        int index = 0;
        double midR = (minR + maxR) / 2.0;
        double midG = (minG + maxG) / 2.0;  
        double midB = (minB + maxB) / 2.0;
        
        if (img.r >= midR) index |= 4;  // Bit 2
        if (img.g >= midG) index |= 2;  // Bit 1
        if (img.b >= midB) index |= 1;  // Bit 0
        
        return index;
    }
    
    // SUBDIVISÃO ESPACIAL: criar 8 octantes filhos
    void createChildren() {
        double midR = (minR + maxR) / 2.0;
        double midG = (minG + maxG) / 2.0;
        double midB = (minB + maxB) / 2.0;
        
        // 8 octantes do cubo 3D
        children[0] = std::make_unique<OctreeNode>(minR, midR, minG, midG, minB, midB);
        children[1] = std::make_unique<OctreeNode>(minR, midR, minG, midG, midB, maxB);
        children[2] = std::make_unique<OctreeNode>(minR, midR, midG, maxG, minB, midB);
        children[3] = std::make_unique<OctreeNode>(minR, midR, midG, maxG, midB, maxB);
        children[4] = std::make_unique<OctreeNode>(midR, maxR, minG, midG, minB, midB);
        children[5] = std::make_unique<OctreeNode>(midR, maxR, minG, midG, minB, maxB);
        children[6] = std::make_unique<OctreeNode>(midR, maxR, midG, maxG, minB, midB);
        children[7] = std::make_unique<OctreeNode>(midR, maxR, midG, maxG, minB, maxB);
        
        isLeaf = false;
    }
};

class OctreeSearch : public ImageDatabase {
private:
    std::unique_ptr<OctreeNode> root;
    int maxImagesPerNode;  // Parâmetro de balanceamento
    int totalImages;
    int maxDepth;
    
    // INSERÇÃO RECURSIVA com divisão adaptativa
    void insertRecursive(OctreeNode* node, const Image& img, int depth = 0) {
        maxDepth = std::max(maxDepth, depth);
        
        if (node->isLeaf) {
            node->images.push_back(img);
            
            // CRITÉRIO DE DIVISÃO: muito cheio e não muito profundo
            if (static_cast<int>(node->images.size()) > maxImagesPerNode && depth < 15) {
                node->createChildren();
                
                // REDISTRIBUIÇÃO: realocar todas as imagens
                for (const auto& existingImg : node->images) {
                    int childIdx = node->getChildIndex(existingImg);
                    insertRecursive(node->children[childIdx].get(), existingImg, depth + 1);
                }
                
                node->images.clear();  // Nó não é mais folha
            }
        } else {
            // Navegar para o octante apropriado
            int childIdx = node->getChildIndex(img);
            insertRecursive(node->children[childIdx].get(), img, depth + 1);
        }
    }
    
    // BUSCA RECURSIVA com PODA ESPACIAL
    void searchRecursive(OctreeNode* node, const Image& query, double threshold, 
                        std::vector<Image>& results) const {
        if (!node) return;
        
        // TÉCNICA DE PODA: região pode conter pontos próximos?
        if (!nodeIntersectsQueryRadius(node, query, threshold)) {
            return;  // Poda toda a subárvore
        }
        
        if (node->isLeaf) {
            // Examinar todas as imagens nesta folha
            for (const auto& img : node->images) {
                double distance = query.distanceTo(img);
                if (distance <= threshold) {
                    results.push_back(img);
                }
            }
        } else {
            // Recursivamente buscar nos filhos
            for (const auto& child : node->children) {
                if (child) {
                    searchRecursive(child.get(), query, threshold, results);
                }
            }
        }
    }
    
    // GEOMETRIC PRUNING: distância mínima do query ao bounding box
    /*
    TÉCNICA PAA: Distância ponto-retângulo em 3D
    - Se query está dentro do box: distância = 0
    - Caso contrário: distância = sqrt(soma dos quadrados das diferenças)
    */
    bool nodeIntersectsQueryRadius(OctreeNode* node, const Image& query, double threshold) const {
        double minDistSq = 0.0;
        
        // Componente R
        if (query.r < node->minR) {
            double diff = node->minR - query.r;
            minDistSq += diff * diff;
        } else if (query.r > node->maxR) {
            double diff = query.r - node->maxR;  
            minDistSq += diff * diff;
        }
        
        // Componente G
        if (query.g < node->minG) {
            double diff = node->minG - query.g;
            minDistSq += diff * diff;
        } else if (query.g > node->maxG) {
            double diff = query.g - node->maxG;
            minDistSq += diff * diff;
        }
        
        // Componente B
        if (query.b < node->minB) {
            double diff = node->minB - query.b;
            minDistSq += diff * diff;
        } else if (query.b > node->maxB) {
            double diff = query.b - node->maxB;
            minDistSq += diff * diff;
        }
        
        return sqrt(minDistSq) <= threshold;
    }
    
    // ANÁLISE ESTRUTURAL: contar nós da árvore
    void countNodes(OctreeNode* node, int& leafCount, int& internalCount) const {
        if (!node) return;
        
        if (node->isLeaf) {
            leafCount++;
        } else {
            internalCount++;
            for (const auto& child : node->children) {
                countNodes(child.get(), leafCount, internalCount);
            }
        }
    }
    
public:
    OctreeSearch(int maxImages = 10) 
        : maxImagesPerNode(maxImages), totalImages(0), maxDepth(0) {
        // Inicializar com espaço RGB completo [0,255]³
        root = std::make_unique<OctreeNode>(0, 255, 0, 255, 0, 255);
    }
    
    void insert(const Image& img) override {
        insertRecursive(root.get(), img);
        totalImages++;
    }
    
    std::vector<Image> findSimilar(const Image& query, double threshold) override {
        std::vector<Image> results;
        searchRecursive(root.get(), query, threshold, results);
        
        std::sort(results.begin(), results.end(), 
                 [&query](const Image& a, const Image& b) {
                     return query.distanceTo(a) < query.distanceTo(b);
                 });
        
        return results;
    }
    
    std::string getName() const override {
        return "Octree 3D (maxPerNode=" + std::to_string(maxImagesPerNode) + ")";
    }
    
    void printAnalysis() const {
        int leafCount = 0, internalCount = 0;
        countNodes(root.get(), leafCount, internalCount);
        
        std::cout << "  ANÁLISE OCTREE 3D:" << std::endl;
        std::cout << "    Total de imagens: " << totalImages << std::endl;
        std::cout << "    Profundidade máxima: " << maxDepth << std::endl;
        std::cout << "    Nós folha: " << leafCount << std::endl;
        std::cout << "    Nós internos: " << internalCount << std::endl;
        std::cout << "    Fator de ramificação médio: " 
                  << (internalCount > 0 ? static_cast<double>(leafCount) / internalCount : 0) << std::endl;
        
        if (leafCount > 0) {
            std::cout << "    Densidade média por folha: " 
                      << static_cast<double>(totalImages) / leafCount << " imagens" << std::endl;
        }
    }
};

// ============================================================================
// ESTRUTURA 4: QUADTREE (ÁRVORE ESPACIAL 2D)  
// ============================================================================
/*
ANÁLISE PAA - QUADTREE:

CONCEITO:
- Árvore de subdivisão para espaço 2D (usando apenas R,G)
- Cada nó tem exatamente 4 filhos (quadrantes)
- Busca ainda considera distância 3D completa (R,G,B)

MOTIVAÇÃO:
- Curse of dimensionality: estruturas espaciais degradam em alta dimensão
- Redução dimensional: projeta RGB(3D) → RG(2D)
- Mantém eficácia para consultas de proximidade

TÉCNICA DE PROJEÇÃO:
- Estruturação: usa apenas coordenadas (R,G)
- Busca: calcula distância euclidiana completa em (R,G,B)
- Trade-off: menor precisão de poda vs menor overhead

COMPLEXIDADES:
- Inserção: O(log n) esperado no espaço 2D
- Busca: O(log n + k) com poda menos eficiente que Octree
- Espaço: O(n + nós internos), menor overhead que Octree

IMPLEMENTAÇÃO ITERATIVA:
- Evita recursão (stack overflow em datasets grandes)
- Usa stack/queue explícitas
- Melhor controle de memória

QUANDO USAR:
- Datasets muito grandes (n > 100000)
- Quando Octree é muito lento
- Distribuição concentrada em 2 dimensões principais
*/

struct QuadtreeNode {
    // BOUNDING RECTANGLE: região 2D que este nó representa (apenas R,G)
    double minR, maxR, minG, maxG;
    
    std::vector<Image> images;  // Imagens nesta região (se folha)
    std::array<std::unique_ptr<QuadtreeNode>, 4> children;  // 4 quadrantes
    bool isLeaf;
    
    QuadtreeNode(double minR, double maxR, double minG, double maxG)
        : minR(minR), maxR(maxR), minG(minG), maxG(maxG), isLeaf(true) {
        for (auto& child : children) {
            child = nullptr;
        }
    }
    
    // TESTE DE CONTENÇÃO no espaço 2D (R,G)
    bool contains(const Image& img) const {
        return img.r >= minR && img.r <= maxR &&
               img.g >= minG && img.g <= maxG;
    }
    
    // FUNÇÃO DE INDEXAÇÃO 2D: qual quadrante contém este ponto?
    /*
    TÉCNICA PAA: Mapeamento binário 2D
    - Bit 1: R >= midR ? 1 : 0  (direita/esquerda)
    - Bit 0: G >= midG ? 1 : 0  (cima/baixo)
    - Resulta em índice 0-3
    */
    int getChildIndex(const Image& img) const {
        int index = 0;
        double midR = (minR + maxR) / 2.0;
        double midG = (minG + maxG) / 2.0;
        
        if (img.r >= midR) index |= 2;  // Bit 1: R >= midR
        if (img.g >= midG) index |= 1;  // Bit 0: G >= midG
        
        return index;
    }
    
    // SUBDIVISÃO 2D: criar 4 quadrantes filhos
    void createChildren() {
        double midR = (minR + maxR) / 2.0;
        double midG = (minG + maxG) / 2.0;
        
        // 4 quadrantes do retângulo 2D
        children[0] = std::make_unique<QuadtreeNode>(minR, midR, minG, midG);  // bottom-left
        children[1] = std::make_unique<QuadtreeNode>(minR, midR, midG, maxG);  // top-left
        children[2] = std::make_unique<QuadtreeNode>(midR, maxR, minG, midG);  // bottom-right
        children[3] = std::make_unique<QuadtreeNode>(midR, maxR, midG, maxG);  // top-right
        
        isLeaf = false;
    }
};

class QuadtreeIterativeSearch : public ImageDatabase {
private:
    std::unique_ptr<QuadtreeNode> root;
    int maxImagesPerNode;
    int totalImages;
    int maxDepth;
    
    // INSERÇÃO ITERATIVA usando Stack Explícita
    /*
    TÉCNICA PAA: Simulação de recursão com stack
    - Evita stack overflow em datasets grandes
    - Melhor controle de memória
    - Facilita debugging e profiling
    */
    void insertIterative(const Image& img) {
        std::stack<std::pair<QuadtreeNode*, int>> nodeStack;
        nodeStack.push({root.get(), 0});
        
        while (!nodeStack.empty()) {
            auto [node, depth] = nodeStack.top();
            nodeStack.pop();
            
            maxDepth = std::max(maxDepth, depth);
            
            if (node->isLeaf) {
                node->images.push_back(img);
                
                // CRITÉRIO DE DIVISÃO adaptativo
                if (static_cast<int>(node->images.size()) > maxImagesPerNode && depth < 12) {
                    node->createChildren();
                    
                    // REDISTRIBUIÇÃO das imagens existentes
                    std::vector<Image> imagesToRedistribute = node->images;
                    node->images.clear();
                    
                    for (const auto& existingImg : imagesToRedistribute) {
                        int childIdx = node->getChildIndex(existingImg);
                        // Adicionar na stack para processamento posterior
                        nodeStack.push({node->children[childIdx].get(), depth + 1});
                        // Inserir diretamente no filho
                        node->children[childIdx]->images.push_back(existingImg);
                    }
                }
            } else {
                int childIdx = node->getChildIndex(img);
                nodeStack.push({node->children[childIdx].get(), depth + 1});
            }
        }
    }
    
    // GEOMETRIC PRUNING 2D com distância 3D
    /*
    TÉCNICA HÍBRIDA PAA:
    - Poda baseada em projeção 2D (R,G) - rápida mas menos precisa  
    - Distância final calculada em 3D (R,G,B) - precisa mas mais cara
    */
    bool nodeIntersectsQueryRadius(QuadtreeNode* node, const Image& query, double threshold) const {
        double minDistSq = 0.0;
        
        // Componente R (dimensão estruturada)
        if (query.r < node->minR) {
            double diff = node->minR - query.r;
            minDistSq += diff * diff;
        } else if (query.r > node->maxR) {
            double diff = query.r - node->maxR;
            minDistSq += diff * diff;
        }
        
        // Componente G (dimensão estruturada)
        if (query.g < node->minG) {
            double diff = node->minG - query.g;
            minDistSq += diff * diff;
        } else if (query.g > node->maxG) {
            double diff = query.g - node->maxG;
            minDistSq += diff * diff;
        }
        
        // Nota: componente B não é considerada na poda (menos eficiente)
        // mas será considerada na distância final
        return sqrt(minDistSq) <= threshold;
    }
    
    // BUSCA ITERATIVA usando Queue (BFS)
    /*
    TÉCNICA PAA: Breadth-First Search
    - Examina níveis da árvore em ordem
    - Melhor localidade de memória
    - Facilita balanceamento de carga
    */
    void searchIterative(const Image& query, double threshold, std::vector<Image>& results) {
        std::queue<QuadtreeNode*> queue;
        queue.push(root.get());
        
        while (!queue.empty()) {
            QuadtreeNode* node = queue.front();
            queue.pop();
            
            if (!node) continue;
            
            // PODA GEOMÉTRICA: vale a pena examinar este nó?
            if (!nodeIntersectsQueryRadius(node, query, threshold)) {
                continue;  // Poda subárvore
            }
            
            if (node->isLeaf) {
                // Examinar todos os pontos nesta folha
                for (const auto& img : node->images) {
                    double distance = query.distanceTo(img);  // DISTÂNCIA 3D COMPLETA
                    if (distance <= threshold) {
                        results.push_back(img);
                    }
                }
            } else {
                // Adicionar filhos na queue para processamento
                for (const auto& child : node->children) {
                    if (child) {
                        queue.push(child.get());
                    }
                }
            }
        }
    }
    
    // ANÁLISE ESTRUTURAL iterativa
    void countNodes(QuadtreeNode* node, int& leafCount, int& internalCount) const {
        std::queue<QuadtreeNode*> queue;
        queue.push(node);
        
        while (!queue.empty()) {
            QuadtreeNode* current = queue.front();
            queue.pop();
            
            if (current->isLeaf) {
                leafCount++;
            } else {
                internalCount++;
                for (const auto& child : current->children) {
                    if (child) {
                        queue.push(child.get());
                    }
                }
            }
        }
    }
    
public:
    QuadtreeIterativeSearch(int maxImages = 25) 
        : maxImagesPerNode(maxImages), totalImages(0), maxDepth(0) {
        // Inicializar com espaço RG completo [0,255]²
        root = std::make_unique<QuadtreeNode>(0, 255, 0, 255);
    }
    
    void insert(const Image& img) override {
        insertIterative(img);
        totalImages++;
    }
    
    std::vector<Image> findSimilar(const Image& query, double threshold) override {
        std::vector<Image> results;
        searchIterative(query, threshold, results);
        
        std::sort(results.begin(), results.end(), 
                 [&query](const Image& a, const Image& b) {
                     return query.distanceTo(a) < query.distanceTo(b);
                 });
        
        return results;
    }
    
    std::string getName() const override {
        return "Quadtree 2D Iterativo (maxPerNode=" + std::to_string(maxImagesPerNode) + ")";
    }
    
    void printAnalysis() const {
        int leafCount = 0, internalCount = 0;
        countNodes(root.get(), leafCount, internalCount);
        
        std::cout << "  ANÁLISE QUADTREE 2D:" << std::endl;
        std::cout << "    Total de imagens: " << totalImages << std::endl;
        std::cout << "    Profundidade máxima: " << maxDepth << std::endl;
        std::cout << "    Nós folha: " << leafCount << std::endl;
        std::cout << "    Nós internos: " << internalCount << std::endl;
        std::cout << "    Razão folha/interno: " 
                  << (internalCount > 0 ? static_cast<double>(leafCount) / internalCount : 0) << std::endl;
        
        if (leafCount > 0) {
            std::cout << "    Densidade média por folha: " 
                      << static_cast<double>(totalImages) / leafCount << " imagens" << std::endl;
        }
        
        std::cout << "    Observação: Estruturação 2D (R,G), busca 3D (R,G,B)" << std::endl;
    }
};

// ============================================================================
// GERAÇÃO DE DADOS SINTÉTICOS PARA BENCHMARKING
// ============================================================================
/*
METODOLOGIA PAA: Synthetic Workload Generation

CARACTERÍSTICAS:
- Distribuição uniforme no espaço RGB
- Ids sequenciais para rastreabilidade  
- Filenames simulados para realismo
- Controle sobre tamanho do dataset
*/

std::vector<Image> generateSyntheticDataset(int count) {
    std::vector<Image> images;
    images.reserve(count);  // Otimização: pré-alocar memória
    
    // Generator de alta qualidade
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> colorDist(0.0, 255.0);
    
    for (int i = 0; i < count; ++i) {
        double r = colorDist(gen);
        double g = colorDist(gen);
        double b = colorDist(gen);
        
        images.emplace_back(i + 1, 
                           "synthetic_" + std::to_string(i + 1) + ".jpg", 
                           r, g, b);
    }
    
    return images;
}

// ============================================================================
// FRAMEWORK DE BENCHMARKING PARA ANÁLISE EXPERIMENTAL
// ============================================================================
/*
METODOLOGIA PAA: Experimental Analysis

MÉTRICAS COLETADAS:
1. Tempo de inserção (construção da estrutura)
2. Tempo de busca (consulta por similaridade)  
3. Número de resultados encontrados
4. Qualidade dos resultados (ordenação por distância)

CONFIGURAÇÃO EXPERIMENTAL:
- Dataset sintético controlado
- Query point fixo (gray médio)
- Threshold fixo para comparação justa
- Medição de alta precisão com chrono
*/

void experimentalAnalysis(ImageDatabase& db, const std::vector<Image>& dataset, 
                         const Image& query, double threshold) {
    
    std::cout << "\n" << std::string(60, '=') << std::endl;
    std::cout << "ANÁLISE EXPERIMENTAL: " << db.getName() << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    // FASE 1: CONSTRUÇÃO DA ESTRUTURA (Inserção)
    std::cout << "FASE 1: Construção da Estrutura de Dados" << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    for (const auto& img : dataset) {
        db.insert(img);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto insertTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    double insertMs = insertTime.count() / 1000.0;
    std::cout << "  Tempo total de inserção: " << insertMs << " ms" << std::endl;
    std::cout << "  Throughput de inserção: " 
              << (dataset.size() / insertMs * 1000.0) << " imagens/segundo" << std::endl;
    std::cout << "  Tempo médio por inserção: " 
              << (insertMs / dataset.size()) << " ms/imagem" << std::endl;
    
    // FASE 2: CONSULTA DE SIMILARIDADE (Busca)
    std::cout << "\nFASE 2: Consulta de Similaridade" << std::endl;
    std::cout << "  Query point: RGB(" << query.r << ", " << query.g << ", " << query.b << ")" << std::endl;
    std::cout << "  Threshold: " << threshold << std::endl;
    
    start = std::chrono::high_resolution_clock::now();
    auto results = db.findSimilar(query, threshold);
    end = std::chrono::high_resolution_clock::now();
    auto searchTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    double searchMs = searchTime.count() / 1000.0;
    std::cout << "  Tempo de busca: " << searchMs << " ms" << std::endl;
    std::cout << "  Resultados encontrados: " << results.size() << std::endl;
    
    if (!results.empty()) {
        std::cout << "  Taxa de seletividade: " 
                  << (static_cast<double>(results.size()) / dataset.size() * 100.0) << "%" << std::endl;
    }
    
    // FASE 3: ANÁLISE DE QUALIDADE DOS RESULTADOS
    std::cout << "\nFASE 3: Qualidade dos Resultados" << std::endl;
    
    if (!results.empty()) {
        // Verificar ordenação (deve estar em ordem crescente de distância)
        bool isSorted = true;
        for (size_t i = 1; i < results.size(); i++) {
            if (query.distanceTo(results[i-1]) > query.distanceTo(results[i])) {
                isSorted = false;
                break;
            }
        }
        
        std::cout << "  Resultados ordenados: " << (isSorted ? "✓ Sim" : "✗ Não") << std::endl;
        std::cout << "  Distância mínima: " << query.distanceTo(results.front()) << std::endl;
        std::cout << "  Distância máxima: " << query.distanceTo(results.back()) << std::endl;
        
        // Mostrar amostra dos resultados
        std::cout << "  Amostra dos primeiros 3 resultados:" << std::endl;
        for (size_t i = 0; i < std::min(results.size(), size_t(3)); i++) {
            std::cout << "    [" << i+1 << "] Distância: " 
                      << query.distanceTo(results[i]) 
                      << " - ID: " << results[i].id << std::endl;
        }
    } else {
        std::cout << "  Nenhum resultado encontrado no threshold especificado" << std::endl;
    }
    
    // FASE 4: ANÁLISE ESTRUTURAL (se disponível)
    // Usar duck typing para chamar análise específica
    if (auto* hashDB = dynamic_cast<HashSearch*>(&db)) {
        std::cout << "\nFASE 4: Análise Estrutural" << std::endl;
        hashDB->printAnalysis();
    } else if (auto* octreeDB = dynamic_cast<OctreeSearch*>(&db)) {
        std::cout << "\nFASE 4: Análise Estrutural" << std::endl;
        octreeDB->printAnalysis();
    } else if (auto* quadtreeDB = dynamic_cast<QuadtreeIterativeSearch*>(&db)) {
        std::cout << "\nFASE 4: Análise Estrutural" << std::endl;
        quadtreeDB->printAnalysis();
    }
}

// ============================================================================
// PROGRAMA PRINCIPAL - COMPARAÇÃO EXPERIMENTAL DAS 4 ESTRUTURAS
// ============================================================================

int main() {
    std::cout << std::string(80, '=') << std::endl;
    std::cout << "PROJETO DE ANÁLISE DE ALGORITMOS (PAA)" << std::endl;
    std::cout << "COMPARAÇÃO DE ESTRUTURAS DE DADOS PARA BUSCA POR SIMILARIDADE" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    // CONFIGURAÇÃO EXPERIMENTAL
    const int DATASET_SIZE = 2000;        // Tamanho do dataset
    const double QUERY_THRESHOLD = 40.0;   // Raio de busca
    const Image QUERY_POINT(0, "query.jpg", 128.0, 128.0, 128.0);  // Cinza médio
    
    std::cout << "\nCONFIGURAÇÃO DO EXPERIMENTO:" << std::endl;
    std::cout << "  Dataset sintético: " << DATASET_SIZE << " imagens RGB" << std::endl;
    std::cout << "  Espaço de busca: [0,255]³ (RGB)" << std::endl;
    std::cout << "  Distribuição: Uniforme" << std::endl;
    std::cout << "  Query point: RGB(" << QUERY_POINT.r << ", " 
              << QUERY_POINT.g << ", " << QUERY_POINT.b << ")" << std::endl;
    std::cout << "  Threshold: " << QUERY_THRESHOLD << std::endl;
    std::cout << "  Métrica: Distância euclidiana" << std::endl;
    
    // Geração do dataset sintético
    std::cout << "\nGerando dataset sintético..." << std::endl;
    std::vector<Image> syntheticDataset = generateSyntheticDataset(DATASET_SIZE);
    std::cout << "Dataset gerado: " << syntheticDataset.size() << " imagens" << std::endl;
    
    // ESTRUTURA 1: Busca Linear (Baseline)
    {
        LinearSearch linearDB;
        experimentalAnalysis(linearDB, syntheticDataset, QUERY_POINT, QUERY_THRESHOLD);
    }
    
    // ESTRUTURA 2: Hash Table Espacial
    {
        HashSearch hashDB(25.0);  // Cell size otimizado
        experimentalAnalysis(hashDB, syntheticDataset, QUERY_POINT, QUERY_THRESHOLD);
    }
    
    // ESTRUTURA 3: Octree 3D
    {
        OctreeSearch octreeDB(15);  // Max 15 imagens por nó
        experimentalAnalysis(octreeDB, syntheticDataset, QUERY_POINT, QUERY_THRESHOLD);
    }
    
    // ESTRUTURA 4: Quadtree 2D Iterativo
    {
        QuadtreeIterativeSearch quadtreeDB(30);  // Max 30 imagens por nó
        experimentalAnalysis(quadtreeDB, syntheticDataset, QUERY_POINT, QUERY_THRESHOLD);
    }
    
    // ANÁLISE TEÓRICA COMPARATIVA
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "ANÁLISE TEÓRICA DE COMPLEXIDADE" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    std::cout << "\n1. BUSCA LINEAR (Força Bruta):" << std::endl;
    std::cout << "   ├─ Inserção: O(1) - adiciona no final do array" << std::endl;
    std::cout << "   ├─ Busca: O(n) - examina todos os elementos" << std::endl;
    std::cout << "   ├─ Espaço: O(n) - armazena apenas os dados" << std::endl;
    std::cout << "   └─ Uso: Datasets pequenos, implementação simples" << std::endl;
    
    std::cout << "\n2. HASH TABLE ESPACIAL (Spatial Grid):" << std::endl;
    std::cout << "   ├─ Inserção: O(1) esperado - hash + insert" << std::endl;
    std::cout << "   ├─ Busca: O(k) onde k = células × densidade" << std::endl;
    std::cout << "   ├─ Espaço: O(n + m) onde m = número de células" << std::endl;
    std::cout << "   └─ Uso: Distribuição uniforme, busca rápida" << std::endl;
    
    std::cout << "\n3. OCTREE 3D (Árvore Espacial):" << std::endl;
    std::cout << "   ├─ Inserção: O(log n) esperado, O(h) onde h = altura" << std::endl;
    std::cout << "   ├─ Busca: O(log n + k) com poda geométrica eficiente" << std::endl;
    std::cout << "   ├─ Espaço: O(n + nós internos)" << std::endl;
    std::cout << "   └─ Uso: Datasets grandes, distribuição não-uniforme" << std::endl;
    
    std::cout << "\n4. QUADTREE 2D (Projeção Espacial):" << std::endl;
    std::cout << "   ├─ Inserção: O(log n) esperado no espaço 2D" << std::endl;
    std::cout << "   ├─ Busca: O(log n + k) com poda menos eficiente" << std::endl;
    std::cout << "   ├─ Espaço: O(n + nós internos), menor overhead" << std::endl;
    std::cout << "   └─ Uso: Datasets muito grandes, curse of dimensionality" << std::endl;
    
    // CONCLUSÕES E TRADE-OFFS
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "TRADE-OFFS E RECOMENDAÇÕES" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    std::cout << "\nFATORES DE ESCOLHA:" << std::endl;
    std::cout << "├─ Tamanho do dataset (n)" << std::endl;
    std::cout << "├─ Distribuição dos dados (uniforme vs clustered)" << std::endl;  
    std::cout << "├─ Dimensionalidade efetiva" << std::endl;
    std::cout << "├─ Frequência de inserções vs consultas" << std::endl;
    std::cout << "├─ Restrições de memória" << std::endl;
    std::cout << "└─ Complexidade de implementação" << std::endl;
    
    std::cout << "\nRECOMENDAÇÕES GERAIS:" << std::endl;
    std::cout << "• n < 1K: Linear Search (simplicidade)" << std::endl;
    std::cout << "• 1K < n < 10K: Hash Table (performance balanceada)" << std::endl;
    std::cout << "• 10K < n < 100K: Octree (poda eficiente)" << std::endl;
    std::cout << "• n > 100K: Quadtree (curse of dimensionality)" << std::endl;
    
    std::cout << "\n" << std::string(80, '=') << std::endl;
    std::cout << "FIM DA ANÁLISE EXPERIMENTAL" << std::endl;
    std::cout << std::string(80, '=') << std::endl;
    
    return 0;
}

/*
=============================================================================
CONCEITOS AVANÇADOS DE PAA DEMONSTRADOS:

1. ANÁLISE DE COMPLEXIDADE ASSINTÓTICA
   - Notação Big O para melhor/médio/pior caso
   - Trade-offs entre tempo e espaço
   - Complexidade amortizada

2. ESTRUTURAS DE DADOS ESPACIAIS
   - Particionamento do espaço (space partitioning)
   - Indexação multidimensional
   - Poda geométrica (geometric pruning)

3. TÉCNICAS ALGORÍTMICAS
   - Divisão e conquista (divide-and-conquer)
   - Hashing com resolução de colisões
   - Busca com poda (branch and bound)

4. OTIMIZAÇÕES DE PERFORMANCE
   - Localidade de memória (cache efficiency)
   - Implementação iterativa vs recursiva
   - Pré-computação e caching

5. ANÁLISE EXPERIMENTAL
   - Metodologia de benchmarking
   - Métricas de performance
   - Validação empírica de complexidade teórica

6. CURSE OF DIMENSIONALITY
   - Degradação de estruturas espaciais em alta dimensão
   - Técnicas de redução dimensional
   - Trade-offs entre precisão e eficiência
=============================================================================
*/