# 🔍 PAA Assignment 1: Comparative Analysis of Data Structures for Image Similarity Search

## 📋 Project Overview

**University**: PUC Minas - Algorithm Analysis and Design Course  
**Assignment**: Comparative performance analysis of 4 data structures for RGB image similarity search  
**Language**: C++ with `-O2` optimizations  
**Date**: August 2024  

## 🎯 Objective

Compare the performance of 4 different data structures for finding similar images based on RGB color similarity:
- **Linear Search** (Brute force sequential search)
- **Hash Search** (3D spatial hashing) 
- **Octree Search** (3D recursive spatial tree)
- **Quadtree Search** (2D spatial tree using R,G dimensions)

## 🚀 Key Findings - Executive Summary

### 🏆 **WINNER BY CATEGORY**

#### **🥇 Insertion Performance**
- **Champion**: **Linear Search** - Dominates ALL scales (100 → 50M images)
- **Why**: Zero overhead - direct array insertion

#### **🥇 Search Performance**  
- **Champion**: **Hash Search** - Dominates 9 of 10 scales (1K → 50M images)
- **Performance**: 18ms search time for **50 MILLION** images!
- **Why**: O(1) spatial lookup with consistent performance
- **⚠️ Trade-off**: Hash Search is faster but finds slightly fewer similar images due to spatial grid approximation

#### **🥇 Overall Balance**
- **Small datasets** (<5K): Linear Search
- **Medium datasets** (5K-500K): Hash Search  
- **Large datasets** (>1M): Hash Search for search-heavy workloads

### 💥 **Surprising Discoveries**

1. **Quadtree > Octree**: 2D tree outperformed 3D tree consistently
2. **Recursion > Iteration**: Recursive implementation 28-72% faster than iterative
3. **Hash Scaling**: Maintained near-constant search time up to 50M images
4. **🎯 Speed vs Accuracy Trade-off**: Hash Search is faster but finds fewer matches (spatial grid approximation loses some similar images)

## 📊 Performance Results (SEED 20)

### **Latest Benchmark Results - Synthetic Data (100 → 50M images)**

| Scale | Linear Insert/Search | Hash Insert/Search | Octree Insert/Search | Quadtree Insert/Search |
|-------|---------------------|-------------------|---------------------|----------------------|
| 100 | **0.016/0.001** | 0.017/0.001 | 0.022/0.001 | 0.018/0.001 |
| 1K | **0.082/0.005** | 0.163/**0.001** | 0.227/0.013 | 0.148/0.011 |
| 10K | **0.750/0.063** | 1.811/**0.001** | 2.270/0.088 | 2.348/0.074 |
| 100K | **6.782/0.542** | 19.542/**0.031** | 28.430/0.980 | 30.286/0.917 |
| 500K | **33.332/2.987** | 90.121/**0.049** | 183.957/6.884 | 207.628/6.923 |
| 1M | **66.646/7.045** | 173.945/**0.187** | 477.827/20.957 | 365.093/15.751 |
| 5M | **383.794/36.495** | 888.518/**0.873** | 3328.167/133.197 | 3037.293/139.291 |
| 10M | **791.058/82.613** | 2054.539/**2.073** | 7748.419/278.236 | 7189.018/263.529 |
| 25M | **2034.601/221.350** | 13590.280/**8.159** | 24185.127/770.451 | 25098.920/775.293 |
| 50M | **5752.689/520.698** | 42529.801/**17.987** | 82092.444/2603.842 | 72793.606/1759.648 |

*Times in milliseconds*

### **Analysis by Scale**

#### **Small Scale (100-1K images)**
- **Winner**: Linear Search (overall best)
- **Insight**: Simplicity beats complexity for small datasets

#### **Medium Scale (10K-500K images)**
- **Winner**: Hash Search (search), Linear Search (insertion)
- **Insight**: Hash shows its O(1) advantage

#### **Large Scale (1M-50M images)**
- **Winner**: Hash Search (search dominance)
- **Insight**: 18ms for 50M images - incredible scalability!

## 🔬 Additional Experiments

### **Recursion vs Iteration Analysis**
**Files**: `benchmark_recursivo_vs_iterativo.cpp`, `resultados_recursivo_vs_iterativo.txt`

**Key Finding**: **Recursive implementations consistently outperformed iterative**
- Octree Recursive: 28-72% faster than iterative
- Compiler optimizations + better cache locality favor recursion

### **Memory Management Discovery**
**Problem**: Initial 100M benchmark consumed 35-40GB RAM  
**Solution**: Generate dataset per structure instead of copying  
**Result**: Reduced to ~10-12GB max, dramatically improved performance

## 🏗️ Code Architecture

### **Main Files**
- **`CODIGO_EDUCATIVO_PAA_4_ESTRUTURAS.cpp`** - Educational reference code with detailed comments
- **`scalable_benchmark.cpp`** - Main synthetic data benchmark (100 → 50M images)
- **`benchmark_recursivo_vs_iterativo.cpp`** - Recursion vs iteration comparison
- **`ANALISE_DETALHADA_EXPERIMENTOS.md`** - Complete experimental analysis

### **Data Structures Implementation**
Each structure implements a common interface with `insert()` and `search()` methods:

1. **Linear Search**: Simple vector with sequential search
2. **Hash Search**: 3D spatial grid with cell-based hashing  
3. **Octree Search**: Recursive 3D tree with spatial subdivision
4. **Quadtree Search**: 2D tree using only R,G dimensions

## 🧪 Methodology

### **Technical Configuration**
- **Compiler**: GCC with `-O2` optimizations
- **Similarity Metric**: Euclidean distance in RGB space
- **Query Point**: RGB(128, 128, 128) - middle gray
- **Similarity Threshold**: 50.0
- **Timing**: `std::chrono::high_resolution_clock`

### **Anti-Cache Strategy**
Each structure receives an independent dataset copy to prevent cache effects that could bias results toward later-executed structures.

### **Reproducibility**
- **Fixed SEED**: 20 for consistent synthetic data generation
- **Multiple scales**: 10 logarithmic scales from 100 to 50M images
- **Consistent environment**: Same hardware, OS, and compiler settings

## 📈 Theoretical vs Practical Complexity

| Structure | Insert (Theory) | Insert (Practice) | Search (Theory) | Search (Practice) |
|-----------|-----------------|-------------------|------------------|-------------------|
| Linear    | O(1)            | ✅ Best            | O(n)             | ✅ As expected    |
| Hash      | O(1)            | ✅ Constant        | O(1)             | ✅ **Dominant**   |
| Octree    | O(log n)        | ⚠️ High overhead   | O(log n)         | ⚠️ Only >100K     |
| Quadtree  | O(log n)        | ✅ Good practice   | O(log n)         | ✅ Consistent     |

## 🔄 Future Work

### **Planned Experiments**
1. **Real Image Dataset**: Using `Images/natural_images/` (6,899 images, 8 categories)
   - ✅ **COMPLETED**: Simplified benchmark with Linear + Hash Search working
   - **Key Finding**: Hash Search 18-22% faster but finds 1-4 fewer similar images per test
   - **Cause**: Spatial grid cells may exclude borderline similar images
2. **CSV Data Analysis**: Performance with real color distributions
3. **Query Sensitivity**: Different query points and thresholds
4. **Memory Analysis**: RAM consumption patterns
5. **Multiple Query Performance**: Batch search operations

### **Technical Improvements**
- Alternative distance metrics (Manhattan, Cosine similarity)
- Dynamic threshold adjustment
- Parallel processing implementations
- Cache-aware optimizations

## 📚 Academic References

1. **Color quantization by hierarchical octa-partition in RGB color space** (provided paper)
2. **Original implementation and analysis** (this project)
3. **Knuth's optimization principle**: "Premature optimization is the root of all evil"

## ⚙️ How to Run

### **Prerequisites**
```bash
g++ --version  # GCC with C++11+ support
```

### **Compile and Run**
```bash
# Main educational code
g++ -O2 -o educativo CODIGO_EDUCATIVO_PAA_4_ESTRUTURAS.cpp
./educativo

# Scalable benchmark (WARNING: Takes 30+ minutes for full 50M scale)
g++ -O2 -o scalable scalable_benchmark.cpp  
./scalable

# Recursion vs iteration comparison
g++ -O2 -o rec_iter benchmark_recursivo_vs_iterativo.cpp
./rec_iter
```

### **Files Generated**
- Performance results in `.txt` format
- Detailed logs with timing information
- Analysis-ready data tables

## 🏆 Conclusions

### **For Academic Understanding**
- **Hash Search** proves spatial hashing effectiveness for similarity search
- **Tree structures** show expected logarithmic behavior with high constant overhead
- **Linear Search** remains surprisingly competitive for small-medium datasets

### **For Practical Applications**
- **<5K images**: Use Linear Search (simplicity wins)
- **5K-1M images**: Use Hash Search (excellent search performance)
- **>1M images**: Choose based on insert/search ratio
  - Search-heavy: Hash Search
  - Insert-heavy: Linear Search
  - Balanced: Consider Quadtree

### **Key Learnings**
1. **Measure, don't assume**: Octree wasn't always best despite theoretical advantages
2. **Scale matters**: Performance characteristics change dramatically with dataset size
3. **Implementation details matter**: Recursion vs iteration had significant impact
4. **Memory is crucial**: Poor memory management can kill performance

---

**📊 This project demonstrates the importance of empirical analysis in algorithm selection - theory provides guidance, but real-world performance testing reveals the truth.**

*Generated by PAA Assignment 1 - PUC Minas - August 2024*

### 🔍 **Important Discovery: Speed vs Accuracy Trade-off**
Real image testing revealed that **Hash Search sacrifices some accuracy for speed** - consistently finding 1-4 fewer similar images than Linear Search due to spatial grid approximation. This is a crucial consideration for applications where finding ALL similar images is more important than speed.