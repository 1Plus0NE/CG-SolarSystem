# Engine - Modular Architecture 

## Overview

O código do `engine` foi reorganizado em módulos independentes para melhorar a manutenibilidade, legibilidade e escalabilidade. Em vez de ter um arquivo monolítico de 531 linhas, o código agora está dividido em módulos temáticos bem definidos.

## Estrutura Modular

### Core (Núcleo)

#### [geometry.h](geometry.h)
**Responsabilidade:** Definições de tipos e estruturas de dados principais

- `Vertex`: Estrutura para vértices 3D (x, y, z)
- `Transform`: Transformações (translate, rotate, scale)
- `Model`: Modelos 3D com vértices e cores
- `Group`: Grafo de cena com transformações, modelos e sub-grupos
- `Star`: Estrutura para estrelas do fundo
- `Camera`: Câmara com posição, orientação e projeção

**Tamanho:** ~60 linhas

### Model Management

#### [model.h](model.h) / [model.cpp](model.cpp)
**Responsabilidade:** Carregamento e cache de modelos 3D

**Funções principais:**
- `loadModelFile()`: Carrega modelos do arquivo .3d
- `getModelVertices()`: Obtém vértices com cache automático
- `clearModelCache()`: Limpa o cache

**Tamanho:** ~50 linhas

### Rendering

#### [rendering.h](rendering.h) / [rendering.cpp](rendering.cpp)
**Responsabilidade:** Renderização de cenas, skybox de estrelas e interface

**Funções principais:**
- `generateStars()`: Gera skybox procedural
- `renderStars()`: Renderiza o campo de estrelas
- `renderGroup()`: Renderiza grupos de forma recursiva
- `renderScene()`: Loop principal de renderização
- `changeSize()`: Redimensionamento da janela
- `updateFPS()`: Contador de FPS

**Tamanho:** ~180 linhas

### Input Processing

#### [input.h](input.h) / [input.cpp](input.cpp)
**Responsabilidade:** Processamento de entrada do usuário

**Funções principais:**
- `processKeys()`: Entrada do teclado (navegação, zoom, reload config)
- `processMouseButtons()`: Cliques de mouse (zoom com scroll)
- `processMouseMotion()`: Movimento do mouse (rotação da câmara)

**Tamanho:** ~60 linhas

### Configuration

#### [config.h](config.h) / [config.cpp](config.cpp)
**Responsabilidade:** Carregamento de configuração XML e parsing

**Funções principais:**
- `parseHexColor()`: Converte cores hex para RGB [0,1]
- `parseGroup()`: Parser recursivo de grupos XML
- `loadConfigs()`: Carrega arquivo XML completo
- `reloadConfig()`: Recarrega configuração em tempo de execução

**Tamanho:** ~175 linhas

### Main Application

#### [engine.cpp](engine.cpp)
**Responsabilidade:** Ponto de entrada da aplicação

**Conteúdo:**
- Declaração de variáveis globais
- Função `main()` com inicialização GLUT
- Setup de callbacks de renderização e input

**Tamanho:** ~120 linhas (reduzido de 531!)

## Vantagens da Reorganização

✅ **Separação de Responsabilidades**
- Cada módulo tem uma responsabilidade clara e única

✅ **Manutenibilidade**
- Código mais fácil de encontrar e modificar
- Redução de complexidade por arquivo

✅ **Reutilização**
- Módulos podem ser facilmente reutilizados em outros projetos
- Headers claramente definem interfaces públicas

✅ **Testabilidade**
- Cada módulo pode ser testado independentemente
- Dependências são explícitas

✅ **Escalabilidade**
- Fácil adicionar novos módulos sem afetar existentes
- Estrutura suporta crescimento futuro

## Fluxo de Execução

```
main() (engine.cpp)
  ├─ loadConfigs() (config.cpp)
  │  ├─ parseHexColor()
  │  ├─ parseGroup()
  │  └─ getModelVertices() (model.cpp)
  │
  ├─ glutInit() & glutCreateWindow()
  │
  ├─ generateStars() (rendering.cpp)
  │
  └─ glutMainLoop()
     ├─ renderScene() → rendering.cpp
     │  ├─ renderStars()
     │  └─ renderGroup()
     │
     ├─ processKeys() → input.cpp
     ├─ processMouseButtons() → input.cpp
     └─ processMouseMotion() → input.cpp
```

## Próximos Passos

1. **Menu Interface**: Integrar suporte a GLUI (menuglui.h/cpp)
2. **Physics/Animation**: Adicionar módulo para dinâmica (physics.cpp)
3. **Tests**: Criar testes unitários para cada módulo
4. **Documentation**: Adicionar Doxygen comments

## Build

```bash
cd engine/build
cmake ..
make
./engine ../../configs/solar_system.xml
```

## Estatísticas

| Arquivo | Linhas | Responsabilidade |
|---------|--------|-------------------|
| engine.cpp | 120 | Main (reduzido em 78%) |
| rendering.cpp | 180 | Renderização |
| config.cpp | 175 | Configuração |
| input.cpp | 60 | Input |
| model.cpp | 50 | Gerenciamento de modelos |
| geometry.h | 60 | Estruturas de dados |
| **Total .cpp** | **585** | **(↑ de 531, mas organizado)** |

*Nota: O aumento é devido aos comentários e organização, mas reduzindo significativamente a complexidade cognitiva.*
