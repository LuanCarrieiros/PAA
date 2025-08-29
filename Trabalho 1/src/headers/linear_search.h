#ifndef LINEAR_SEARCH_H
#define LINEAR_SEARCH_H

#include "main.cpp"

class LinearSearch : public ImageDatabase {
private:
    std::vector<Image> images;
    
public:
    void insert(const Image& img) override {
        images.push_back(img);
    }
    
    std::vector<Image> findSimilar(const Image& query, double threshold) override {
        std::vector<Image> results;
        
        // Busca linear - O(n)
        for (const auto& img : images) {
            double distance = query.distanceTo(img);
            if (distance <= threshold) {
                results.push_back(img);
            }
        }
        
        // Ordena por distancia (mais similares primeiro)
        std::sort(results.begin(), results.end(), 
                 [&query](const Image& a, const Image& b) {
                     return query.distanceTo(a) < query.distanceTo(b);
                 });
        
        return results;
    }
    
    std::string getName() const override {
        return "Linear Search";
    }
    
    size_t size() const {
        return images.size();
    }
};

#endif