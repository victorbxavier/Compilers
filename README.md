# Compilador MiniJava

Projeto da disciplina DIM0164 — Compiladores (2026.1, UFRN).

Implementação de um compilador para a linguagem MiniJava, composto por:

- **Pré-processador** — remoção de comentários e espaços em excesso (integrado ao Flex)
- **Analisador Léxico** — tokenização do código fonte usando Flex (reporta linha e coluna)
- **Analisador Sintático** — parser recursivo descendente com construção de AST
- **Árvore Sintática Abstrata (AST)** — representação estruturada do programa em memória
- **Tabela de Símbolos** — registro de classes, métodos e variáveis com suporte a herança
- **Analisador Semântico** — verificação de tipos, escopo, chamadas de método e compatibilidade

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

Para limpar os artefatos:

```bash
make clean
```

### Compilação manual (sem Make)

```bash
flex src/lexer.l
g++ lex.yy.cc -o lexer -lfl
```

## Execução

```bash
./build/compiler [opções] <arquivo.ling>
```

### Opções

| Flag                 | Descrição                                          |
| -------------------- | -------------------------------------------------- |
| `--tokens`           | Exibe a lista de tokens (para após análise léxica) |
| `--ast`              | Exibe a árvore sintática abstrata                  |
| `--symbols`          | Exibe a tabela de símbolos                         |
| `--suggest`          | Ativa sugestões de correção léxica                 |
| `--stop-first-error` | Para no primeiro erro léxico                       |
| `--help`             | Exibe mensagem de ajuda                            |

### Exemplos

```bash
# Análise completa (léxica + sintática + semântica)
./build/compiler assets/prog-factorial-v2.ling
./build/compiler assets/prog-bubblesort-v2.ling

# Exibir a AST e tabela de símbolos
./build/compiler --ast --symbols assets/prog-factorial-v2.ling

# Exibir apenas a lista de tokens
./build/compiler --tokens assets/prog-factorial-v2.ling

# Testar programa com erros semânticos
./build/compiler assets/prog-semantic-error-v2.ling
```

### Saída esperada

- **Sucesso:** `=== Análise semântica concluída sem erros ===`
- **Erro léxico:** mensagem com linha:coluna e sugestão de correção
- **Erro sintático:** mensagem com linha:coluna, token esperado e token encontrado
- **Erro semântico:** mensagem com linha, tipo do erro e contexto

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
   Resultado: sucesso ou lista de erros
```

## Estrutura do Projeto

```
src/
├── token.h               # Definição dos tipos de token (enum + struct com linha:coluna)
├── lexer.l               # Analisador léxico (Flex)
├── ast.h                 # Nós da Árvore Sintática Abstrata
├── parser.h              # Analisador sintático (constrói AST + preenche tabela)
├── symbol_table.h        # Tabela de símbolos (com busca hierárquica e herança)
├── semantic_analyzer.h   # Analisador semântico (percorre AST + consulta tabela)
└── main.cpp              # Programa principal (orquestra o pipeline)
assets/                   # Programas de teste (.ling)
docs/                     # Gramática, checklists e documentação
relatorio/                # Relatório técnico (LaTeX)
Makefile                  # Automação de build
```

## Equipe

- JOÃO GUILHERME LOPES ALVES DA COSTA
- JOSÉ DAVI VIANA FRANCELINO
- JUSCELINO PEREIRA DE ARAUJO
- THIAGO DE OLIVEIRA CORDEIRO
- VICTOR BASTOS XAVIER
