# Comparative Analysis of Data Structures for Image Similarity Search

## Project Overview

**Institution**: PUC Minas - Algorithm Analysis and Design Course  
**Assignment**: Empirical performance analysis of data structures for RGB image similarity search  
**Language**: C++17 with compiler optimizations  
**Date**: August 2025  

**Team Members**:
- Luan Barbosa Rosa Carrieiros
- Diego Moreira Rocha  
- Iago Fereguetti Ribeiro
- Bernardo Ferreira Temponi

## Research Objective

This study compares the performance characteristics of five distinct data structures for finding similar images based on RGB color space similarity:

1. **Linear Search** - Sequential brute force approach
2. **Hash Search** - 3D spatial hashing with grid-based indexing
3. **Hash Dynamic Search** - Adaptive expansion spatial hashing
4. **Octree Search** - 3D recursive spatial tree subdivision
5. **Quadtree Search** - 2D spatial tree using R,G dimensions

## Key Research Findings

### Performance Champions by Category

#### Insertion Performance
**Winner**: Linear Search - Consistent dominance across all scales  
**Advantage**: Zero structural overhead with direct array insertion  
**Performance**: O(1) theoretical and practical complexity

#### Search Performance  
**Winner**: Hash Search - Superior performance for datasets >1K images  
**Achievement**: 18ms search time for 50 million images  
**Advantage**: O(1) spatial lookup with consistent scaling behavior

#### Precision vs Speed Trade-off
**Discovery**: Hash-based structures sacrifice 5-15% accuracy for significant speed improvements  
**Implication**: Choice depends on application requirements (speed vs completeness)

### Unexpected Research Outcomes

1. **2D vs 3D Trees**: Quadtree consistently outperformed Octree despite using fewer dimensions
2. **Recursion Advantage**: Recursive implementations achieved 28-72% better performance than iterative versions
3. **Scaling Behavior**: Hash structures maintained near-constant search times up to 50M images
4. **Memory Impact**: Proper memory management reduced RAM consumption from 40GB to 12GB

## Experimental Results

### Large-Scale Synthetic Benchmark (100 → 50M images)

| Scale | Linear (ms) | Hash (ms) | Hash Dynamic (ms) | Octree (ms) | Quadtree (ms) |
|-------|-------------|-----------|-------------------|-------------|---------------|
| 1K    | 0.082/0.005 | 0.163/**0.001** | 0.165/0.002 | 0.227/0.013 | 0.148/0.011 |
| 10K   | 0.750/0.063 | 1.811/**0.001** | 1.820/0.011 | 2.270/0.088 | 2.348/0.074 |
| 100K  | **6.782**/0.542 | 19.542/**0.031** | 19.680/0.098 | 28.430/0.980 | 30.286/0.917 |
| 1M    | **66.646**/7.045 | 173.945/**0.187** | 174.220/0.445 | 477.827/20.957 | 365.093/15.751 |
| 50M   | **5752.689**/520.698 | 42529.801/**17.987** | 42847.132/89.441 | 82092.444/2603.842 | 72793.606/1759.648 |

*Format: Insertion/Search times in milliseconds*

### Real Image Dataset Testing (7,721 images)

Evaluation using natural images dataset with 8 categories (airplane, car, cat, dog, flower, fruit, motorbike, person):

| Dataset Size | Linear Found | Hash Found | Hash Dynamic Found | Accuracy Ratio |
|--------------|--------------|------------|-------------------|----------------|
| 500          | 171          | 146        | 171               | 85.4% / 100%   |
| 1000         | 357          | 307        | 357               | 86.0% / 100%   |
| 7721         | 2858         | 2471       | 2858              | 86.5% / 100%   |

**Key Insight**: Hash Dynamic Search achieves Linear Search precision while maintaining superior performance characteristics.

## Research Methodology

### Technical Configuration
- **Compiler**: GCC with -O2 optimization flags
- **Distance Metric**: Euclidean distance in RGB color space  
- **Evaluation Query**: RGB(128, 128, 128) center point
- **Similarity Threshold**: 50.0 units
- **Timing Mechanism**: High-resolution chrono library

### Experimental Design
- **Memory Pattern**: CREATE→TEST→DESTROY to optimize RAM usage
- **Anti-Cache Strategy**: Independent dataset generation per structure
- **Reproducibility**: Fixed seed (20) for consistent synthetic data
- **Scale Testing**: Logarithmic scaling from 100 to 50M images

## Implementation Architecture

### Core Design Patterns
All data structures implement a unified `ImageDatabase` interface ensuring fair comparison:

```cpp
class ImageDatabase {
public:
    virtual void insert(const Image& img) = 0;
    virtual std::vector<Image> findSimilar(const Image& query, double threshold) = 0;
    virtual std::string getName() const = 0;
};
```

### Memory Management Innovation
Implementation uses RAII (Resource Acquisition Is Initialization) with smart pointers to prevent memory leaks and optimize resource usage during large-scale testing.

## Theoretical vs Empirical Analysis

| Structure | Insert Complexity | Search Complexity | Practical Performance |
|-----------|-------------------|-------------------|----------------------|
| Linear    | O(1)             | O(n)              | Excellent for <5K    |
| Hash      | O(1)             | O(1)              | Outstanding for >1K   |
| Hash Dynamic | O(1)          | O(r³)             | Precision-optimized  |
| Octree    | O(log n)         | O(log n)          | High overhead        |
| Quadtree  | O(log n)         | O(log n)          | Consistent scaling   |

## Academic Contributions

### Novel Findings
1. **Dimensionality Paradox**: 2D spatial structures outperformed 3D equivalents
2. **Memory-Performance Relationship**: Demonstrated critical impact of memory management on algorithmic performance
3. **Precision-Speed Trade-off Quantification**: Empirical measurement of accuracy loss in approximate spatial structures

### Research Applications
- Computer vision similarity search optimization
- Large-scale image database indexing strategies
- Real-time image processing system design

## Usage Instructions

### Prerequisites
- GCC compiler with C++17 support
- Minimum 16GB RAM for large-scale testing

### Compilation and Execution
```bash
# Main educational demonstration (5 structures comparison)
g++ -O2 -std=c++17 -o main src/main.cpp
./main

# Large-scale synthetic benchmark (100 → 50M images)
g++ -O2 -o scalable src/benchmarks/scalable_benchmark.cpp
./scalable

# Real image dataset evaluation (requires images/ directory)
g++ -O2 -std=c++17 -o img_benchmark benchmark_imagens_locais.exe
./img_benchmark

# 100M images benchmark (extreme scale testing)
g++ -O2 -o benchmark_100M src/benchmarks/benchmark_100M_only.cpp
./benchmark_100M
```

## Future Research Directions

### Planned Investigations
1. **Alternative Distance Metrics**: Manhattan, Cosine similarity evaluation
2. **Parallel Processing**: Multi-threading implementation analysis  
3. **Dynamic Threshold Optimization**: Adaptive similarity boundary adjustment
4. **Hybrid Approaches**: Combining multiple data structures for optimal performance

### Technical Improvements
- Cache-aware memory access patterns
- GPU acceleration for large-scale processing
- Advanced spatial partitioning algorithms

## Conclusions

### Academic Insights
This research demonstrates that empirical analysis is essential for algorithmic selection in practical applications. Theoretical complexity analysis provides valuable guidance, but real-world performance characteristics often diverge from theoretical predictions due to implementation details, memory hierarchy effects, and constant factor variations.

### Practical Recommendations
- **Small datasets (<5K images)**: Linear Search for simplicity and performance
- **Medium datasets (5K-1M images)**: Hash Search for optimal search performance
- **Large datasets (>1M images)**: Selection based on precision requirements
  - Speed priority: Hash Search
  - Precision priority: Hash Dynamic Search
  - Balanced approach: Quadtree

### Key Research Impact
This study provides quantitative evidence for the importance of empirical algorithm analysis and establishes performance benchmarks for image similarity search applications in computer science research and industry applications.

---

*This research was conducted as part of the Algorithm Analysis and Design course at PUC Minas, contributing to the understanding of practical data structure performance in image processing applications.*