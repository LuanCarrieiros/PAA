# Como Compilar e Executar - PAA Trabalho 1

## Estrutura do Projeto

```
PAA/
├── README.md                                    # Visão geral do projeto
├── src/
│   ├── main.cpp                                # Código educativo principal
│   ├── headers/                                # Bibliotecas desenvolvidas
│   │   ├── hash_search.h
│   │   ├── linear_search.h
│   │   ├── octree_search.h
│   │   ├── octree_iterative.h
│   │   ├── quadtree.h
│   │   └── stb_image.h                         # Para processamento de imagens
│   └── benchmarks/                             # Experimentos
│       ├── scalable_benchmark.cpp              # Benchmark principal (100→50M)
│       ├── benchmark_recursivo_vs_iterativo.cpp
│       ├── benchmark_100M_only.cpp
│       ├── benchmark_imagens_simples.cpp       # Imagens reais
│       └── benchmark_imagens_locais.cpp
├── resultados/                                 # Resultados experimentais
│   ├── resultados50Mseed42.txt
│   └── resultadosOctaQuad.txt
└── docs/                                       # Documentação
    └── ANALISE_DETALHADA_EXPERIMENTOS.md
```

## Comandos de Compilação

### Código Principal (Imagens Reais)
```bash
g++ -std=c++17 -O2 -o main src/main.cpp
./main
# Processa imagens reais com extração RGB baseada em arquivo
# Escalas: 10K, 25K, 50K, 100K, 150K, 206K imagens
```

### Benchmark Sintético Principal (100 → 50M imagens)
```bash
g++ -O2 -o scalable src/benchmarks/scalable_benchmark.cpp
./scalable
# AVISO: Pode levar 30+ minutos para 50M imagens
```

### Comparação Recursão vs Iteração
```bash
g++ -O2 -o rec_iter src/benchmarks/benchmark_recursivo_vs_iterativo.cpp
./rec_iter
```

### Benchmark Específico 100M (Experimental)
```bash
g++ -O2 -o benchmark_100m src/benchmarks/benchmark_100M_only.cpp
./benchmark_100m
# AVISO: Requer ~12GB RAM (com 16GB após 2 horas não consegui)
```

## Principais Descobertas (Dataset Real - 206,395 imagens)

- **Hash Search**: Domina busca (0.791ms para 206K imagens!)
- **Linear Search**: Domina inserção (20.487ms - zero overhead)
- **Quadtree**: Encontra 4.26× mais imagens (12,993 vs 3,051)
- **Octree**: Performance degradada (23.145ms vs 0.791ms - 29.3× mais lenta)
- **Trade-off Crítico**: Precisão vs Velocidade - Hash rápida e precisa, Quadtree lenta mas encontra mais resultados
- **Query Fixa**: RGB(66,35,226) com threshold 40.0

## Documentação Completa

- **README.md**: Visão geral e resultados principais
- **docs/ANALISE_DETALHADA_EXPERIMENTOS.md**: Análise técnica completa
- **resultados/**: Dados brutos dos experimentos

## 🎯 Resultados Resumidos

| Estrutura | Melhor Para | Tempo (206K) | Encontradas |
|-----------|-------------|--------------|-------------|
| **Hash Search** | Busca rápida | 0.791ms | 3,051 |
| **Linear Search** | Inserção | 20.487ms inserir | 3,051 |
| **Quadtree** | Maior recall | 3.249ms | 12,993 |
| **Octree** | ❌ Evitar | 23.145ms | 3,051 |
| **Hash Dynamic** | Balanceado | 1.650ms | 5,970 |

---
*Projeto PAA - PUC Minas - 2025/2*