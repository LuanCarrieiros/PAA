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

### [`Trabalho 2/`](./Trabalho%202/) - Estruturas para Alta Dimensionalidade (**TRABALHO FINAL**)

Análise comparativa de estruturas de busca por similaridade em **dois contextos dimensionais**:

**Experimento Principal - ColorHistogram 32D:**
- 📊 Dataset: **68.040 vetores** de histogramas de cores (32 dimensões) - [UCI Corel Image Features](https://archive.ics.uci.edu/dataset/119/corel+image+features)
- 🎯 Estruturas testadas: LinearSearch, LSH, M-Tree
- ✅ **LSH vence**: 2.9× mais rápido em buscas (50K escala)
- ✅ Valida design de LSH para alta dimensionalidade real

**Experimento Validação - RGB 3D:**
- 📊 Dataset: **26.179 imagens** Animals-10 (Kaggle)
- 🎯 Total: **7 estruturas** (Trabalho 1 + LSH + M-Tree)
- ✅ **Octree/Quadtree vencem**: Estruturas clássicas dominam em 3D
- ✅ Demonstra overhead de LSH em baixa dimensão

**Descoberta - Bidirectional Curse of Dimensionality:**
- ⚡ LSH **excelente em 32D**, **ruim em 3D**
- ⚡ Estruturas espaciais (Octree/Quadtree) **ótimas em 3D**, **não funcionam em 32D**
- 📊 Conclusão: **Matching dimensão-algoritmo é crítico**

**Resultados ColorHistogram 32D (50K vetores):**
- 🥇 **Busca**: LSH (1.67ms) vs LinearSearch (4.83ms) - **2.9× speedup**
- 📈 **Inserção**: LinearSearch (15.2ms) vs LSH (88.7ms)
- ✅ **Precisão**: LSH ~95% vs Linear 100%

**Resultados RGB 3D (26K imagens):**
- 🥇 **Busca**: Octree (2.12ms) vs LSH (4.62ms) - estruturas clássicas dominam
- 🥇 **Inserção**: Linear (1.00ms) - sem overhead
- ❌ **LSH em 3D**: 53× mais lento na inserção - não recomendado

**Destaques Técnicos:**
- ✅ Dois modos no mesmo programa: `./trabalho2` (RGB) e `./trabalho2 hist` (32D)
- ✅ LSH com 10 tabelas hash (L=10, k=8 para 32D / k=4 para 3D)
- ✅ M-Tree com poda por desigualdade triangular
- ✅ Query interativa (RGB) e fixa (ColorHistogram)
- ✅ Resultados com timestamp único
- ✅ Makefile profissional: `make run` / `make run-hist`
- 📄 Documentação LaTeX (formato JBCS)

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