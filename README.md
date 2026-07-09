# Compilador MiniJava

Projeto da disciplina DIM0164 — Compiladores (2026.1, UFRN).

Implementação de um compilador para a linguagem MiniJava, composto por:

- **Analisador Léxico** — tokenização do código fonte usando Flex (reporta linha e coluna)
- **Analisador Sintático** — parser recursivo descendente com construção de AST
- **Árvore Sintática Abstrata (AST)** — representação estruturada do programa em memória
- **Tabela de Símbolos** — registro de classes, métodos e variáveis com suporte a herança
- **Analisador Semântico** — verificação de tipos, escopo, chamadas de método e compatibilidade
- **Geração de Código Intermediário** — código de três endereços (3AC) gerado a partir da AST

## Pré-requisitos

- `g++` (C++17)
- `flex`
- `make`

No Ubuntu/WSL:

```bash
sudo apt install build-essential flex
```

## Compilação

```bash
make all
```

O binário será gerado em `build/compiler`.

Para limpar os artefatos e recompilar:

```bash
make clean && make all
```

## Execução

```bash
./build/compiler [opções] <arquivo.ling>
```

### Opções

| Flag                 | Descrição                                              |
| -------------------- | ------------------------------------------------------ |
| `--tokens`           | Exibe a lista de tokens (para após análise léxica)     |
| `--ast`              | Exibe a árvore sintática abstrata                      |
| `--symbols`          | Exibe a tabela de símbolos                             |
| `--no-ir`            | Oculta o código intermediário (3AC exibido por padrão) |
| `--suggest`          | Ativa sugestões de correção léxica e sintática         |
| `--stop-first-error` | Para no primeiro erro léxico                           |
| `--help`             | Exibe mensagem de ajuda                                |

### Exemplos

```bash
# Compilação completa (exibe código intermediário por padrão)
./build/compiler assets/unidade-2/prog-factorial-v2.ling

# Exibir AST + tabela + código intermediário
./build/compiler --ast --symbols assets/unidade-2/prog-bubblesort-v2.ling

# Apenas tokens
./build/compiler --tokens assets/unidade-2/prog-factorial-v2.ling

# Ocultar código intermediário (só mostra mensagens de sucesso/erro)
./build/compiler --no-ir assets/unidade-2/prog-factorial-v2.ling

# Programa com erros semânticos
./build/compiler assets/unidade-2/prog-semantic-error-v2.ling
```

### Saída esperada

- **Sucesso:** código intermediário (3AC) + `=== Compilação concluída com sucesso ===`
- **Erro léxico:** mensagem com linha:coluna e sugestão de correção
- **Erro sintático:** mensagem com linha:coluna, token esperado vs encontrado
- **Erro semântico:** mensagem com linha e descrição do erro de tipo/escopo

## Pipeline de Compilação

```
Código Fonte (.ling)
       │
       ▼
┌─────────────┐
│    Lexer    │──▶ Lista de Tokens
│   (Flex)    │
└─────────────┘
       │
       ▼
┌─────────────┐
│   Parser    │──▶ AST + Tabela de Símbolos
│(Rec. Desc.) │
└─────────────┘
       │
       ▼
┌─────────────┐
│  Análise    │──▶ Verificação de tipos, escopo, herança
│  Semântica  │
└─────────────┘
       │
       ▼
┌─────────────┐
│  Geração de │──▶ Código de Três Endereços (3AC)
│  Código     │
└─────────────┘
       │
       ▼
   Saída: código intermediário ou lista de erros
```

## Estrutura do Projeto

```
src/
├── token.h                # Definição dos tipos de token
├── lexer.l                # Analisador léxico (Flex)
├── ast.h                  # Nós da Árvore Sintática Abstrata
├── parser.h               # Analisador sintático (constrói AST)
├── symbol_table.h         # Tabela de símbolos
├── semantic_analyzer.h    # Analisador semântico
├── three_address_code.h   # Estrutura do código de três endereços
├── codegen.h              # Gerador de código intermediário
└── main.cpp               # Programa principal (pipeline)
assets/
├── unidade-1/             # Programas de teste (gramática antiga)
└── unidade-2/             # Programas de teste (gramática nova)
docs/                      # Documentação, checklists, fluxos
Makefile                   # Automação de build
```

## Equipe

- JOÃO GUILHERME LOPES ALVES DA COSTA
- JOSÉ DAVI VIANA FRANCELINO
- JUSCELINO PEREIRA DE ARAUJO
- THIAGO DE OLIVEIRA CORDEIRO
- VICTOR BASTOS XAVIER
