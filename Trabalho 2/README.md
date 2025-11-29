# Trabalho 2 - Estruturas para Alta Dimensionalidade

**PAA (Projeto e Análise de Algoritmos)** | PUC Minas | 2025/2

---

## 🎯 Visão Geral

Comparação de estruturas de busca por similaridade em **dois contextos**:
- **ColorHistogram 32D** (68K vetores) - Alta dimensionalidade REAL
- **RGB 3D** (26K imagens) - Baixa dimensionalidade

**Estruturas testadas:**
1. **LinearSearch** - Baseline O(n)
2. **LSH** (Locality-Sensitive Hashing) - Busca aproximada sublinear
3. **M-Tree** - Árvore métrica com poda
4. **+ 4 estruturas espaciais** (apenas RGB 3D): Hash, Octree, Quadtree, Hash Dynamic

---

## 🚀 Compilação Rápida

### Pré-requisitos:
```bash
# Ubuntu/Debian
sudo apt install g++ libopencv-dev make

# Windows (MSYS2)
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-opencv make
```

### Compilar e Executar:
```bash
cd "Trabalho 2"
make clean
make run          # Modo RGB 3D (padrão)
make run-hist     # Modo ColorHistogram 32D
make help         # Ver todos os comandos
```

**Ou manualmente:**
```bash
g++ -std=c++17 -O2 -Isrc -o bin/trabalho2 src/main.cpp $(pkg-config --cflags --libs opencv4)
./bin/trabalho2        # RGB 3D
./bin/trabalho2 hist   # ColorHistogram 32D
```

---

## 📊 Resultados - ColorHistogram 32D (Principal)

**Dataset:** 68,040 vetores de 32 dimensões (histogramas de cores)
**Query:** Primeiro vetor (ID=1)
**Threshold:** 0.3 (distância Euclidiana normalizada)

### Performance em Escala (50K vetores):

| Estrutura | Insert (ms) | Search (ms) | Speedup | Precisão |
|-----------|-------------|-------------|---------|----------|
| **LinearSearch** | 15.2 | 4.83 | 1.0× | 100% (baseline) |
| **LSH (L=10, k=8)** | 88.7 | **1.67** | **2.9×** | ~95% |
| **M-Tree** | 16.1 | 4.91 | 1.0× | 100% |

### Escalas Completas (1K até 68K):

```
[TESTANDO] Escala: 1000 vetores 32D
  LinearSearch_32D              : Insert=   0.32ms, Search=   0.09ms, Found=    12
  LSH_32D(L=10,k=8)             : Insert=   1.89ms, Search=   0.04ms, Found=    11
  MTree_32D(simplified)         : Insert=   0.35ms, Search=   0.11ms, Found=    12

[TESTANDO] Escala: 50000 vetores 32D
  LinearSearch_32D              : Insert=  15.20ms, Search=   4.83ms, Found=   892
  LSH_32D(L=10,k=8)             : Insert=  88.73ms, Search=   1.67ms, Found=   845
  MTree_32D(simplified)         : Insert=  16.08ms, Search=   4.91ms, Found=   892
```

**Conclusão:** LSH demonstra **clara vantagem em 32D** - busca 2-3× mais rápida mantendo ~95% de precisão.

---

## 📊 Resultados - RGB 3D (Validação)

**Dataset:** 26,179 imagens Animals-10 (Kaggle)
**Query:** Seleção interativa da pasta `./query/`
**Threshold:** 40.0 (distância Euclidiana RGB)

### Performance (26K imagens):

| Estrutura | Insert (ms) | Search (ms) | Found | Recall |
|-----------|-------------|-------------|-------|--------|
| **Linear Search** | 1.00 | 2.32 | 8380 | 100% |
| **Octree** | 5.48 | **2.12** | 8380 | 100% |
| **Quadtree** | 6.92 | 2.25 | 8380 | 100% |
| **Hash Dynamic** | 4.14 | 2.15 | 8380 | 100% |
| **LSH** | 53.91 | 4.62 | 8039 | 95.9% |
| **M-Tree** | 11.85 | 2.46 | 7650 | 91.3% |

**Conclusão:** Em RGB 3D, **estruturas clássicas dominam** - Octree/Quadtree são mais simples, rápidos e precisos.

---

## 🔍 Interpretação dos Resultados

### Bidirectional Curse of Dimensionality

**LSH em 32D (ColorHistogram):**
- ✅ **Vence** - 2.9× mais rápido que linear
- ✅ Validado em contexto de alta dimensão
- ✅ Trade-off precisão/velocidade aceitável

**LSH em 3D (RGB):**
- ❌ **Perde** - 2.2× mais lento que Octree
- ❌ Overhead desnecessário em baixa dimensão
- ❌ Pior que estruturas espaciais simples

### Guideline de Seleção:

| Dimensões | Dataset | Melhor Escolha | Justificativa |
|-----------|---------|----------------|---------------|
| **Alta (32D+)** | ColorHistogram | **LSH** | Busca sublinear, escalável |
| **Baixa (3D)** | RGB Imagens | **Octree/Quadtree** | Simples, exato, rápido |
| **Qualquer** | Pequeno (<5K) | **Linear** | Overhead não compensa |

---

## 📁 Datasets

### 1. ColorHistogram 32D (Principal)
- **Arquivo:** `database_colorhistogram.asc/ColorHistogram.asc`
- **Formato:** Texto (ID + 32 valores separados por espaço)
- **Tamanho:** 68,040 vetores
- **Origem:** [UCI Corel Image Features Dataset](https://archive.ics.uci.edu/dataset/119/corel+image+features)
- **Como usar:** Já incluído no repositório

### 2. RGB 3D (Validação)
- **Fonte:** [Animals-10 Kaggle](https://www.kaggle.com/datasets/alessiocorrado99/animals10)
- **Tamanho:** 26,179 imagens (.jpg/.png)
- **Preparação:**
  ```bash
  mkdir -p images query
  # Baixar e extrair Animals-10 para ./images/
  # Copiar 2-3 imagens para ./query/ (seleção interativa)
  ```

---

## 🛠️ Estrutura do Projeto

```
Trabalho 2/
├── Makefile                          # Build system profissional
├── README.md                         # Esta documentação
├── src/
│   ├── main.cpp                      # Programa principal (ambos modos)
│   └── headers/
│       ├── image_database.h          # Interface base
│       ├── lsh_search.h              # LSH para RGB 3D
│       ├── mtree_search.h            # M-Tree para RGB 3D
│       ├── octree_search.h           # Octree (apenas RGB)
│       └── ...                       # Outras estruturas
├── database_colorhistogram.asc/
│   └── ColorHistogram.asc            # Dataset 32D (68K vetores)
├── images/                           # Dataset RGB 3D (26K imagens)
├── query/                            # Imagens de query (RGB 3D)
├── resultados/                       # Resultados com timestamp
└── LaTex/
    └── main.tex                      # Relatório JBCS
```

---

## 📖 Uso Detalhado

### Modo ColorHistogram 32D:
```bash
make run-hist

# Saída:
# MODO: ColorHistogram 32D (database_colorhistogram.asc)
# Carregando 1000 vetores...
# LinearSearch_32D: Insert=0.32ms, Search=0.09ms, Found=12
# LSH_32D: Insert=1.89ms, Search=0.04ms, Found=11
# ...
# ✅ Resultados salvos em: resultados_hist32d_DD-MM-YYYY_HHMMSS.txt
```

### Modo RGB 3D:
```bash
make run

# Menu interativo:
# SELEÇÃO DE IMAGEM DE QUERY
# [ 1] dog_001.jpg
# [ 2] cat_045.jpg
# [ 0] Usar query padrão
# Digite sua escolha: 1
#
# ✓ Query REAL carregada: ./query/dog_001.jpg
# [TESTANDO] Escala: 1000 imagens REAIS
# Linear Search: Insert=0.01ms, Search=0.36ms, Found=303
# ...
# ✅ Resultados salvos em: resultados_DD-MM-YYYY_HHMMSS.txt
```

---

## 🎓 Conceitos PAA Demonstrados

1. **Alta Dimensionalidade:** LSH, random projections, 32D vectors
2. **Curse of Dimensionality:** Bidirectional (high-dim structures fail in low-dim)
3. **Spatial Indexing:** Octree, Quadtree (3D partitioning)
4. **Metric Trees:** M-Tree, triangle inequality pruning
5. **Trade-offs:** Exato vs Aproximado, Inserção vs Busca

---

## 📚 Referências

- **LSH:** Indyk & Motwani (1998) - Approximate Nearest Neighbors
- **M-Tree:** Ciaccia et al. (1997) - M-tree: An Efficient Access Method
- **ColorHistogram:** Standard benchmark dataset for high-dimensional indexing

---

## ⚙️ Comandos Makefile

```bash
make              # Compila projeto
make run          # Executa modo RGB 3D
make run-hist     # Executa modo ColorHistogram 32D
make clean        # Limpa binários
make help         # Mostra todos os comandos
```

---

**Autores:** Luan Carrieiros, Diego Rocha, Iago Ribeiro, Bernardo Temponi, Arthur Moraes
**Instituição:** PUC Minas - ICEI
**Ano:** 2025/2
