# Fluxo de Compilação — Unidade 2 (AST + Análise Semântica)

## Visão Geral do Pipeline

```
┌─────────────┐    ┌──────────┐    ┌──────────────┐    ┌─────────┐    ┌───────────────┐
│ Código Fonte│───▶│  Lexer   │───▶│   Parser     │───▶│   AST   │───▶│   Análise     │
│ (.ling)     │    │  (Flex)  │    │ (Rec. Desc.) │    │(árvore) │    │  Semântica    │
└─────────────┘    └──────────┘    └──────────────┘    └─────────┘    └───────────────┘
                        │                 │                  │                │
                   Lista de          Constrói a         Estrutura        Percorre a AST
                   Tokens            AST + preenche     em memória       consultando a
                                     Tabela de Símb.    do programa      Tabela de Símb.
```

O compilador agora executa **4 etapas sequenciais**:

1. **Análise Léxica** — texto → tokens
2. **Análise Sintática** — tokens → AST (e preenche tabela de símbolos)
3. **AST pronta** — representação estruturada do programa em memória
4. **Análise Semântica** — percorre a AST verificando tipos, escopo e consistência

---

## O Que Mudou da Unidade 1 para a Unidade 2

### Unidade 1: Parser como validador
```
Tokens ──▶ Parser ──▶ "correto" ou "erro sintático"
                  └──▶ Tabela de Símbolos (só impressa, nunca consultada)
```

### Unidade 2: Parser como construtor + verificação semântica
```
Tokens ──▶ Parser ──▶ AST (árvore de objetos em memória)
                  └──▶ Tabela de Símbolos (preenchida E consultada)
                              │
                              ▼
                  Análise Semântica percorre a AST
                              │
                              ▼
                  "correto" ou "erro semântico na linha X: ..."
```

---

## Etapa 1: Análise Léxica (mesmo conceito, melhorias pontuais)

```
"class Factorial { public int compute(int num) { ... } }"
                          │
                          ▼
[CLASS, ID("Factorial"), LBRACE, PUBLIC, INT, ID("compute"), LPAREN, ...]
```

**O que muda na unidade 2:**
- Agora reporta **linha E coluna** nos erros (antes só linha)
- Flag `--stop-first-error` controla se para no primeiro erro ou lista todos

---

## Etapa 2: Análise Sintática (agora constrói a AST)

### Antes (Unidade 1) — só validava:
```cpp
void parseCmd() {
    if (currentType() == TokenType::IF) {
        match(IF); match(LPAREN);
        parseExp();          // valida e descarta
        match(RPAREN);
        parseCmd();          // valida e descarta
        match(ELSE);
        parseCmd();          // valida e descarta
    }
}
// Retorno: void (nada é produzido, só "passou ou não passou")
```

### Agora (Unidade 2) — constrói nós:
```cpp
unique_ptr<Stmt> parseCmd() {
    if (currentType() == TokenType::IF) {
        match(IF); match(LPAREN);
        auto cond = parseExp();       // retorna nó de expressão
        match(RPAREN); match(LBRACE);
        auto then = parseLcom();      // retorna lista de comandos
        match(RBRACE);
        
        vector<unique_ptr<Stmt>> elseBody;
        if (currentType() == ELSE) {  // else é opcional agora
            match(ELSE); match(LBRACE);
            elseBody = parseLcom();
            match(RBRACE);
        }
        
        return make_unique<If>(move(cond), move(then), move(elseBody));
    }
}
// Retorno: um nó If com seus filhos (condição, corpo-then, corpo-else)
```

**A diferença fundamental:** cada função `parse*()` agora **retorna** um objeto que representa aquela construção. Esses objetos se encaixam formando a árvore.

---

## Etapa 3: A AST em Memória

Após o parser terminar, a AST existe como uma árvore de objetos interligados:

```
Program (raiz)
│
├── MainClass: "Factorial"
│   └── body: [Print]
│              └── exp: MethodCall
│                       ├── object: NewObject("Fac")
│                       ├── method: "ComputeFac"
│                       └── args: [IntLiteral(10)]
│
└── ClassDecl: "Fac"
    └── methods:
        └── MethodDecl: "ComputeFac" (retorna int)
            ├── params: [(int, "num")]
            ├── locals: [(int, "num_aux")]
            ├── body:
            │   └── If
            │       ├── cond: LessThan
            │       │          ├── left: Id("num")
            │       │          └── right: IntLiteral(1)
            │       ├── then: [Assign("num_aux", IntLiteral(1))]
            │       └── else: [Assign("num_aux", Times(Id("num"), MethodCall(...)))]
            └── returnExp: Id("num_aux")
```

### Cada nó é uma classe C++:

```
ASTNode (base abstrata)
├── Program         → filhos: MainClass + vector<ClassDecl>
├── MainClass       → filhos: nome (string) + body (vector<Stmt>)
├── ClassDecl       → filhos: nome, parent, vars, methods
├── MethodDecl      → filhos: tipo retorno, nome, params, locals, body, returnExp
├── VarDecl         → filhos: tipo, nome
│
├── Stmt (base para comandos)
│   ├── If          → filhos: condition (Exp), thenBody, elseBody
│   ├── While       → filhos: condition (Exp), body
│   ├── Print       → filhos: expression (Exp)
│   ├── Assign      → filhos: variable (string), value (Exp)
│   └── ArrayAssign → filhos: array (string), index (Exp), value (Exp)
│
└── Exp (base para expressões)
    ├── And         → filhos: left (Exp), right (Exp)
    ├── LessThan    → filhos: left (Exp), right (Exp)
    ├── Plus        → filhos: left (Exp), right (Exp)
    ├── Minus       → filhos: left (Exp), right (Exp)
    ├── Times       → filhos: left (Exp), right (Exp)
    ├── Not         → filhos: expr (Exp)
    ├── ArrayLookup → filhos: array (Exp), index (Exp)
    ├── ArrayLength → filhos: array (Exp)
    ├── MethodCall  → filhos: object (Exp), method (string), args (vector<Exp>)
    ├── IntLiteral  → filhos: value (int)
    ├── True        → (sem filhos)
    ├── False       → (sem filhos)
    ├── IdentifierExp → filhos: name (string)
    ├── This        → (sem filhos)
    ├── NewObject   → filhos: className (string)
    └── NewArray    → filhos: size (Exp)
```

### Precedência de operadores na AST

A gramática nova garante que operadores de maior precedência ficam **mais profundos** na árvore. Para `1 + 2 * 3`:

```
Plus                    ← + está mais alto (menor precedência)
├── left: IntLiteral(1)
└── right: Times        ← * está mais fundo (maior precedência)
           ├── left: IntLiteral(2)
           └── right: IntLiteral(3)
```

Isso é consequência da hierarquia na gramática:
```
Exp → Andexp → Relexp → Addexp → Mulexp → Unexp → Psfexp → Priexp
      (&&)      (<)      (+,-)    (*)      (!)    ([], ., .method)
      menor                                              maior
      precedência                                        precedência
```

---

## Etapa 4: Análise Semântica (percorrimento da AST)

Após a AST estar completa, um módulo separado (ex: `semantic_analyzer.h`) percorre
a árvore **de cima para baixo**, verificando regras de significado.

### Fluxo da análise semântica:

```
                    AST (Program)
                         │
          ┌──────────────┼──────────────┐
          ▼              ▼              ▼
    1ª passada:     2ª passada:    Reportar
    Registrar       Verificar      erros
    todas as        tipos,         encontrados
    classes e       escopo,
    métodos na      herança
    tabela
```

### Passada 1 — Registro (preenche a tabela completamente):
```
Para cada classe no programa:
  - Registra a classe na tabela (nome, campos, métodos)
  - Se tem extends, registra o pai

Resultado: tabela sabe que "Fac" existe, tem método "ComputeFac" que retorna int, etc.
```

### Passada 2 — Verificação (consulta a tabela):
```
Para cada classe:
  Para cada método:
    Para cada comando no corpo:
      - Se é Assign("x", exp):
          → "x" existe? (consulta tabela)
          → Qual o tipo de "x"? (consulta tabela)
          → Qual o tipo de exp? (analisa recursivamente)
          → São compatíveis? Se não → ERRO SEMÂNTICO
      - Se é If(cond, ...):
          → Tipo de cond é boolean? Se não → ERRO
    Para a expressão de retorno:
      → Tipo bate com o declarado no método? Se não → ERRO
```

### Exemplo de verificação numa expressão:

Dado o nó `Plus(Id("x"), IntLiteral(1))`:

```
analyzeExp(Plus):
├── analyzeExp(Id("x")):
│   └── consulta tabela → "x" é do tipo int ✓
│   └── retorna: int
├── analyzeExp(IntLiteral(1)):
│   └── retorna: int
└── Ambos são int? SIM → Plus retorna int ✓
```

Dado o nó `Plus(Id("flag"), IntLiteral(1))` onde `flag` é boolean:

```
analyzeExp(Plus):
├── analyzeExp(Id("flag")):
│   └── consulta tabela → "flag" é do tipo boolean
│   └── retorna: boolean
├── analyzeExp(IntLiteral(1)):
│   └── retorna: int
└── Ambos são int? NÃO → ERRO: "operador + requer dois int, recebeu boolean e int"
```

---

## Fluxo Completo no `main.cpp` (Unidade 2)

```cpp
1.  Parseia argumentos (--tokens, --ast, --symbols, --suggest, --stop-first-error, arquivo)
2.  Abre o arquivo fonte (.ling)
3.  Executa análise léxica: yylex() → tokenList[]
4.  Se houve erro léxico → reporta e encerra (ou continua, depende da flag)
5.  Se modo --tokens → imprime lista de tokens e encerra
6.  Cria o Parser com a lista de tokens
7.  Executa análise sintática: auto ast = parser.parse()
    → Parser constrói a AST E preenche a tabela de símbolos
8.  Se erro sintático → reporta e encerra
9.  Se modo --ast → imprime a AST e encerra
10. Se modo --symbols → imprime tabela de símbolos
11. Executa análise semântica: analyzer.analyze(ast)
    → Percorre a AST consultando a tabela
12. Se erros semânticos → reporta todos
13. Se tudo ok → "Compilação concluída sem erros"
```

---

## Relação entre Arquivos (Unidade 2)

```
token.h ◄──────────────────────────────────────────────┐
   ▲                                                    │
   │                                                    │
   ├── lexer.l (tokenização)                            │
   │                                                    │
   ├── ast.h (define nós da árvore) ◄──────┐            │
   │                                       │            │
   ├── parser.h (consome tokens, constrói AST, preenche tabela)
   │       │                               │            │
   │       ├── symbol_table.h              │            │
   │       └── ast.h ─────────────────────-┘            │
   │                                                    │
   ├── semantic_analyzer.h (percorre AST, consulta tabela)
   │       ├── ast.h                                    │
   │       └── symbol_table.h                           │
   │                                                    │
   └── main.cpp (orquestra tudo) ──────────────────────-┘
```

---

## Resumo Rápido

| Arquivo | Etapa | Entrada | Saída |
|---------|-------|---------|-------|
| `token.h` | — | — | Definições compartilhadas |
| `lexer.l` | Análise Léxica | Código fonte (chars) | `vector<Token>` |
| `ast.h` | — | — | Definição dos nós da AST |
| `parser.h` | Análise Sintática | `vector<Token>` | AST + Tabela de Símbolos preenchida |
| `symbol_table.h` | Tabela de Símbolos | Chamadas do parser | Registro consultável de declarações |
| `semantic_analyzer.h` | Análise Semântica | AST + Tabela | Lista de erros semânticos |
| `main.cpp` | Orquestração | CLI + arquivo | Coordena tudo, imprime resultados |

---

## Comparação Direta: Unidade 1 vs Unidade 2

```
UNIDADE 1:
  .ling → [Lexer] → tokens → [Parser (valida)] → "ok/erro"
                                     └──▶ Tabela (impressa)

UNIDADE 2:
  .ling → [Lexer] → tokens → [Parser (constrói)] → AST
                                     └──▶ Tabela (preenchida)
                                                      │
                                                      ▼
                                          [Análise Semântica]
                                          percorre AST + consulta Tabela
                                                      │
                                                      ▼
                                              "ok" ou erros semânticos
```

A grande diferença: o parser deixa de ser o "fim da linha" e passa a ser uma etapa intermediária que **produz** algo (a AST) para a próxima etapa consumir.
