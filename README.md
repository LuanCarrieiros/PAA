# Projeto e Análise de Algoritmos

**Instituição**: PUC Minas - Ciência da Computação  
**Campus**: Coração Eucarístico  
**Turno**: Manhã  
**Período**: 2025/2

Este repositório contém os trabalhos desenvolvidos na disciplina de **Projeto e Análise de Algoritmos**, organizados por módulos associados aos dias de aula conforme cronograma disponível no Canvas.

## 📁 Estrutura do Repositório

A disciplina contempla **3 trabalhos práticos** ao longo do semestre. Este repositório será atualizado conforme o desenvolvimento de cada trabalho.

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

### [`Trabalho 2/`](./Trabalho%202/) - Estruturas de Dados para Alta Dimensionalidade

Implementação e comparação de estruturas avançadas para busca de similaridade em alta dimensionalidade:

**Estruturas Implementadas:**
- **LSH (Locality-Sensitive Hashing)** 🆕 - Busca aproximada sublinear com projeções aleatórias
- **M-Tree (Metric Tree)** 🆕 - Busca exata em espaços métricos com poda triangular

**Comparação Completa:**
- 🔄 Integra com as 5 estruturas do Trabalho 1
- 📊 Compara 7 estruturas em um único framework
- 🎯 Analisa trade-offs: Busca exata vs aproximada
- ⚡ Avalia impacto da curse of dimensionality

**Destaques do trabalho:**
- ✅ LSH com múltiplas tabelas hash (L=10, k=4)
- ✅ M-Tree com split balanceado e poda eficiente
- ✅ Benchmarks sintéticos de 1K → 50K imagens
- ✅ Análise teórica e prática de complexidade
- ✅ Documentação completa com guias de compilação
- 📊 **Objetivo**: Relatório JBCS comparativo (até 4 páginas)

### `Trabalho 3/` - *Planejado*
Será disponibilizado em breve...

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

*Este repositório está em desenvolvimento ativo. Os Trabalhos 2 e 3 serão adicionados conforme o progresso da disciplina.*