#ifndef IMAGE_DATABASE_H
#define IMAGE_DATABASE_H

#include <vector>
#include <string>
#include <cmath>

/**
 * @brief Representa uma imagem no espaço RGB
 *
 * Estrutura base para todas as implementações de busca.
 * Cada imagem possui um ID, nome do arquivo e coordenadas RGB.
 */
struct Image {
    int id;
    std::string filename;
    double r, g, b;  // Coordenadas no espaço RGB (0-255)

    Image(int _id, const std::string& _filename, double _r, double _g, double _b)
        : id(_id), filename(_filename), r(_r), g(_g), b(_b) {}

    /**
     * @brief Calcula a distância euclidiana entre duas imagens no espaço RGB
     * @param other Outra imagem para calcular a distância
     * @return Distância euclidiana 3D
     */
    double distanceTo(const Image& other) const {
        double dr = r - other.r;
        double dg = g - other.g;
        double db = b - other.b;
        return sqrt(dr*dr + dg*dg + db*db);
    }

    /**
     * @brief Imprime informações da imagem
     */
    void print() const {
        printf("Image %d (%s): RGB(%.1f, %.1f, %.1f)\n",
               id, filename.c_str(), r, g, b);
    }
};

/**
 * @brief Interface abstrata para estruturas de dados de busca de imagens
 *
 * Define o contrato que todas as estruturas de dados devem implementar,
 * permitindo comparações justas entre diferentes algoritmos.
 */
class ImageDatabase {
public:
    virtual ~ImageDatabase() = default;

    /**
     * @brief Insere uma imagem na estrutura de dados
     * @param img Imagem a ser inserida
     */
    virtual void insert(const Image& img) = 0;

    /**
     * @brief Busca imagens similares dentro de um threshold de distância
     * @param query Imagem de consulta
     * @param threshold Distância máxima para considerar similaridade
     * @return Vetor de imagens similares, ordenadas por distância
     */
    virtual std::vector<Image> findSimilar(const Image& query, double threshold) = 0;

    /**
     * @brief Retorna o nome da estrutura de dados
     * @return String identificando a implementação
     */
    virtual std::string getName() const = 0;
};

#endif // IMAGE_DATABASE_H
