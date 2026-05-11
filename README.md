# Compilador MiniJava

Projeto da disciplina DIM0164 — Compiladores (2026.1, UFRN).

Implementação de um compilador para a linguagem MiniJava, composto por:

- **Pré-processador** — remoção de comentários e espaços em excesso (integrado ao Flex)
- **Analisador Léxico** — tokenização do código fonte usando Flex
- **Analisador Sintático** — parser recursivo descendente baseado em gramática LL(1)
- **Tabela de Símbolos** — registro de classes, métodos e variáveis declaradas

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
./build/compiler <arquivo.ling>
```

### Exemplos

```bash
# Análise completa (léxica + sintática)
./build/compiler assets/prog-factorial.ling
./build/compiler assets/prog-bubblesort.ling

# Exibir apenas a lista de tokens (sem parsing)
./build/compiler --tokens assets/prog-factorial.ling
```

### Saída esperada

- **Sucesso:** `=== Código sintaticamente correto ===`
- **Erro léxico:** mensagem com linha e sugestão de correção
- **Erro sintático:** mensagem com linha, token esperado e token encontrado

## Estrutura do Projeto

```
src/
├── token.h       # Definição dos tipos de token (enum + struct)
├── lexer.l       # Analisador léxico (Flex)
├── parser.h      # Analisador sintático (recursive descent)
└── main.cpp      # Programa principal (pipeline léxico → sintático)
assets/           # Programas de teste (.ling)
docs/             # Gramática, checklist e documentação
Makefile          # Automação de build
```

## Equipe

- JOÃO GUILHERME LOPES ALVES DA COSTA
- JOSÉ DAVI VIANA FRANCELINO
- JUSCELINO PEREIRA DE ARAUJO
- THIAGO DE OLIVEIRA CORDEIRO
- VICTOR BASTOS XAVIER
