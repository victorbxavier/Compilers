# Modificações Realizadas — Unidade 2

## Resumo Geral

O compilador foi reescrito para a Unidade 2 com as seguintes capacidades novas:
- Parser com gramática atualizada (precedência de operadores)
- Geração de AST (Árvore Sintática Abstrata)
- Impressão da AST no terminal
- Análise semântica completa (verificação de tipos, escopo, herança)
- Novas flags de linha de comando
- Erros léxicos agora reportam linha **e coluna**

---

## Arquivos Modificados

### `src/token.h`
- **Adicionado** campo `int column` ao struct `Token`
- **Alterado** `printToken()` para mostrar formato `linha:coluna` em vez de apenas `line N`

### `src/lexer.l`
- **Adicionado** contador de coluna `columnNumber` (reseta a cada `\n`, incrementa a cada caractere)
- **Adicionada** flag global `stopOnFirstError` — se ativa, o lexer retorna após o primeiro erro
- **Adicionada** flag global `showSuggestions` — controla se sugestões de Levenshtein são exibidas
- **Alterado** `addToken()` para receber e registrar a coluna
- **Alteradas** todas as regras para passar `columnNumber` e chamar `updateColumn()` após cada token
- **Alterados** mensagens de erro para exibir `linha:coluna`
- **Adicionada** opção `%option noinput`

### `src/parser.h`
- **Reescrito completamente** para a gramática nova do professor (valdigleis.site)
- **Retorno** mudou de `bool` para `unique_ptr<Program>` — agora constrói a AST
- **Novas funções de expressão** com hierarquia de precedência:
  - `parseAndexp()` — operador `&&` (menor precedência)
  - `parseRelexp()` — operador `<`
  - `parseAddexp()` — operadores `+`, `-`
  - `parseMulexp()` — operador `*`
  - `parseUnexp()` — operador `!`
  - `parsePsfexp()` — operadores pós-fixos: `[]`, `.length`, `.metodo()`
  - `parsePriexp()` — expressões primárias (maior precedência)
- **Novas funções de comandos**:
  - `parseLcom()` — lista de comandos (retorna `vector<unique_ptr<Stmt>>`)
  - `parseCom()` — comando individual (retorna `unique_ptr<Stmt>`)
  - `parseComAss()` — atribuição simples ou em array
- **Gramática do `if` alterada**: agora exige `{ }` e o `else` é opcional
- **Gramática do `while` alterada**: agora exige `{ }`
- **Métodos**: `parseDefM()` usa fatoração `DefM'` (com ou sem argumentos)
- Mensagens de erro agora incluem `linha:coluna`

### `src/symbol_table.h`
- **Adicionado** campo `parent` ao struct `Symbol` (para herança entre classes)
- **Adicionado** método `lookup(name, scope)` — busca hierárquica: local → classe → herdado
- **Adicionado** método `lookupClass(name)` — busca classe por nome
- **Adicionado** método `lookupMethod(className, methodName)` — busca método (com herança)
- **Adicionado** método `isDuplicate(name, scope, cat)` — detecta declarações duplicadas
- **Adicionado** método `getMethodParams(className, methodName)` — retorna parâmetros formais
- **Adicionado** método privado `lookupInherited()` — busca recursiva em classes pai

### `src/main.cpp`
- **Reescrito** para orquestrar o novo pipeline: Lexer → Parser (AST) → Análise Semântica
- **Novas flags**: `--ast`, `--symbols`, `--suggest`, `--stop-first-error`
- **Inclusão** do `semantic_analyzer.h`
- **Alterado** fluxo: `parser.parse()` agora retorna `unique_ptr<Program>` (a AST)
- **Adicionada** etapa de análise semântica após parsing bem-sucedido
- **Atualizado** `printUsage()` com todas as flags

### `Makefile`
- **Atualizado** variável `HEADERS` para incluir `ast.h`, `parser.h`, `symbol_table.h`, `semantic_analyzer.h`
- **Adicionado** target `test` que roda factorial-v2 e bubblesort-v2 com `--ast --symbols`
- **Removidos** targets antigos `test-factorial` e `test-bubblesort`

---

## Arquivos Criados

### `src/ast.h` (reescrito do zero)
Hierarquia completa de nós da AST:
- **Base**: `ASTNode` (com campo `line` e método virtual `print()`)
- **Tipos**: `IntType`, `BoolType`, `IntArrayType`, `IdentifierType`
- **Expressões** (18 nós): `AndExp`, `LessThanExp`, `PlusExp`, `MinusExp`, `TimesExp`, `NotExp`, `ArrayLookupExp`, `ArrayLengthExp`, `MethodCallExp`, `IntLiteralExp`, `TrueLiteralExp`, `FalseLiteralExp`, `IdentifierExp`, `ThisExp`, `NewObjectExp`, `NewArrayExp`
- **Comandos** (6 nós): `BlockStmt`, `IfStmt`, `WhileStmt`, `PrintStmt`, `AssignStmt`, `ArrayAssignStmt`
- **Declarações**: `VarDecl`, `Formal`, `MethodDecl`, `ClassDecl`, `MainClass`, `Program`
- **Utilitário**: `typeToString()` para converter nó Type em string

### `src/semantic_analyzer.h` (novo)
Módulo de análise semântica que percorre a AST:
- Verifica tipos de todos os operadores (`+`, `-`, `*`, `<`, `&&`, `!`)
- Verifica compatibilidade em atribuições
- Verifica condições de `if`/`while` (devem ser `boolean`)
- Verifica `System.out.println` (requer `int`)
- Verifica existência de variáveis (consulta tabela de símbolos)
- Verifica existência de classes em `new NomeClasse()`
- Verifica chamadas de método (existência, número/tipos de argumentos)
- Verifica tipo de retorno dos métodos
- Verifica classes vazias (erro semântico)
- Suporte a herança (subtipo é compatível com supertipo)

### `assets/prog-factorial-v2.ling`
Programa Factorial adaptado para a nova gramática (chaves obrigatórias em if/else).

### `assets/prog-bubblesort-v2.ling`
Programa BubbleSort adaptado para a nova gramática (chaves em if/else/while).

### `assets/prog-semantic-error-v2.ling`
Programa com 4 erros semânticos intencionais:
1. Atribuição `x = true` onde x é `int`
2. Condição `if (x + 1)` — int em vez de boolean
3. Atribuição `result = flag && true` onde result é `int`
4. `System.out.println(flag)` — flag é boolean, requer int

---

## O que ainda falta (itens não marcados no checklist)

- [ ] **6.2.3** Despacho dinâmico (override de métodos) — estrutura preparada mas não testada
- [ ] **6.3.3** Detecção de variáveis duplicadas no mesmo escopo — método `isDuplicate()` existe mas não é chamado
- [ ] **7.3** Criar 1 programa com erro sintático para nova gramática
- [ ] **7.4** Validação final de todos os testes
- [ ] **8.4** Preparação para correção (24–28 de junho)
- [ ] **8.5** Entrega final
