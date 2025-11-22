# Projeto e Análise de Algoritmos

**Instituição**: PUC Minas - Ciência da Computação  
**Campus**: Coração Eucarístico  
**Turno**: Manhã  
**Período**: 2025/2

Este repositório contém os trabalhos desenvolvidos na disciplina de **Projeto e Análise de Algoritmos**, organizados por módulos associados aos dias de aula conforme cronograma disponível no Canvas.

## 📁 Estrutura do Repositório

A disciplina contempla **2 trabalhos práticos principais** ao longo do semestre, focados em estruturas de dados e análise de algoritmos.

### [`Trabalho 1/`](./Trabalho%201/) - Análise Comparativa de Estruturas de Dados para Busca de Similaridade de Imagens RGB

Estudo empírico comparativo de 5 estruturas de dados para busca de similaridade em imagens RGB:

- **Linear Search** (baseline brute force)
- **Hash Search** (hashing espacial 3D com grid)  
- **Hash Dynamic Search** (expansão adaptativa)
- **Octree Search** (árvore espacial 3D recursiva)
- **Quadtree Search** (árvore espacial 2D)

**Destaques do trabalho:**
- ✅ Implementação em C++17 com otimizações
- ✅ Benchmarks de 10K → 206K imagens reais  
- ✅ Dataset real com extração RGB baseada em arquivo
- ✅ Análise de trade-offs precisão vs velocidade
- ✅ Documentação LaTeX completa (4 páginas)
- 📊 **Resultados**: Hash Search domina buscas (0.791ms), Linear Search domina inserções (20.487ms)

### [`Trabalho 2/`](./Trabalho%202/) - Estruturas de Dados para Alta Dimensionalidade (**TRABALHO FINAL**)

Implementação e análise comparativa de estruturas avançadas para busca de similaridade em alta dimensionalidade, com foco em avaliar a eficácia de estruturas modernas versus clássicas em espaços RGB 3D.

**Estruturas Novas Implementadas:**
- **LSH (Locality-Sensitive Hashing)** 🆕 - Busca aproximada sublinear com random projections
- **M-Tree (Metric Tree)** 🆕 - Busca exata em espaços métricos com poda triangular

**Comparação Completa:**
- 🔄 Integra as 5 estruturas do Trabalho 1 + 2 novas (total: **7 estruturas**)
- 📊 Dataset real: **26.179 imagens** do [Animals-10 Kaggle](https://www.kaggle.com/datasets/alessiocorrado99/animals10)
- 🎯 Análise de trade-offs: Busca exata vs aproximada
- ⚡ Descoberta: **Curse of Dimensionality funciona "ao contrário"** em RGB 3D

**Resultados Principais (26K imagens):**
- 🥇 **Melhor Inserção**: Linear Search (1.02ms)
- 🥇 **Melhor Busca**: Hash Dynamic Search (2.47ms) e Quadtree (2.25ms)
- 📉 **LSH**: Overhead alto em 3D (58.19ms insert) - não vale a pena para RGB
- ⚖️ **M-Tree**: Competitivo em pequena escala (<5K), degrada com dataset grande

**Destaques do trabalho:**
- ✅ Implementação completa de LSH com múltiplas tabelas (L=10, k=4, w=50)
- ✅ M-Tree com split balanceado e poda por desigualdade triangular
- ✅ Extração RGB real via OpenCV de 26K+ imagens
- ✅ Benchmarks em escalas: 1K, 5K, 10K, 25K, 26K imagens
- ✅ Query real extraída de imagem (RGB: 66, 35, 226)
- ✅ Documentação LaTeX completa (formato JBCS, até 4 páginas)
- 📊 **Conclusão**: Estruturas básicas (Hash, Quadtree) dominam em RGB 3D; LSH/M-Tree são over-engineering

---

## 🚀 Como usar cada trabalho

Cada trabalho possui instruções específicas de compilação e execução em sua respectiva pasta. Consulte o README individual de cada trabalho.

## Conteúdo da Disciplina

A disciplina aborda os seguintes tópicos principais:

- **Visão geral do semestre e alguns conceitos**
- **Revisão de grafos**
- **Custo computacional e ordens de complexidade**
- **Tratabilidade**
- **Algoritmos Gulosos**
- **Divisão e conquista**
- **Programação dinâmica**
- **Outras estratégias de projeto de algoritmos**

## ⚙️ Tecnologias Utilizadas

- **C++17** - Implementação dos algoritmos
- **LaTeX** - Documentação acadêmica  
- **Git** - Controle de versão
- **GCC** - Compilação com otimizações

---

## 📊 Visão Geral dos Resultados

| Trabalho | Dataset | Estruturas | Melhor Inserção | Melhor Busca | Descoberta Principal |
|----------|---------|------------|-----------------|--------------|---------------------|
| **T1** | 206K imagens | 5 básicas | Linear (20ms) | Hash (0.79ms) | Hash domina em datasets grandes |
| **T2** | 26K imagens | 7 (5+2 novas) | Linear (1.02ms) | Quadtree (2.25ms) | LSH/M-Tree têm overhead em 3D |

---

*Este repositório contém os trabalhos completos da disciplina de Projeto e Análise de Algoritmos (2025/2).*