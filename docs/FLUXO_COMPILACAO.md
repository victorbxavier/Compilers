# Fluxo de Compilação — Compilador MiniJava

## Visão Geral do Pipeline

```
┌─────────────┐     ┌──────────────┐     ┌──────────────────┐     ┌──────────────────┐
│ Código Fonte│────▶│ Analisador   │────▶│ Analisador       │────▶│ Tabela de        │
│ (.ling)     │     │ Léxico (Flex)│     │ Sintático        │     │ Símbolos         │
└─────────────┘     └──────────────┘     └──────────────────┘     └──────────────────┘
                         │                       │                        │
                    Lista de Tokens         Validação da            Registro de
                    (tokenList)            estrutura gramatical     classes, métodos
                                                                   e variáveis
```

O compilador executa **3 etapas sequenciais**:

1. **Análise Léxica** — transforma o texto em tokens
2. **Análise Sintática** — valida a estrutura gramatical consumindo os tokens
3. **Construção da Tabela de Símbolos** — registra declarações (integrada ao parser)

---

## Arquivos e suas Responsabilidades

### `src/token.h` — Definição dos Tokens

**Papel:** Define o "vocabulário" do compilador — todos os tipos de token que a linguagem reconhece.

**Conteúdo:**
- `enum class TokenType` — enumeração com todos os tipos possíveis (palavras-chave, operadores, delimitadores, literais)
- `struct Token` — estrutura que armazena: tipo, lexema (texto original) e linha no código fonte
- `tokenTypeToString()` — converte um tipo de token para string legível (usado em mensagens de erro)
- `printToken()` — imprime um token formatado no terminal

**Quando é usado:** Por todos os outros arquivos. É a "linguagem comum" entre o lexer, o parser e o main.

---

### `src/lexer.l` — Analisador Léxico (Flex)

**Papel:** Lê o código fonte caractere a caractere e produz uma lista de tokens. Também atua como **pré-processador**, removendo comentários e espaços.

**Como funciona:**
1. Recebe o arquivo `.ling` via `yyin` (ponteiro FILE)
2. Aplica regras de expressões regulares na ordem definida
3. Para cada match, chama `addToken()` que insere o token na lista global `tokenList`
4. Ignora comentários (`//` e `/* */`) e whitespace
5. Se encontra um caractere inválido, reporta erro léxico com sugestão (via distância de Levenshtein)

**Variáveis globais exportadas:**
- `tokenList` — vetor com todos os tokens gerados
- `lexerHadError` — flag indicando se houve erro léxico
- `lineNumber` — contador de linhas (para mensagens de erro)

**Prioridade das regras:**
```
Comentários → Whitespace → Palavras-chave → Operadores → Delimitadores → Identificadores → Números → Erros
```

> Palavras-chave são reconhecidas **antes** de identificadores porque aparecem primeiro no arquivo `.l`.

---

### `src/parser.h` — Analisador Sintático (Recursive Descent)

**Papel:** Consome a lista de tokens e verifica se a sequência respeita a gramática LL(1) da linguagem MiniJava. Simultaneamente, constrói a tabela de símbolos.

**Como funciona:**
1. Recebe o `vector<Token>` produzido pelo lexer
2. Mantém um cursor (`pos_`) que avança token a token
3. Cada regra da gramática é uma função (`parseProg`, `parseMainC`, `parseCmd`, etc.)
4. Usa `match()` para consumir tokens esperados — se o token atual não bate, reporta erro
5. Usa lookahead (1 token) para decidir qual produção seguir

**Estrutura das regras gramaticais:**
```
Prog     → MainC DefCl
MainC    → 'class' Id '{' 'public' 'static' 'void' 'main' '(' ... ')' '{' CmdList '}' '}'
DefCl    → 'class' Id DefCl' | λ
DefMet   → 'public' Type Id '(' Args ')' '{' DefVar CmdList 'return' Exp ';' '}' DefMet | λ
Cmd      → '{' CmdList '}' | 'if' ... | 'while' ... | 'System.out.println(...)' | Id Cmd'
Exp      → PrimExp Exp'
```

**Tabela de Símbolos (integrada):** Durante o parsing, ao encontrar declarações de classes, métodos, variáveis e parâmetros, o parser chama `symbolTable_.addSymbol()` registrando nome, tipo, escopo e categoria.

---

### `src/symbol_table.h` — Tabela de Símbolos

**Papel:** Armazena todas as declarações encontradas durante a análise sintática.

**Conteúdo:**
- `enum class SymbolCategory` — categorias: CLASS, METHOD, INSTANCE_VAR, LOCAL_VAR, PARAMETER
- `struct Symbol` — registro com nome, tipo, escopo, linha e categoria
- `class SymbolTable` — vetor de símbolos com métodos para adicionar e imprimir

**Quando é preenchida:** Pelo parser, durante a análise sintática, sempre que uma declaração é reconhecida.

---

### `src/main.cpp` — Programa Principal (Orquestrador)

**Papel:** Coordena todo o pipeline de compilação. É o ponto de entrada do programa.

**Fluxo de execução:**

```cpp
1. Parseia argumentos da linha de comando (--tokens, --help, arquivo)
2. Abre o arquivo fonte (.ling)
3. Configura o Flex (yyin = file)
4. Executa análise léxica: yylex()
5. Adiciona token END_OF_FILE à lista
6. Se houve erro léxico → encerra com código 1
7. Se modo --tokens → imprime lista de tokens e encerra
8. Cria o Parser com a lista de tokens
9. Executa análise sintática: parser.parse()
10. Se sucesso → imprime "Código sintaticamente correto" + tabela de símbolos
11. Se falha → imprime erro e encerra com código 1
```

---

### `Makefile` — Automação de Build

**Papel:** Automatiza a compilação do projeto em dois passos:

```
1. flex src/lexer.l → build/lex.yy.c        (gera código C do lexer)
2. g++ compila lex.yy.c + main.cpp → build/compiler
```

**Dependências entre objetos:**
```
build/compiler
├── build/main.o        ← src/main.cpp + token.h + parser.h + symbol_table.h
└── build/lex.yy.o     ← build/lex.yy.c (gerado pelo Flex) + token.h
```

---

## Fluxo Completo: Do Código Fonte ao Resultado

```
prog-factorial.ling
        │
        ▼
┌─────────────────────────────────────────────────────────┐
│ main.cpp: abre arquivo, configura yyin                  │
└─────────────────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────┐
│ lexer.l (Flex): lê caracteres, remove comentários,      │
│ identifica tokens, preenche tokenList[]                  │
│                                                         │
│ Entrada: "class Factorial {"                            │
│ Saída:   [CLASS, ID("Factorial"), LBRACE, ...]          │
└─────────────────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────┐
│ parser.h: consome tokenList[], valida gramática LL(1),  │
│ constrói tabela de símbolos                             │
│                                                         │
│ Entrada: [CLASS, ID("Factorial"), LBRACE, ...]          │
│ Saída:   sucesso/erro + SymbolTable preenchida          │
└─────────────────────────────────────────────────────────┘
        │
        ▼
┌─────────────────────────────────────────────────────────┐
│ main.cpp: imprime resultado final                       │
│   ✓ "Código sintaticamente correto" + tabela            │
│   ✗ "Erro sintático na linha X: esperado Y, encontrou Z"│
└─────────────────────────────────────────────────────────┘
```

---

## Relação entre Arquivos (Grafo de Dependências)

```
token.h ◄──────────────────────────────┐
   ▲                                    │
   │                                    │
   ├── lexer.l (usa TokenType, addToken)│
   │                                    │
   ├── parser.h (usa Token, TokenType) ─┤
   │       │                            │
   │       └── symbol_table.h           │
   │                                    │
   └── main.cpp (usa Token, printToken)─┘
```

---

## Resumo Rápido

| Arquivo | Etapa | Entrada | Saída |
|---------|-------|---------|-------|
| `token.h` | — | — | Definições de tipos compartilhadas |
| `lexer.l` | Análise Léxica | Código fonte (chars) | `vector<Token>` |
| `parser.h` | Análise Sintática | `vector<Token>` | Validação + Tabela de Símbolos |
| `symbol_table.h` | Tabela de Símbolos | Chamadas do parser | Registro de declarações |
| `main.cpp` | Orquestração | Argumentos CLI + arquivo | Coordena tudo, imprime resultado |
| `Makefile` | Build | Código fonte C++/Flex | Binário `build/compiler` |
