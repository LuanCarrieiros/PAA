# 🚀 Como Compilar e Executar - PAA Trabalho 1

## 📁 Estrutura do Projeto

```
PAA/
├── README.md                                    # 📚 Visão geral do projeto
├── src/
│   ├── PAA_Trabalho1.cpp                       # 🎓 Código educativo principal
│   ├── headers/                                # 📂 Bibliotecas desenvolvidas
│   │   ├── hash_search.h
│   │   ├── linear_search.h
│   │   ├── octree_search.h
│   │   ├── octree_iterative.h
│   │   ├── quadtree.h
│   │   └── stb_image.h                         # Para processamento de imagens
│   └── benchmarks/                             # 🧪 Experimentos
│       ├── scalable_benchmark.cpp              # Benchmark principal (100→50M)
│       ├── benchmark_recursivo_vs_iterativo.cpp
│       ├── benchmark_100M_only.cpp
│       ├── benchmark_imagens_simples.cpp       # ✅ : Imagens reais
│       └── benchmark_imagens_locais.cpp
├── resultados/                                 # 📊 Resultados experimentais
│   ├── resultados50Mseed42.txt
│   └── resultadosOctaQuad.txt
└── docs/                                       # 📖 Documentação
    └── ANALISE_DETALHADA_EXPERIMENTOS.md
```

## ⚙️ Comandos de Compilação

### **🎓 Código Educativo Principal**
```bash
g++ -O2 -o paa_trabalho1 src/PAA_Trabalho1.cpp
./paa_trabalho1
```

### **🚀 Benchmark Sintético Principal (100 → 50M imagens)**
```bash
g++ -O2 -o scalable src/benchmarks/scalable_benchmark.cpp
./scalable
# ⚠️ AVISO: Pode levar 30+ minutos para 50M imagens
```

### **📸 Benchmark com Imagens Reais **
```bash
g++ -O2 -std=c++17 -o img_benchmark src/benchmarks/benchmark_imagens_simples.cpp
./img_benchmark
# Requer: pasta Images/natural_images/ com imagens do Kaggle
```

### **🔄 Comparação Recursão vs Iteração**
```bash
g++ -O2 -o rec_iter src/benchmarks/benchmark_recursivo_vs_iterativo.cpp
./rec_iter
```

### **💾 Benchmark Específico 100M (Experimental)**
```bash
g++ -O2 -o benchmark_100m src/benchmarks/benchmark_100M_only.cpp
./benchmark_100m
# ⚠️ AVISO: Requer ~12GB RAM (com 16GB após 2 horas não consegui)
```

## 🎯 Principais Descobertas

- **Hash Search**: Domina busca (18.7ms para 50M imagens!)
- **Linear Search**: Domina inserção (zero overhead)
- **Quadtree > Octree**: 2D supera 3D consistentemente
- **Recursão > Iteração**: 28-72% mais rápido
- **Trade-off Descoberto**: Hash é mais rápida mas encontra menos imagens similares

## 📚 Documentação Completa

- **README.md**: Visão geral e resultados principais
- **docs/ANALISE_DETALHADA_EXPERIMENTOS.md**: Análise técnica completa
- **resultados/**: Dados brutos dos experimentos

---
*Projeto PAA - PUC Minas - Agosto 2025*