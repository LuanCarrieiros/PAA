#ifndef OCTREE_SEARCH_H
#define OCTREE_SEARCH_H

#include <vector>
#include <algorithm>
#include <memory>

/**
 * @brief No da Octree para indexacao de imagens no espaco RGB 3D
 * 
 * Cada no representa uma regiao cubica do espaco RGB e pode ter ate 8 filhos,
 * dividindo o cubo em sub-regioes menores quando necessario.
 */
struct OctreeNode {
    // Limites da regiao que este no representa no espaco RGB
    double minR, maxR;  // Faixa do canal Red (0-255)
    double minG, maxG;  // Faixa do canal Green (0-255) 
    double minB, maxB;  // Faixa do canal Blue (0-255)
    
    // Imagens armazenadas neste no (so folhas tem imagens)
    std::vector<Image> images;
    
    // 8 filhos representando os 8 sub-cubos (octantes)
    // Ordem: [000, 001, 010, 011, 100, 101, 110, 111] em binario RGB
    std::array<std::unique_ptr<OctreeNode>, 8> children;
    
    // Flag para indicar se e um no folha
    bool isLeaf;
    
    /**
     * @brief Construtor do no
     * @param minR, maxR Limites do canal Red
     * @param minG, maxG Limites do canal Green  
     * @param minB, maxB Limites do canal Blue
     */
    OctreeNode(double minR, double maxR, double minG, double maxG, double minB, double maxB)
        : minR(minR), maxR(maxR), minG(minG), maxG(maxG), minB(minB), maxB(maxB), 
          isLeaf(true) {
        // Inicializa todos os filhos como nullptr
        children.fill(nullptr);
    }
    
    /**
     * @brief Verifica se uma imagem esta dentro dos limites deste no
     * @param img A imagem a ser testada
     * @return true se a imagem esta dentro dos limites, false caso contrario
     */
    bool contains(const Image& img) const {
        return img.r >= minR && img.r <= maxR &&
               img.g >= minG && img.g <= maxG &&
               img.b >= minB && img.b <= maxB;
    }
    
    /**
     * @brief Calcula o indice do filho onde uma imagem deve ser inserida
     * @param img A imagem para calcular o indice
     * @return Indice do filho (0-7) baseado na posicao RGB da imagem
     */
    int getChildIndex(const Image& img) const {
        int index = 0;
        double midR = (minR + maxR) / 2.0;
        double midG = (minG + maxG) / 2.0;  
        double midB = (minB + maxB) / 2.0;
        
        // Bit 0: Red >= midR
        if (img.r >= midR) index |= 4;
        // Bit 1: Green >= midG  
        if (img.g >= midG) index |= 2;
        // Bit 2: Blue >= midB
        if (img.b >= midB) index |= 1;
        
        return index;
    }
    
    /**
     * @brief Cria os 8 filhos dividindo o espaco atual em octantes
     * 
     * Divide cada dimensao RGB ao meio, criando 8 sub-cubos:
     * - 000 (0): minR-midR, minG-midG, minB-midB
     * - 001 (1): minR-midR, minG-midG, midB-maxB  
     * - ... e assim por diante
     */
    void createChildren() {
        double midR = (minR + maxR) / 2.0;
        double midG = (minG + maxG) / 2.0;
        double midB = (minB + maxB) / 2.0;
        
        // Cria os 8 octantes seguindo a ordem binaria RGB
        children[0] = std::make_unique<OctreeNode>(minR, midR, minG, midG, minB, midB); // 000
        children[1] = std::make_unique<OctreeNode>(minR, midR, minG, midG, midB, maxB); // 001  
        children[2] = std::make_unique<OctreeNode>(minR, midR, midG, maxG, minB, midB); // 010
        children[3] = std::make_unique<OctreeNode>(minR, midR, midG, maxG, midB, maxB); // 011
        children[4] = std::make_unique<OctreeNode>(midR, maxR, minG, midG, minB, midB); // 100
        children[5] = std::make_unique<OctreeNode>(midR, maxR, minG, midG, midB, maxB); // 101
        children[6] = std::make_unique<OctreeNode>(midR, maxR, midG, maxG, minB, midB); // 110  
        children[7] = std::make_unique<OctreeNode>(midR, maxR, midG, maxG, midB, maxB); // 111
        
        isLeaf = false; // Agora tem filhos, nao e mais folha
    }
};

/**
 * @brief Octree para busca eficiente de imagens similares no espaco RGB
 * 
 * Implementa uma arvore espacial 3D que divide hierarquicamente o espaco RGB
 * em regioes menores, permitindo buscas de proximidade mais eficientes que
 * a busca linear tradicional.
 * 
 * Baseado no algoritmo HOQ (Hierarchical Octa-partition Quantization)
 * mencionado no paper de referencia.
 */
class OctreeSearch : public ImageDatabase {
private:
    std::unique_ptr<OctreeNode> root;  // No raiz da arvore
    int maxImagesPerNode;              // Threshold para dividir nos
    int totalImages;                   // Contador total de imagens
    int maxDepth;                      // Profundidade maxima observada
    
    /**
     * @brief Insere uma imagem recursivamente na arvore
     * @param node No atual da insercao
     * @param img Imagem a ser inserida  
     * @param depth Profundidade atual (para estatisticas)
     */
    void insertRecursive(OctreeNode* node, const Image& img, int depth = 0) {
        maxDepth = std::max(maxDepth, depth);
        
        // Se e folha e nao excedeu o threshold, adiciona aqui
        if (node->isLeaf) {
            node->images.push_back(img);
            
            // Se excedeu o threshold, divide o no
            if (node->images.size() > maxImagesPerNode) {
                node->createChildren();
                
                // Redistribui as imagens existentes para os filhos
                for (const auto& existingImg : node->images) {
                    int childIdx = node->getChildIndex(existingImg);
                    insertRecursive(node->children[childIdx].get(), existingImg, depth + 1);
                }
                
                // Limpa as imagens do no pai (agora nao e mais folha)
                node->images.clear();
            }
        } else {
            // Nao e folha, insere no filho apropriado
            int childIdx = node->getChildIndex(img);
            insertRecursive(node->children[childIdx].get(), img, depth + 1);
        }
    }
    
    /**
     * @brief Busca imagens similares recursivamente na arvore
     * @param node No atual da busca
     * @param query Imagem de consulta
     * @param threshold Raio de busca (distancia maxima)
     * @param results Vetor para acumular resultados
     */
    void searchRecursive(OctreeNode* node, const Image& query, double threshold, 
                        std::vector<Image>& results) const {
        
        if (!node) return;
        
        // Verifica se o no pode conter imagens similares
        // Calcula distancia minima possivel do query para este cubo
        if (!nodeIntersectsQueryRadius(node, query, threshold)) {
            return; // Poda: este no nao pode ter resultados
        }
        
        if (node->isLeaf) {
            // No folha: verifica todas as imagens
            for (const auto& img : node->images) {
                double distance = query.distanceTo(img);
                if (distance <= threshold) {
                    results.push_back(img);
                }
            }
        } else {
            // No interno: busca recursivamente nos filhos
            for (const auto& child : node->children) {
                if (child) {
                    searchRecursive(child.get(), query, threshold, results);
                }
            }
        }
    }
    
    /**
     * @brief Verifica se um no pode conter imagens dentro do raio de busca
     * @param node No a ser testado
     * @param query Imagem de consulta  
     * @param threshold Raio de busca
     * @return true se o no intersecta com a esfera de busca
     */
    bool nodeIntersectsQueryRadius(OctreeNode* node, const Image& query, double threshold) const {
        // Calcula distancia minima do ponto query para o cubo do no
        double minDistSq = 0.0;
        
        // Para cada dimensao, calcula a distancia minima
        if (query.r < node->minR) {
            double diff = node->minR - query.r;
            minDistSq += diff * diff;
        } else if (query.r > node->maxR) {
            double diff = query.r - node->maxR;  
            minDistSq += diff * diff;
        }
        
        if (query.g < node->minG) {
            double diff = node->minG - query.g;
            minDistSq += diff * diff;
        } else if (query.g > node->maxG) {
            double diff = query.g - node->maxG;
            minDistSq += diff * diff;
        }
        
        if (query.b < node->minB) {
            double diff = node->minB - query.b;
            minDistSq += diff * diff;
        } else if (query.b > node->maxB) {
            double diff = query.b - node->maxB;
            minDistSq += diff * diff;
        }
        
        // Se distancia minima <= threshold, pode ter resultados
        return sqrt(minDistSq) <= threshold;
    }
    
    /**
     * @brief Conta nos recursivamente para estatisticas
     * @param node No atual
     * @param leafCount Contador de folhas (por referencia)
     * @param internalCount Contador de nos internos (por referencia)
     */
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
    /**
     * @brief Construtor da Octree
     * @param maxImages Numero maximo de imagens por no antes da divisao
     */
    OctreeSearch(int maxImages = 10) 
        : maxImagesPerNode(maxImages), totalImages(0), maxDepth(0) {
        // Cria raiz cobrindo todo o espaco RGB (0-255 em cada canal)
        root = std::make_unique<OctreeNode>(0, 255, 0, 255, 0, 255);
    }
    
    void insert(const Image& img) override {
        insertRecursive(root.get(), img);
        totalImages++;
    }
    
    std::vector<Image> findSimilar(const Image& query, double threshold) override {
        std::vector<Image> results;
        searchRecursive(root.get(), query, threshold, results);
        
        // Ordena resultados por distancia (mais similares primeiro)
        std::sort(results.begin(), results.end(), 
                 [&query](const Image& a, const Image& b) {
                     return query.distanceTo(a) < query.distanceTo(b);
                 });
        
        return results;
    }
    
    std::string getName() const override {
        return "Octree Search (maxPerNode=" + std::to_string(maxImagesPerNode) + ")";
    }
    
    /**
     * @brief Imprime estatisticas detalhadas da Octree
     */
    void printStats() const {
        int leafCount = 0, internalCount = 0;
        countNodes(root.get(), leafCount, internalCount);
        
        std::cout << "Octree Stats:" << std::endl;
        std::cout << "  Total de imagens: " << totalImages << std::endl;
        std::cout << "  Max imagens por no: " << maxImagesPerNode << std::endl;
        std::cout << "  Profundidade maxima: " << maxDepth << std::endl;
        std::cout << "  Nos folha: " << leafCount << std::endl;
        std::cout << "  Nos internos: " << internalCount << std::endl;
        std::cout << "  Total de nos: " << (leafCount + internalCount) << std::endl;
        
        if (leafCount > 0) {
            std::cout << "  Imagens por folha (media): " 
                      << static_cast<double>(totalImages) / leafCount << std::endl;
        }
    }
};

#endif