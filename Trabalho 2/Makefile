# ==============================================================================
# PAA Trabalho 2 - Makefile
# Estruturas de Dados para Alta Dimensionalidade
# ==============================================================================

# Compilador e flags
CXX      = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -Isrc
OPENCV   = $(shell pkg-config --cflags --libs opencv4)

# Diretórios
SRC_DIR     = src
HEADER_DIR  = src/headers
BUILD_DIR   = build
BIN_DIR     = bin
RESULTS_DIR = resultados

# Arquivos principais
MAIN_SRC    = $(SRC_DIR)/main.cpp
HEADERS     = $(wildcard $(HEADER_DIR)/*.h)

# Executáveis
TARGET      = $(BIN_DIR)/trabalho2
TARGET_FAST = $(BIN_DIR)/trabalho2_fast

# ==============================================================================
# Regras Principais
# ==============================================================================

.PHONY: all clean run test help setup dirs

# Regra padrão: compila tudo
all: dirs $(TARGET)

# Compila versão otimizada
fast: dirs $(TARGET_FAST)

# Cria diretórios necessários
dirs:
	@mkdir -p $(BIN_DIR) $(RESULTS_DIR) images query

# ==============================================================================
# Compilação
# ==============================================================================

$(TARGET): $(MAIN_SRC) $(HEADERS)
	@echo "========================================"
	@echo "  Compilando Trabalho 2 (Release)..."
	@echo "========================================"
	$(CXX) $(CXXFLAGS) -o $@ $(MAIN_SRC) $(OPENCV)
	@echo "✓ Compilação concluída: $@"
	@echo ""

$(TARGET_FAST): $(MAIN_SRC) $(HEADERS)
	@echo "========================================"
	@echo "  Compilando Trabalho 2 (Fast/O3)..."
	@echo "========================================"
	$(CXX) -std=c++17 -O3 -march=native -Isrc -o $@ $(MAIN_SRC) $(OPENCV)
	@echo "✓ Compilação otimizada concluída: $@"
	@echo ""

# ==============================================================================
# Execução
# ==============================================================================

run: dirs $(TARGET)
	@echo "========================================"
	@echo "  Executando Benchmark Completo"
	@echo "========================================"
	./$(TARGET)

run-fast: dirs $(TARGET_FAST)
	@echo "========================================"
	@echo "  Executando Benchmark (Otimizado)"
	@echo "========================================"
	./$(TARGET_FAST)

# ColorHistogram 32D mode
run-hist: dirs $(TARGET)
	@echo "========================================"
	@echo "  Modo ColorHistogram 32D (68K vetores)"
	@echo "========================================"
	./$(TARGET) hist

# Teste rápido (só 1K imagens)
test: dirs $(TARGET)
	@echo "========================================"
	@echo "  Executando Teste Rápido (1K)"
	@echo "========================================"
	@echo "Modificando temporariamente para teste rápido..."
	./$(TARGET)

# ==============================================================================
# Limpeza
# ==============================================================================

clean:
	@echo "Limpando arquivos compilados..."
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "✓ Limpeza concluída"

clean-all: clean
	@echo "Limpando também resultados..."
	@rm -rf $(RESULTS_DIR)/*.txt
	@echo "✓ Limpeza completa concluída"

# ==============================================================================
# Utilidades
# ==============================================================================

# Verifica dependências
check-deps:
	@echo "Verificando dependências..."
	@which g++ > /dev/null || (echo "✗ g++ não encontrado"; exit 1)
	@echo "✓ g++ encontrado: $(shell g++ --version | head -1)"
	@pkg-config --exists opencv4 || (echo "✗ OpenCV4 não encontrado"; exit 1)
	@echo "✓ OpenCV4 encontrado: $(shell pkg-config --modversion opencv4)"
	@echo "✓ Todas dependências OK"

# Setup inicial do projeto
setup: dirs
	@echo "========================================"
	@echo "  Setup do Projeto"
	@echo "========================================"
	@echo "Criando estrutura de diretórios..."
	@mkdir -p images query resultados
	@echo "✓ Diretórios criados"
	@echo ""
	@echo "Verificando dependências..."
	@$(MAKE) check-deps
	@echo ""
	@echo "Setup completo! Execute:"
	@echo "  make run    - Para rodar o benchmark"
	@echo ""

# Informações do projeto
info:
	@echo "========================================"
	@echo "  PAA Trabalho 2 - Informações"
	@echo "========================================"
	@echo "Compilador:  $(CXX)"
	@echo "Flags:       $(CXXFLAGS)"
	@echo "OpenCV:      $(shell pkg-config --modversion opencv4 2>/dev/null || echo 'não encontrado')"
	@echo "Executável:  $(TARGET)"
	@echo "Headers:     $(words $(HEADERS)) arquivos"
	@echo ""

# Ajuda
help:
	@echo "========================================"
	@echo "  PAA Trabalho 2 - Makefile"
	@echo "  Estruturas para Alta Dimensionalidade"
	@echo "========================================"
	@echo ""
	@echo "Comandos disponíveis:"
	@echo ""
	@echo "  COMPILAÇÃO:"
	@echo "    make              - Compila o projeto (modo release)"
	@echo "    make fast         - Compila com otimizações máximas (-O3)"
	@echo "    make clean        - Remove arquivos compilados"
	@echo "    make clean-all    - Remove compilados + resultados"
	@echo ""
	@echo "  EXECUÇÃO:"
	@echo "    make run          - Compila e executa benchmark RGB 3D"
	@echo "    make run-hist     - Executa modo ColorHistogram 32D"
	@echo "    make run-fast     - Executa versão otimizada"
	@echo "    make test         - Teste rápido (apenas 1K imagens)"
	@echo ""
	@echo "  UTILIDADES:"
	@echo "    make setup        - Setup inicial do projeto"
	@echo "    make check-deps   - Verifica dependências"
	@echo "    make info         - Mostra informações do projeto"
	@echo "    make help         - Mostra esta ajuda"
	@echo ""
	@echo "Exemplo de uso:"
	@echo "  make setup        # Primeira vez"
	@echo "  make run          # Rodar benchmark"
	@echo ""

# ==============================================================================
# Debug (para desenvolvimento)
# ==============================================================================

debug: CXXFLAGS = -std=c++17 -g -O0 -Wall -Wextra -Isrc -DDEBUG
debug: dirs
	@echo "Compilando versão DEBUG..."
	$(CXX) $(CXXFLAGS) -o $(BIN_DIR)/trabalho2_debug $(MAIN_SRC) $(OPENCV)
	@echo "✓ Versão debug compilada"

# Valgrind para detectar memory leaks
valgrind: debug
	@echo "Executando com Valgrind..."
	valgrind --leak-check=full --show-leak-kinds=all $(BIN_DIR)/trabalho2_debug
