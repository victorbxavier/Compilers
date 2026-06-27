# Modificações Realizadas — Unidade 2

## O que mudou da Unidade 1 para a Unidade 2?

Na Unidade 1, o compilador apenas **validava** se o código estava sintaticamente correto e imprimia a tabela de símbolos. O parser era um "validador" — consumia tokens e dizia sim ou não.

Na Unidade 2, o compilador passou a **entender** o programa:

- O parser agora **constrói** uma Árvore Sintática Abstrata (AST) em memória
- A tabela de símbolos é **consultada** (não apenas impressa)
- Um novo módulo de **análise semântica** percorre a AST verificando regras de significado

```
UNIDADE 1:  .ling → Lexer → Tokens → Parser (valida) → "ok/erro" + Tabela (impressa)

UNIDADE 2:  .ling → Lexer → Tokens → Parser (constrói AST) → Análise Semântica → "ok/erros"
                                            └──▶ Tabela (preenchida E consultada)
```

---

## Resumo das Capacidades Novas

| Capacidade                    | Descrição                                                                            |
| ----------------------------- | ------------------------------------------------------------------------------------ |
| **AST**                       | Representação em árvore do programa, com nós para expressões, comandos e declarações |
| **Precedência de operadores** | Gramática reescrita com hierarquia: `&&` < `<` < `+,-` < `*` < `!` < `[],.`          |
| **Análise semântica**         | Verifica tipos, escopo, existência de variáveis, chamadas de método, herança         |
| **Erros com linha:coluna**    | Lexer agora reporta posição exata                                                    |
| **Múltiplos erros**           | O compilador lista todos os erros encontrados em vez de parar no primeiro            |
| **Flags de CLI**              | `--ast`, `--symbols`, `--suggest`, `--stop-first-error`                              |
| **Herança**                   | Tabela de símbolos suporta busca em classes pai                                      |

---

## Detalhamento por Arquivo

### `src/token.h` — Definição dos tokens

**O que mudou:**

- Adicionado campo `int column` ao struct `Token`
- `printToken()` agora mostra formato `linha:coluna`

**Por quê:** Para mensagens de erro mais precisas (antes só mostrava a linha).

---

### `src/lexer.l` — Analisador léxico

**O que mudou:**

- Contador de coluna (`columnNumber`) — reseta a cada `\n`, incrementa a cada caractere
- Flag `stopOnFirstError` — se ativa, o lexer retorna após o primeiro erro
- Flag `showSuggestions` — controla se sugestões de Levenshtein são exibidas (ativada por `--suggest`)
- Todas as regras agora passam a coluna para `addToken()`
- Threshold de sugestão aumentado para distância ≤ 3
- Opção `%option noinput` para eliminar warnings

**Comportamento padrão:** lista todos os erros léxicos encontrados. Com `--stop-first-error`, para no primeiro. Com `--suggest`, mostra "você quis dizer X?" para tokens inválidos próximos de keywords e sugestões estruturais para erros sintáticos.

---

### `src/ast.h` — Árvore Sintática Abstrata (NOVO)

**O que é:** Hierarquia de classes C++ que representa a estrutura do programa em memória. Cada construção da linguagem vira um nó da árvore.

**Estrutura:**

```
ASTNode (base com campo `line` e método virtual `print()`)
│
├── Program           → MainClass + vector<ClassDecl>
├── MainClass         → nome, parâmetro, body
├── ClassDecl         → nome, parent, vars, methods
├── MethodDecl        → tipo retorno, nome, params, locals, body, returnExp
├── VarDecl           → tipo, nome
├── Formal            → tipo, nome (parâmetro de método)
│
├── Type (base para tipos)
│   ├── IntType, BoolType, IntArrayType, IdentifierType
│
├── Stmt (base para comandos)
│   ├── IfStmt        → condition, thenBody, elseBody (else opcional)
│   ├── WhileStmt     → condition, body
│   ├── PrintStmt     → expression
│   ├── AssignStmt    → variable, value
│   └── ArrayAssignStmt → array, index, value
│
└── Exp (base para expressões)
    ├── AndExp, LessThanExp, PlusExp, MinusExp, TimesExp
    ├── NotExp
    ├── ArrayLookupExp, ArrayLengthExp
    ├── MethodCallExp  → object, method, args
    ├── IntLiteralExp, TrueLiteralExp, FalseLiteralExp
    ├── IdentifierExp, ThisExp
    ├── NewObjectExp, NewArrayExp
```

**Utilitário:** `typeToString()` converte nó Type em string ("int", "boolean", "int[]", "NomeClasse").

---

### `src/parser.h` — Analisador sintático (REESCRITO)

**Mudança principal:** cada função `parse*()` agora **retorna** um nó da AST em vez de apenas validar.

**Gramática nova** (do professor, valdigleis.site):

- `if` e `while` agora **exigem chaves** `{ }` no corpo
- `else` é **opcional**
- Expressões usam hierarquia de precedência com 7 níveis:

```
parseExp() → parseAndexp() → parseRelexp() → parseAddexp() → parseMulexp() → parseUnexp() → parsePsfexp() → parsePriexp()
   (&&)           (<)           (+, -)           (*)            (!)         ([], .length, .method())    (literais, id, this, new)
```

**Funções novas:**

- `parseAndexp()` — operador `&&`
- `parseRelexp()` — operador `<`
- `parseAddexp()` — operadores `+`, `-`
- `parseMulexp()` — operador `*`
- `parseUnexp()` — operador `!`
- `parsePsfexp()` — operadores pós-fixos (`[]`, `.length`, `.método()`)
- `parsePriexp()` — expressões primárias
- `parseLcom()` — lista de comandos
- `parseCom()` — comando individual
- `parseComAss()` — atribuição simples ou em array
- `parseLexp()` — lista de expressões (argumentos de método)

**Tratamento de erros:** O parser aceita flag `stopOnFirstError`. Por padrão, reporta o erro e para a construção da AST (recuperação completa de erros sintáticos exigiria reestruturação significativa, mas a mensagem é descritiva).

---

### `src/symbol_table.h` — Tabela de símbolos (EXPANDIDA)

**O que mudou:**

- Campo `parent` no struct `Symbol` (para herança)
- `lookup(name, scope)` — busca hierárquica: método local → classe → classe pai
- `lookupClass(name)` — busca classe por nome
- `lookupMethod(className, methodName)` — busca método com suporte a herança
- `isDuplicate(name, scope, cat)` — detecta declarações duplicadas
- `getMethodParams(className, methodName)` — retorna lista de parâmetros formais de um método

**Busca hierárquica:** Quando o analisador semântico pergunta "qual o tipo de `x`?", a tabela procura primeiro no escopo local (método), depois na classe, depois na classe pai (herança).

---

### `src/semantic_analyzer.h` — Analisador semântico (NOVO)

**O que faz:** Percorre a AST após o parsing e verifica regras de significado.

**Verificações realizadas:**

| Verificação                                     | Exemplo de erro                                        |
| ----------------------------------------------- | ------------------------------------------------------ |
| Tipos de operadores aritméticos (`+`, `-`, `*`) | `true + 1` → operador '+' requer int                   |
| Tipo do operador relacional (`<`)               | `true < 1` → operador '<' requer int                   |
| Tipo do operador lógico (`&&`)                  | `1 && true` → operador '&&' requer boolean             |
| Tipo do operador de negação (`!`)               | `!5` → operador '!' requer boolean                     |
| Condição de `if`/`while`                        | `if (x + 1)` → condição deve ser boolean               |
| `System.out.println`                            | `println(flag)` → requer int                           |
| Compatibilidade de atribuição                   | `x = true` onde x é int → incompatível                 |
| Existência de variáveis                         | `y = 10` onde y não foi declarada                      |
| Existência de classes                           | `new Xyz()` onde Xyz não existe                        |
| Chamadas de método                              | Verifica existência, número e tipos de argumentos      |
| Tipo de retorno                                 | Valor retornado deve ser compatível com tipo declarado |
| Acesso a array                                  | `x[i]` — x deve ser int[], i deve ser int              |
| `.length`                                       | Requer int[]                                           |
| Herança                                         | Subtipo é compatível com supertipo                     |
| Classes vazias                                  | Classe sem atributos nem métodos → erro                |

**Estratégia:** Duas passadas implícitas:

1. O parser já preencheu a tabela com todas as declarações
2. O analisador percorre a AST consultando a tabela para cada expressão/comando

---

### `src/main.cpp` — Programa principal (REESCRITO)

**Pipeline:**

```
1. Parseia argumentos de CLI
2. Análise léxica (Flex → tokenList)
3. Se erro léxico → lista todos e encerra
4. Se --tokens → imprime tokens e encerra
5. Análise sintática (Parser → AST + Tabela)
6. Se erro sintático → reporta e encerra
7. Se --ast → imprime a AST
8. Se --symbols → imprime tabela de símbolos
9. Análise semântica (percorre AST + consulta tabela)
10. Se erros semânticos → lista todos
11. Se tudo ok → "Análise semântica concluída sem erros"
```

**Flags disponíveis:**

- `--tokens` — mostra lista de tokens e para
- `--ast` — mostra a árvore sintática
- `--symbols` — mostra a tabela de símbolos
- `--suggest` — ativa sugestões de correção léxica e sintática
- `--stop-first-error` — para no primeiro erro

---

### `Makefile`

- Headers atualizados: inclui `ast.h`, `parser.h`, `symbol_table.h`, `semantic_analyzer.h`
- Target `test` roda os programas v2 com `--ast --symbols`
- Target `clean` remove pasta `build/`

---

## Programas de Teste Adicionados

| Arquivo                       | Descrição                                                  |
| ----------------------------- | ---------------------------------------------------------- |
| `prog-factorial-v2.ling`      | Factorial adaptado para nova gramática (chaves em if/else) |
| `prog-bubblesort-v2.ling`     | BubbleSort adaptado para nova gramática                    |
| `prog-semantic-error-v2.ling` | 4 erros semânticos intencionais para validar o analisador  |
| `prog-erro-lexico.ling`       | Erros léxicos para testar `--suggest`                      |
