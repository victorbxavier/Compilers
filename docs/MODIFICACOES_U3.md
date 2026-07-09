# Modificações Realizadas — Unidade 3

## Resumo Geral

O compilador foi estendido com um **back-end de geração de código intermediário** (Código de Três Endereços — 3AC). Agora, após validar o programa (léxico + sintático + semântico), o compilador **percorre a AST e produz instruções** que descrevem o comportamento do programa de forma genérica.

```
ANTES (Unidade 2):  .ling → Lexer → Parser (AST) → Semântica → "ok/erros"
AGORA (Unidade 3):  .ling → Lexer → Parser (AST) → Semântica → CodeGen → Código 3AC
```

---

## Arquivos Criados

### `src/three_address_code.h` (NOVO)

**O que é:** Módulo que define a estrutura de representação do código de três endereços.

**Conteúdo:**

- `enum class OpCode` — 22 operações suportadas:
  - Aritméticas: `ADD`, `SUB`, `MUL`
  - Relacionais/lógicas: `LT`, `AND`, `NOT`
  - Cópia: `COPY`
  - Controle de fluxo: `LABEL`, `GOTO`, `IF_TRUE`, `IF_FALSE`
  - Funções: `PARAM`, `CALL`, `RETURN`
  - I/O: `PRINT`
  - Arrays: `NEW_ARRAY`, `ARRAY_LOAD`, `ARRAY_STORE`, `ARRAY_LENGTH`
  - Objetos: `NEW_OBJ`
  - Programa: `HALT`, `NOP`

- `struct Instruction` — uma instrução com: `op`, `result`, `arg1`, `arg2`

- `class TAC` — lista de instruções com:
  - `emit(op, result, arg1, arg2)` — adiciona instrução
  - `concat(other)` — concatena listas
  - `newTemp()` — gera temporários únicos (t0, t1, t2, ...)
  - `newLabel()` — gera labels únicos (L0, L1, L2, ...)
  - `print()` — imprime todas as instruções formatadas com numeração

---

### `src/codegen.h` (NOVO)

**O que é:** Gerador de código que percorre a AST recursivamente e produz instruções 3AC.

**Classe `CodeGenerator`:**

- Recebe a tabela de símbolos e a AST no construtor
- Método principal: `generate()` → retorna objeto `TAC`
- Percorre em **pós-ordem**: primeiro gera código dos filhos, depois do nó atual

**Geração por tipo de nó:**

| Nó AST                        | Instruções 3AC geradas                                             |
| ----------------------------- | ------------------------------------------------------------------ |
| `PlusExp(a, b)`               | `t = add left right`                                               |
| `MinusExp(a, b)`              | `t = sub left right`                                               |
| `TimesExp(a, b)`              | `t = mul left right`                                               |
| `LessThanExp(a, b)`           | `t = lt left right`                                                |
| `AndExp(a, b)`                | `t = and left right`                                               |
| `NotExp(a)`                   | `t = not val`                                                      |
| `IntLiteralExp(n)`            | `t = n`                                                            |
| `TrueLiteralExp`              | `t = 1`                                                            |
| `FalseLiteralExp`             | `t = 0`                                                            |
| `IdentifierExp(x)`            | retorna `x` (sem instrução)                                        |
| `ThisExp`                     | retorna `this`                                                     |
| `NewObjectExp(C)`             | `t = new C`                                                        |
| `NewArrayExp(size)`           | `t = new int[size]`                                                |
| `ArrayLookupExp(a, i)`        | `t = a[i]`                                                         |
| `ArrayLengthExp(a)`           | `t = length a`                                                     |
| `MethodCallExp(obj, m, args)` | `param obj`, `param args...`, `t = call m, N`                      |
| `AssignStmt(x, E)`            | `x = tempE`                                                        |
| `ArrayAssignStmt(a, i, E)`    | `a[i] = tempE`                                                     |
| `PrintStmt(E)`                | `print tempE`                                                      |
| `IfStmt(c, then, else)`       | `ifFalse c goto Lelse`, then, `goto Lend`, `Lelse:`, else, `Lend:` |
| `WhileStmt(c, body)`          | `Lstart:`, `ifFalse c goto Lend`, body, `goto Lstart`, `Lend:`     |
| `MethodDecl`                  | `label Classe.Método:`, body, `return retVal`                      |

---

## Arquivos Modificados

### `src/main.cpp`

- **Adicionada** flag `--no-ir` que oculta o código intermediário (exibido por padrão)
- **Adicionada** etapa 4 no pipeline: geração de código após análise semântica
- **Incluído** `codegen.h`
- **Atualizado** `printUsage()` com a nova flag

### `Makefile`

- **Adicionados** `three_address_code.h` e `codegen.h` às dependências de headers
- **Atualizado** target `test` para usar `--ir`

---

## Como Usar

```bash
# Compilação completa (exibe 3AC por padrão)
./build/compiler assets/unidade-2/prog-factorial-v2.ling

# Com AST e tabela de símbolos também
./build/compiler --ast --symbols assets/unidade-2/prog-bubblesort-v2.ling

# Ocultar código intermediário
./build/compiler --no-ir assets/unidade-2/prog-factorial-v2.ling

# Pipeline completo sem flags de debug
./build/compiler assets/unidade-2/prog-factorial-v2.ling
```

---

## Exemplo de Saída

Para `prog-factorial-v2.ling`:

```
=== Código Intermediário (3AC) — 25 instruções ===
   0:  main:
   1:  t0 = new Fac
   2:  param t0
   3:  t1 = 10
   4:  param t1
   5:  t2 = call ComputeFac, 2
   6:  print t2
   7:  halt
   8:  Fac.ComputeFac:
   9:  t3 = 1
  10:  t4 = lt num t3
  11:  ifFalse t4 goto L0
  12:  t5 = 1
  13:  num_aux = t5
  14:  goto L1
  15:  L0:
  16:  param this
  17:  t6 = 1
  18:  t7 = sub num t6
  19:  param t7
  20:  t8 = call ComputeFac, 2
  21:  t9 = mul num t8
  22:  num_aux = t9
  23:  L1:
  24:  return num_aux
```

---

## O Que Ainda Falta (Itens opcionais/bônus)

- [ ] Tradução de 3AC para código TAM (Triangle Abstract Machine)
- [ ] Otimizações no código intermediário (eliminação de subexpressões comuns, propagação de constantes)
