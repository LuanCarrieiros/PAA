# ============================================================================
# Build Script para Trabalho 2 (PowerShell)
# Equivalente ao Makefile para Windows
# ============================================================================

param(
    [string]$Command = "help"
)

$SRC_DIR = "src"
$HEADER_DIR = "src/headers"
$BIN_DIR = "bin"
$RESULTS_DIR = "resultados"
$MAIN_SRC = "src/main.cpp"
$TARGET = "bin/trabalho2.exe"

# Cores para output
function Write-Success { Write-Host $args -ForegroundColor Green }
function Write-Info { Write-Host $args -ForegroundColor Cyan }
function Write-Error { Write-Host $args -ForegroundColor Red }
function Write-Warning { Write-Host $args -ForegroundColor Yellow }

# ============================================================================
# Funções
# ============================================================================

function Create-Directories {
    Write-Info "Criando estrutura de diretórios..."
    New-Item -ItemType Directory -Force -Path $BIN_DIR | Out-Null
    New-Item -ItemType Directory -Force -Path $RESULTS_DIR | Out-Null
    New-Item -ItemType Directory -Force -Path "images" | Out-Null
    New-Item -ItemType Directory -Force -Path "query" | Out-Null
    Write-Success "✓ Diretórios criados"
}

function Check-Dependencies {
    Write-Info "`nVerificando dependências..."

    # Verifica g++
    $gpp = Get-Command g++ -ErrorAction SilentlyContinue
    if ($null -eq $gpp) {
        Write-Error "✗ g++ não encontrado!"
        Write-Warning "  Instale MinGW ou MSYS2: https://www.msys2.org/"
        return $false
    }
    Write-Success "✓ g++ encontrado: $($gpp.Source)"

    # Verifica OpenCV
    $opencv = & pkg-config --modversion opencv4 2>$null
    if ($LASTEXITCODE -ne 0) {
        Write-Warning "⚠ OpenCV4 não encontrado via pkg-config"
        Write-Warning "  Tentando compilar sem pkg-config..."
    } else {
        Write-Success "✓ OpenCV4 encontrado: $opencv"
    }

    return $true
}

function Build-Project {
    param([string]$OptLevel = "-O2", [string]$OutputName = $TARGET)

    Write-Info "`n========================================"
    Write-Info "  Compilando Trabalho 2..."
    Write-Info "========================================"

    Create-Directories

    # Comando de compilação
    $includes = "-Isrc"
    $flags = "-std=c++17 $OptLevel -Wall -Wextra"

    # Tenta com pkg-config primeiro
    $opencv_flags = & pkg-config --cflags --libs opencv4 2>$null

    if ($LASTEXITCODE -eq 0) {
        Write-Info "Compilando com OpenCV (pkg-config)..."
        $cmd = "g++ $flags $includes -o `"$OutputName`" `"$MAIN_SRC`" $opencv_flags"
    } else {
        # Fallback: tenta caminhos padrão do MSYS2
        Write-Warning "pkg-config não disponível, usando caminhos padrão..."
        $opencv_includes = "-IC:/msys64/mingw64/include/opencv4"
        $opencv_libs = "-LC:/msys64/mingw64/lib -lopencv_core -lopencv_imgcodecs -lopencv_imgproc"
        $cmd = "g++ $flags $includes $opencv_includes -o `"$OutputName`" `"$MAIN_SRC`" $opencv_libs"
    }

    Write-Info "Executando: $cmd`n"

    # Executa compilação
    Invoke-Expression $cmd

    if ($LASTEXITCODE -eq 0) {
        Write-Success "`n✓ Compilação concluída: $OutputName"
        return $true
    } else {
        Write-Error "`n✗ Erro na compilação!"
        return $false
    }
}

function Run-Program {
    if (-not (Test-Path $TARGET)) {
        Write-Warning "Executável não encontrado. Compilando primeiro..."
        if (-not (Build-Project)) {
            return
        }
    }

    Write-Info "`n========================================"
    Write-Info "  Executando Benchmark"
    Write-Info "========================================"

    & $TARGET
}

function Clean-Build {
    Write-Info "Limpando arquivos compilados..."
    if (Test-Path $BIN_DIR) {
        Remove-Item -Recurse -Force $BIN_DIR
    }
    Write-Success "✓ Limpeza concluída"
}

function Clean-All {
    Clean-Build
    Write-Info "Limpando também resultados..."
    if (Test-Path "$RESULTS_DIR/*.txt") {
        Remove-Item -Force "$RESULTS_DIR/*.txt"
    }
    Write-Success "✓ Limpeza completa concluída"
}

function Show-Help {
    Write-Host @"

========================================
  PAA Trabalho 2 - Build Script
  Estruturas para Alta Dimensionalidade
========================================

USO: .\build.ps1 [comando]

COMANDOS DISPONÍVEIS:

  COMPILAÇÃO:
    build         - Compila o projeto (modo release -O2)
    fast          - Compila com otimizações máximas (-O3)
    debug         - Compila em modo debug (-g -O0)
    clean         - Remove arquivos compilados
    clean-all     - Remove compilados + resultados

  EXECUÇÃO:
    run           - Compila e executa benchmark completo
    test          - Teste rápido

  UTILIDADES:
    setup         - Setup inicial do projeto
    check-deps    - Verifica dependências
    info          - Mostra informações do projeto
    help          - Mostra esta ajuda

EXEMPLOS:
  .\build.ps1 setup          # Primeira vez
  .\build.ps1 run            # Compilar e executar
  .\build.ps1 clean          # Limpar

ALTERNATIVAS:
  Se preferir usar Makefile, instale:
  - MinGW/MSYS2: https://www.msys2.org/
  - Ou use WSL: wsl -d Ubuntu

========================================

"@
}

function Show-Info {
    Write-Info "`n========================================"
    Write-Info "  PAA Trabalho 2 - Informações"
    Write-Info "========================================`n"

    $gpp_version = & g++ --version 2>$null | Select-Object -First 1
    $opencv_version = & pkg-config --modversion opencv4 2>$null

    Write-Host "Compilador:  " -NoNewline
    if ($gpp_version) { Write-Success $gpp_version } else { Write-Error "não encontrado" }

    Write-Host "OpenCV:      " -NoNewline
    if ($opencv_version) { Write-Success $opencv_version } else { Write-Warning "não encontrado" }

    Write-Host "Executável:  $TARGET"

    $headers = (Get-ChildItem -Path $HEADER_DIR -Filter "*.h" -ErrorAction SilentlyContinue).Count
    Write-Host "Headers:     $headers arquivos"
    Write-Host ""
}

function Setup-Project {
    Write-Info "`n========================================"
    Write-Info "  Setup do Projeto"
    Write-Info "========================================`n"

    Create-Directories

    if (Check-Dependencies) {
        Write-Success "`n✓ Setup completo!"
        Write-Info "`nPróximos passos:"
        Write-Info "  .\build.ps1 run    - Para rodar o benchmark"
    } else {
        Write-Error "`n✗ Dependências faltando!"
        Write-Info "Instale MinGW/MSYS2: https://www.msys2.org/"
    }
}

# ============================================================================
# Main
# ============================================================================

switch ($Command.ToLower()) {
    "build" {
        Build-Project
    }
    "fast" {
        Build-Project -OptLevel "-O3" -OutputName "bin/trabalho2_fast.exe"
    }
    "debug" {
        Build-Project -OptLevel "-g -O0" -OutputName "bin/trabalho2_debug.exe"
    }
    "run" {
        if (Build-Project) {
            Run-Program
        }
    }
    "test" {
        Write-Info "Executando teste rápido..."
        if (Build-Project) {
            Run-Program
        }
    }
    "clean" {
        Clean-Build
    }
    "clean-all" {
        Clean-All
    }
    "setup" {
        Setup-Project
    }
    "check-deps" {
        Check-Dependencies
    }
    "info" {
        Show-Info
    }
    "help" {
        Show-Help
    }
    default {
        Write-Warning "Comando desconhecido: $Command"
        Show-Help
    }
}
