# Trabalho 2 - Estruturas de Dados para Alta Dimensionalidade

**Disciplina**: Projeto e Análise de Algoritmos (PAA)
**Instituição**: PUC Minas - Ciência da Computação
**Período**: 2025/2

## 📋 Visão Geral

Este trabalho é uma **continuação do Trabalho 1**, implementando e comparando estruturas de dados especializadas para **busca de similaridade em alta dimensionalidade**. Enquanto o Trabalho 1 analisou estruturas básicas (Linear, Hash, Quadtree, Octree), o Trabalho 2 adiciona duas estruturas avançadas:

- **LSH (Locality-Sensitive Hashing)** - Busca aproximada sublinear
- **M-Tree (Metric Tree)** - Busca exata com poda baseada em desigualdade triangular

**Total**: **7 estruturas comparadas** em um único framework de benchmark!

---

## 🎯 Objetivos

1. ✅ Implementar estruturas de dados para **alta dimensionalidade**
2. ✅ Comparar com estruturas básicas do Trabalho 1
3. ✅ Analisar **trade-offs** entre busca exata vs aproximada
4. ✅ Avaliar impacto da **curse of dimensionality**
5. ✅ Documentar resultados em relatório JBCS (até 4 páginas)

---

## 📊 Estruturas Implementadas

### **Trabalho 1 - Estruturas Básicas** (5 estruturas)

| Estrutura | Tipo | Complexidade Busca | Aplicação |
|-----------|------|-------------------|-----------|
| Linear Search | Força bruta | O(n) | Baseline, datasets pequenos |
| Hash Search | Spatial hashing 3D | O(1) esperado | Busca rápida em 3D |
| Hash Dynamic | Hashing adaptativo | O(r³) | Precisão moderada |
| Octree | Árvore espacial 3D | O(log n) | Subdivisão recursiva |
| Quadtree | Árvore espacial 2D | O(log n) | Projeção 2D+1 |

### **Trabalho 2 - Alta Dimensionalidade** 🆕 (2 estruturas novas)

| Estrutura | Tipo | Complexidade Busca | Aplicação |
|-----------|------|-------------------|-----------|
| **LSH** 🆕 | Hashing probabilístico | O(n^ρ), ρ<1 | **Busca aproximada rápida** |
| **M-Tree** 🆕 | Árvore métrica | O(log n + k) | **Busca exata em espaços métricos** |

---

## 🔬 Implementações Detalhadas

### 1️⃣ **LSH (Locality-Sensitive Hashing)**

**Arquivo**: `src/headers/lsh_search.h`

#### **Conceito:**
LSH é uma técnica probabilística que usa funções hash especiais onde pontos similares têm **alta probabilidade de colidirem no mesmo bucket**.

#### **Técnica Implementada:**
- **Random Projections** (projeções aleatórias)
- Projeta pontos RGB em direções gaussianas aleatórias
- Quantiza as projeções em bins
- Usa múltiplas tabelas hash (L=10) para aumentar recall

#### **Parâmetros:**
```cpp
LSH_Search(int L = 10,      // Número de tabelas hash
           int k = 4,       // Projeções por tabela
           double w = 50.0) // Largura dos bins
```

#### **Características:**
- ✅ **Busca sublinear** - O(n^ρ) onde ρ < 1
- ✅ **Escalável** - Funciona bem em alta dimensão
- ✅ **Configurável** - Trade-off precisão vs velocidade
- ⚠️ **Aproximada** - Pode perder alguns resultados (menor recall)

#### **Funções Especiais:**
- `findSimilar()` - Busca padrão
- `findSimilarMultiprobe()` - Busca em buckets vizinhos (maior recall)
- `printStats()` - Estatísticas detalhadas (buckets, colisões, load factor)

#### **Referência:**
> Indyk & Motwani (1998) - "Approximate Nearest Neighbors: Towards Removing the Curse of Dimensionality"

---

### 2️⃣ **M-Tree (Metric Tree)**

**Arquivo**: `src/headers/mtree_search.h`

#### **Conceito:**
M-Tree é uma estrutura de árvore balanceada que funciona em **qualquer espaço métrico** (onde existe uma função de distância válida).

#### **Propriedades Métricas Utilizadas:**
1. **Não-negatividade**: d(x,y) ≥ 0
2. **Identidade**: d(x,y) = 0 ⟺ x = y
3. **Simetria**: d(x,y) = d(y,x)
4. **Desigualdade Triangular**: d(x,z) ≤ d(x,y) + d(y,z)

#### **Técnica de Poda:**
Usa a **desigualdade triangular** para podar subárvores:
```
Se |d(query, parent) - radius| > threshold, pode podar toda a subárvore
```

#### **Estrutura:**
- Cada nó tem um **routing object** (objeto representativo)
- Cada nó tem um **covering radius** (raio de cobertura)
- Split policy: **Balanced Split** (divide mais ou menos igualmente)
- Seeds para split: **Par de objetos mais distantes**

#### **Parâmetros:**
```cpp
MTree_Search(int capacity = 10)  // Capacidade máxima antes de split
```

#### **Características:**
- ✅ **Busca exata** - Garante encontrar todos os resultados
- ✅ **Métrica genérica** - Funciona com qualquer distância métrica
- ✅ **Poda eficiente** - Usa desigualdade triangular
- ⚠️ **Complexidade de implementação** - Balanceamento de árvore

#### **Funções Especiais:**
- `insert()` - Inserção com split automático
- `findSimilar()` - Range query com poda
- `printStats()` - Estatísticas (altura, nós folha, branching factor)

#### **Referência:**
> Ciaccia, Patella & Zezula (1997) - "M-tree: An Efficient Access Method for Similarity Search in Metric Spaces"

---

## 🚀 Como Compilar e Executar

### **Pré-requisitos:**
- ✅ GCC com suporte C++17 ou superior
- ✅ **OpenCV** (obrigatório para extração real de pixels)
- ✅ Sistema operacional: Linux, macOS ou Windows (MinGW/MSYS2)

### **Instalação do OpenCV:**

**Windows (MSYS2):**
```bash
pacman -S mingw-w64-x86_64-opencv
```

**Linux (Ubuntu/Debian):**
```bash
sudo apt install libopencv-dev
```

**macOS (Homebrew):**
```bash
brew install opencv
```

### **Compilação:**

```bash
# Navegar para a pasta do Trabalho 2
cd "Trabalho 2"

# Compilar com OpenCV
g++ -std=c++17 -O2 -o main src/main.cpp `pkg-config --cflags --libs opencv4`

# Executar
./main
```

### **Preparação:**
```bash
# Criar pastas necessárias
mkdir -p images query

# Copiar suas imagens para ./images/
# Copiar imagem de query para ./query/query.jpg (opcional)
```

### **Compilação com Warnings:**
```bash
g++ -std=c++17 -O2 -Wall -Wextra -o main src/main.cpp `pkg-config --cflags --libs opencv4`
```

### **Output Esperado:**
```
==================================================================================
 PAA TRABALHO 2 - Estruturas para Alta Dimensionalidade
 Comparação: Estruturas Básicas (T1) vs Estruturas Avançadas (T2)
==================================================================================

CONFIGURAÇÃO DO BENCHMARK:
  Query: RGB(128, 128, 128)
  Threshold: 40.0
  Escalas: 1000, 5000, 10000, 25000, 50000
  Dataset: Sintético com distribuição uniforme

[TESTANDO] Escala: 1000 imagens
Gerando dataset...
  Linear Search            : Insert=   0.50ms, Search=   0.02ms, Found=   15
  Hash Search              : Insert=   1.20ms, Search=   0.01ms, Found=   15
  ...
```

---

## 📊 Resultados Esperados

### **Trade-offs Observados:**

| Métrica | Melhor Estrutura | Justificativa |
|---------|------------------|---------------|
| **Inserção Rápida** | Linear Search | O(1) sem overhead |
| **Busca Rápida (aproximada)** | LSH / Hash Search | Sublinear / constante |
| **Busca Exata** | M-Tree | Poda eficiente com garantia |
| **Precisão (recall)** | Linear / M-Tree | Busca exata sem aproximações |
| **Escalabilidade** | LSH | Sublinear em alta dimensão |

### **Análise por Escala:**

**Pequenos datasets (< 5K):**
- Linear Search competitivo
- Overhead de estruturas complexas não compensa

**Médios datasets (5K-50K):**
- Hash Search domina busca
- LSH começa a mostrar vantagens

**Grandes datasets (> 50K):**
- LSH é o vencedor (busca aproximada)
- M-Tree melhor que árvores espaciais (Octree/Quadtree)

---

## 📁 Estrutura do Projeto

```
Trabalho 2/
├── Enunciado.txt                      # Enunciado original do trabalho
├── README.md                          # Esta documentação
├── src/
│   ├── main.cpp                       # Programa principal com benchmarks
│   ├── headers/
│   │   ├── image_database.h          # Interface base (comum)
│   │   ├── lsh_search.h              # 🆕 LSH implementation
│   │   └── mtree_search.h            # 🆕 M-Tree implementation
│   └── benchmarks/
│       └── (futuros benchmarks específicos)
├── resultados/
│   └── (resultados experimentais)
└── LaTex/
    └── (relatório JBCS - a ser criado)
```

---

## 🎓 Conceitos de PAA Demonstrados

### **Alta Dimensionalidade:**
1. **Curse of Dimensionality** - Degradação de estruturas espaciais
2. **Dimensionality Reduction** - Projeção de alta dimensão para baixa
3. **Approximate Search** - Trade-off precisão vs velocidade

### **Técnicas Avançadas:**
1. **Locality-Sensitive Hashing** - Hashing probabilístico
2. **Random Projections** - Projeções aleatórias gaussianas
3. **Metric Space Indexing** - Estruturas baseadas em propriedades métricas
4. **Triangle Inequality Pruning** - Poda usando desigualdade triangular

### **Trade-offs:**
1. **Exato vs Aproximado** - LSH (aproximado) vs M-Tree (exato)
2. **Tempo vs Espaço** - Múltiplas tabelas LSH vs árvore M-Tree
3. **Inserção vs Busca** - Otimização para diferentes operações

---

## 📝 Relatório JBCS (Trabalho Escrito)

O relatório acadêmico será elaborado seguindo o template JBCS, com até 4 páginas, contendo:

### **Estrutura Proposta:**

1. **Introdução** (0.5 página)
   - Contexto: Alta dimensionalidade em busca de similaridade
   - Objetivos: Comparar estruturas básicas vs avançadas
   - Contribuições: Implementação de LSH e M-Tree

2. **LSH - Locality-Sensitive Hashing** (1 página)
   - Fundamentação teórica
   - Detalhes de implementação
   - Análise de complexidade
   - Resultados experimentais

3. **M-Tree - Metric Tree** (1 página)
   - Fundamentação teórica
   - Estrutura e algoritmos
   - Split policy e poda
   - Resultados experimentais

4. **Comparação Experimental** (1 página)
   - Metodologia de benchmarking
   - Tabelas comparativas (7 estruturas)
   - Gráficos de performance
   - Análise de trade-offs

5. **Conclusões** (0.5 página)
   - Resumo dos resultados
   - Recomendações práticas
   - Trabalhos futuros

---

## 👥 Divisão de Responsabilidades

**Sugestão de divisão para grupo de 5 membros:**

1. **Membro 1-2**: Implementação e testes do LSH
2. **Membro 3-4**: Implementação e testes do M-Tree
3. **Membro 5**: Benchmarks, integração e análise comparativa
4. **Todos**: Escrita colaborativa do relatório JBCS

---

## 🔍 Experimentos Adicionais Sugeridos

### **1. Variação de Parâmetros LSH:**
```bash
# Testar diferentes configurações de L (tabelas) e k (projeções)
L=5, k=3, w=50    # Configuração rápida
L=10, k=4, w=50   # Configuração balanceada (padrão)
L=20, k=6, w=50   # Configuração precisa
```

### **2. Variação de Capacidade M-Tree:**
```bash
capacity=5    # Árvore mais profunda
capacity=10   # Balanceado (padrão)
capacity=20   # Árvore mais rasa
```

### **3. Datasets com Diferentes Distribuições:**
- Uniforme (atual)
- Clustered (pontos em grupos)
- Normal (distribuição gaussiana)
- Skewed (distribuição assimétrica)

---

## 📚 Referências

### **LSH:**
1. Indyk, P., & Motwani, R. (1998). Approximate nearest neighbors: towards removing the curse of dimensionality. *STOC*.
2. Datar, M., et al. (2004). Locality-sensitive hashing scheme based on p-stable distributions. *SCG*.

### **M-Tree:**
1. Ciaccia, P., Patella, M., & Zezula, P. (1997). M-tree: An efficient access method for similarity search in metric spaces. *VLDB*.
2. Zezula, P., et al. (2006). Similarity Search: The Metric Space Approach. *Springer*.

### **Geral:**
1. Böhm, C., et al. (2001). Searching in high-dimensional spaces: Index structures for improving the performance of multimedia databases. *ACM Computing Surveys*.

---

## 🎯 Conclusão

Este trabalho demonstra a **evolução de estruturas básicas para estruturas avançadas** na busca de similaridade:

- **Trabalho 1**: Fundamentos (Linear, Hash, Árvores espaciais)
- **Trabalho 2**: Alta dimensionalidade (LSH, M-Tree)

A implementação de **7 estruturas** em um único framework permite análise comparativa robusta e demonstra profundo entendimento de:
- Estruturas de dados espaciais
- Técnicas de hashing probabilístico
- Indexação métrica
- Trade-offs algorítmicos

---

*Projeto PAA - PUC Minas - 2025/2*
