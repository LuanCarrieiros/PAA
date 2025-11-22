# Como Compilar e Executar - Trabalho 2 (com OpenCV)

## 📋 Pré-requisitos

### **1. Compilador C++17**
- GCC 7.0+ (Linux/macOS)
- MinGW/MSYS2 (Windows)
- MSVC 2017+ (Windows)

### **2. OpenCV (OBRIGATÓRIO)**
O projeto usa OpenCV para extrair RGB **REAL** dos pixels das imagens.

---

## 🔧 Instalação do OpenCV

### **Windows (MSYS2/MinGW - RECOMENDADO)**

#### **Instalar MSYS2:**
1. Baixe o instalador: https://www.msys2.org/
2. Execute o instalador
3. Abra o terminal MSYS2 MinGW 64-bit

#### **Instalar OpenCV:**
```bash
# Atualizar pacotes
pacman -Syu

# Instalar OpenCV
pacman -S mingw-w64-x86_64-opencv

# Verificar instalação
pkg-config --modversion opencv4
```

---

### **Linux (Ubuntu/Debian)**

```bash
# Atualizar repositórios
sudo apt update

# Instalar OpenCV
sudo apt install libopencv-dev

# Verificar instalação
pkg-config --modversion opencv4
```

---

### **macOS (Homebrew)**

```bash
# Instalar OpenCV
brew install opencv

# Verificar instalação
pkg-config --modversion opencv4
```

---

## 🚀 Compilação

### **Compilação Rápida (com pkg-config):**

```bash
cd "Trabalho 2"

# Compilar com OpenCV
g++ -std=c++17 -O2 -o main src/main.cpp `pkg-config --cflags --libs opencv4`
```

### **Compilação Manual (se pkg-config não funcionar):**

#### **Windows (MSYS2):**
```bash
g++ -std=c++17 -O2 -o main src/main.cpp \
    -I/mingw64/include/opencv4 \
    -L/mingw64/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc
```

#### **Linux:**
```bash
g++ -std=c++17 -O2 -o main src/main.cpp \
    -I/usr/include/opencv4 \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc
```

#### **macOS:**
```bash
g++ -std=c++17 -O2 -o main src/main.cpp \
    -I/opt/homebrew/include/opencv4 \
    -L/opt/homebrew/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc
```

---

## ▶️ Execução

### **1. Preparar estrutura de pastas:**
```bash
cd "Trabalho 2"
mkdir -p images query
```

### **2. Adicionar imagens:**
```bash
# Copie suas imagens para a pasta images/
# Formatos suportados: .jpg, .jpeg, .png, .bmp, .tiff, .tif
```

### **3. Adicionar query (opcional):**
```bash
# Copie uma imagem de query para:
# ./query/query.jpg
```

### **4. Executar:**
```bash
./main
```

---

## 📊 Output Esperado

```
==================================================================================
 PAA TRABALHO 2 - Estruturas para Alta Dimensionalidade
 Comparação: Estruturas Básicas (T1) vs Estruturas Avançadas (T2)
 DATASET: https://www.kaggle.com/datasets/alessiocorrado99/animals10/data?select=raw-img
==================================================================================

Auto-detectando imagens em: ./images/
Auto-deteccao concluida: 206395 imagens encontradas
Query REAL carregada: ./query/query.jpg
RGB REAL extraido: (123.4, 98.7, 215.6)

CONFIGURAÇÃO DO BENCHMARK:
  Dataset: ./images/ (206395 imagens REAIS)
  Query: RGB(123, 99, 216)
  Threshold: 40.0
  Escalas: 1000, 5000, 10000, 25000, 50000, 206395
  Fonte RGB: Extração REAL dos PIXELS usando OpenCV

[TESTANDO] Escala: 1000 imagens REAIS
Carregando dataset...
Processadas 1000 imagens reais...
Dataset REAL carregado: 1000 imagens de ./images/
  Linear Search            : Insert=   X.XXms, Search=   X.XXms, Found=  XXX
  Hash Search              : Insert=   X.XXms, Search=   X.XXms, Found=  XXX
  ...
```

---

## ⚙️ Troubleshooting

### **Erro: "opencv2/opencv.hpp: No such file or directory"**

**Solução**: OpenCV não está instalado ou não está no path.

**Windows (MSYS2):**
```bash
pacman -S mingw-w64-x86_64-opencv
```

**Linux:**
```bash
sudo apt install libopencv-dev
```

**macOS:**
```bash
brew install opencv
```

---

### **Erro: "undefined reference to 'cv::imread'"**

**Solução**: Faltam as bibliotecas do OpenCV no link.

**Compile com:**
```bash
g++ -std=c++17 -O2 -o main src/main.cpp `pkg-config --cflags --libs opencv4`
```

Ou manualmente:
```bash
g++ -std=c++17 -O2 -o main src/main.cpp \
    -I/mingw64/include/opencv4 \
    -L/mingw64/lib \
    -lopencv_core -lopencv_imgcodecs -lopencv_imgproc
```

---

### **Erro: "Package opencv4 was not found"**

**Solução**: Tente opencv (sem o 4):
```bash
g++ -std=c++17 -O2 -o main src/main.cpp `pkg-config --cflags --libs opencv`
```

---

### **Erro: Headers do Trabalho 1 não encontrados**

**Solução**: Certifique-se que a estrutura de pastas está correta:
```
PAA/
├── Trabalho 1/
│   └── src/headers/
│       ├── linear_search.h
│       ├── hash_search.h
│       ├── octree_search.h
│       └── quadtree.h
└── Trabalho 2/
    └── src/
        └── main.cpp
```

---

## 🎯 Verificar se OpenCV está funcionando

### **Teste rápido:**
```bash
# Criar arquivo de teste
cat > test_opencv.cpp << 'EOF'
#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    std::cout << "OpenCV Version: " << CV_VERSION << std::endl;

    cv::Mat img = cv::imread("test.jpg");
    if (img.empty()) {
        std::cout << "Nenhuma imagem carregada (normal se test.jpg não existir)" << std::endl;
    } else {
        std::cout << "Imagem carregada com sucesso!" << std::endl;
        cv::Scalar avg = cv::mean(img);
        std::cout << "RGB médio: (" << avg[2] << ", " << avg[1] << ", " << avg[0] << ")" << std::endl;
    }

    return 0;
}
EOF

# Compilar e executar
g++ -o test_opencv test_opencv.cpp `pkg-config --cflags --libs opencv4`
./test_opencv
```

---

## 📝 Notas Importantes

### **Extração RGB Real:**
O código agora usa `cv::mean()` para calcular a **cor média** de todos os pixels:
- ✅ Lê TODOS os pixels da imagem
- ✅ Calcula a média dos canais R, G, B
- ✅ Representa a cor dominante da imagem
- ✅ 100% baseado em dados reais, não simulação!

### **Performance:**
- A extração com OpenCV é **mais lenta** que hash (lê todos os pixels)
- Para 200K imagens, pode levar alguns minutos
- Mas os resultados são **100% reais**! 🎯

### **Alternativas de Extração:**
Se quiser outros métodos além da média:

```cpp
// Cor dominante (histograma)
// Pixel central
// Mediana das cores
// Etc.
```

---

## 🔍 Exemplo Completo de Compilação

### **Windows (MSYS2 MinGW 64-bit):**
```bash
# 1. Abrir MSYS2 MinGW 64-bit terminal
# 2. Navegar até a pasta
cd /c/Users/yluan/Documents/GitHub/PAA/"Trabalho 2"

# 3. Compilar
g++ -std=c++17 -O2 -o main.exe src/main.cpp `pkg-config --cflags --libs opencv4`

# 4. Executar
./main.exe
```

### **Linux:**
```bash
cd ~/Documents/GitHub/PAA/"Trabalho 2"
g++ -std=c++17 -O2 -o main src/main.cpp `pkg-config --cflags --libs opencv4`
./main
```

### **macOS:**
```bash
cd ~/Documents/GitHub/PAA/"Trabalho 2"
g++ -std=c++17 -O2 -o main src/main.cpp `pkg-config --cflags --libs opencv4`
./main
```

---

## ✅ Checklist Antes de Executar

- [ ] OpenCV instalado (`pkg-config --modversion opencv4`)
- [ ] Pasta `./images/` criada
- [ ] Imagens copiadas para `./images/`
- [ ] (Opcional) `./query/query.jpg` criada
- [ ] Código compilado sem erros
- [ ] Executável criado (`main` ou `main.exe`)

---

*Projeto PAA - PUC Minas - 2025/2 - Agora com extração REAL de pixels usando OpenCV!* 🎨
