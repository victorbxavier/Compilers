# Checklist do Projeto — Compilador MiniJava (Unidade 2)

> Baseado no documento `Trabalho_2_Compiladores-2.pdf` e na gramática disponível em https://valdigleis.site

---

## Fase 1: Adaptar para a Nova Gramática do Professor

A gramática nova (disponível em https://valdigleis.site) já está em LL(1): sem recursão à esquerda, sem compartilhamento de prefixos, e com hierarquia de precedência de operadores.

### Nova gramática de expressões (hierarquia de precedência):
```
Exp       → Andexp
Andexp    → Relexp Andexp'
Andexp'   → '&&' Relexp Andexp' | λ
Relexp    → Addexp Relexp'
Relexp'   → '<' Addexp Relexp' | λ
Addexp    → Mulexp Addexp'
Addexp'   → '+' Mulexp Addexp' | '-' Mulexp Addexp' | λ
Mulexp    → Unexp Mulexp'
Mulexp'   → '*' Unexp Mulexp' | λ
Unexp     → '!' Unexp | Psfexp
Psfexp    → Priexp Psfexp'
Psfexp'   → '[' Exp ']' Psfexp' | '.' 'length' Psfexp' | '.' Id '(' Lexp ')' Psfexp' | λ
Priexp    → '(' Exp ')' | 'true' | 'false' | Id | Number | 'this' | 'new' Id '(' ')' | 'new' 'int' '[' Exp ']'
Lexp      → Exp Lexp' | λ
Lexp'     → ',' Exp Lexp' | λ
```

### Outras mudanças na gramática:
```
Lcom      → Com Lcom'
Lcom'     → Com Lcom' | λ
Com       → Id ComAss | 'if' '(' Exp ')' '{' Lcom '}' I | 'while' '(' Exp ')' '{' Lcom '}' | 'System.out.println' '(' Exp ')' ';'
ComAss    → '=' Exp ';' | '[' Exp ']' '=' Exp ';'
I         → 'else' '{' Lcom '}' | λ

DefM      → 'public' Type Id '(' DefM' | λ
DefM'     → Args ')' '{' DefV Lcom 'return' Exp ';' '}' DefM | ')' '{' DefV Lcom 'return' Exp ';' '}' DefM

Type      → 'int' Type' | 'boolean' | Id
Type'     → '[' ']' | λ
```

### Tarefas:
- [x] **1.1** Reescrever o parser com as novas produções de expressões (Andexp, Relexp, Addexp, Mulexp, Unexp, Psfexp, Priexp)
- [x] **1.2** Adaptar as regras de comandos: `Lcom`/`Lcom'` em vez de `CmdList`, `Com` em vez de `Cmd`, `ComAss` em vez de `CmdTail`
- [x] **1.3** Adaptar o `if`: agora é `if (Exp) { Lcom } I` onde `I → else { Lcom } | λ` (else é opcional)
- [x] **1.4** Adaptar `DefM'`: fatoração já feita na gramática (Args vs sem Args)
- [x] **1.5** Adaptar `Type` com `Type'` (para diferenciar `int` de `int[]`)
- [x] **1.6** Verificar que os programas de teste existentes continuam funcionando

## Fase 2: Melhorias no Analisador Léxico

- [x] **2.1** Reportar localização exata do erro léxico: **linha e coluna** (atualmente só reporta linha)
  - [x] Adicionar contador de coluna no lexer (reseta a cada `\n`)
- [x] **2.2** Implementar comportamento de parada configurável por flag
  - [x] Com flag `--stop-first-error` → para no primeiro erro e imprime apenas ele
  - [x] Sem a flag → processa toda a entrada e imprime todos os erros léxicos encontrados
- [x] **2.3** O lexer já remove comentários sem pré-processador separado (✅ já feito)

## Fase 3: Flags de Linha de Comando

- [x] **3.1** Flag para printar a lista de tokens (✅ já existe `--tokens`)
- [x] **3.2** Flag para parar no primeiro erro léxico (`--stop-first-error`)
- [x] **3.3** Flag para printar a AST (`--ast`)
- [x] **3.4** Flag para ativar sugestões de correção léxica e sintática (`--suggest`)
- [x] **3.5** Flag para printar a tabela de símbolos após análise sintática (`--symbols`)
- [x] **3.6** Documentar todas as flags no `--help`

## Fase 4: Geração da Árvore Sintática Abstrata (AST)

- [x] **4.1** Definir a hierarquia completa de nós da AST
  - [x] Nó base: `ASTNode`
  - [x] **Programa**: `Program` (MainClass + lista de ClassDecl)
  - [x] **Classes**: `MainClass`, `ClassDecl` (com ou sem extends)
  - [x] **Métodos**: `MethodDecl` (tipo retorno, nome, parâmetros, variáveis locais, comandos, expressão de retorno)
  - [x] **Variáveis**: `VarDecl` (tipo, nome)
  - [x] **Tipos**: `IntType`, `BoolType`, `IntArrayType`, `IdentifierType`
  - [x] **Comandos (Stmt)**: `BlockStmt`, `IfStmt`, `WhileStmt`, `PrintStmt`, `AssignStmt`, `ArrayAssignStmt`
  - [x] **Expressões (Exp)**: `AndExp`, `LessThanExp`, `PlusExp`, `MinusExp`, `TimesExp`, `ArrayLookupExp`, `ArrayLengthExp`, `MethodCallExp`, `IntLiteralExp`, `TrueLiteralExp`, `FalseLiteralExp`, `IdentifierExp`, `ThisExp`, `NewArrayExp`, `NewObjectExp`, `NotExp`
- [x] **4.2** Remover/substituir o `ast.h` atual (incompleto e com erros de compilação)
- [x] **4.3** Modificar o parser para construir a AST durante a análise sintática
  - [x] Cada função `parse*()` retorna um `unique_ptr` para o nó correspondente
  - [x] A precedência dos operadores é refletida na profundidade da árvore
- [x] **4.4** Implementar impressão da AST no terminal
  - [x] Formato indentado textual via método `print()` recursivo em cada nó
  - [x] Ativada pela flag `--ast`

## Fase 5: Tabela de Símbolos (Melhorias)

- [x] **5.1** Garantir que a tabela é atualizada durante o parsing com tipo e escopo
- [x] **5.2** Suportar busca hierárquica por escopo (método → classe → classe pai)
- [x] **5.3** Registrar informações de herança (classe pai) na tabela
- [x] **5.4** Separar a impressão da tabela com a flag `--symbols`

## Fase 6: Análise Semântica

### 6.1 — Semântica dos Tipos (Seção 4.1 do trabalho)
- [x] **6.1.1** `-`, `+`, `*` recebem dois `int` e retornam `int`
- [x] **6.1.2** `<` recebe dois `int` e retorna `boolean`
- [x] **6.1.3** `&&` recebe dois `boolean` e retorna `boolean`
- [x] **6.1.4** `!` recebe um `boolean` e retorna `boolean`
- [x] **6.1.5** `[]` indexa um `int[]` com índice `int`
- [x] **6.1.6** `.length` é aplicado a `int[]` e retorna `int`
- [x] **6.1.7** Atribuição exige compatibilidade de tipos

### 6.2 — Semântica das Classes e Herança (Seção 4.2)
- [x] **6.2.1** Classe vazia (sem atributos e sem métodos) = **erro semântico**
- [x] **6.2.2** Herança de campos e métodos
- [ ] **6.2.3** Despacho dinâmico (override de métodos)
- [x] **6.2.4** `new NomeClasse()` → verificar que a classe existe

### 6.3 — Semântica das Variáveis (Seção 4.3)
- [x] **6.3.1** Resolução de nomes: parâmetros do método → atributos da classe → atributos herdados
- [x] **6.3.2** Variável usada deve estar declarada
- [ ] **6.3.3** Sem variáveis duplicadas no mesmo escopo

### 6.4 — Semântica dos Comandos (Seção 4.4)
- [x] **6.4.1** `x = E`: tipo de E compatível com tipo de x
- [x] **6.4.2** `a[i] = E`: `a` é `int[]`, `i` é `int`, `E` é `int`
- [x] **6.4.3** Condição do `if`/`while` deve ser `boolean`
- [x] **6.4.4** `System.out.println(E)`: E deve ser `int`

### 6.5 — Semântica das Expressões (Seção 4.5)
- [x] **6.5.1** Precedência pela profundidade na AST (garantido pela gramática)
- [x] **6.5.2** Avaliação esquerda para direita

### 6.6 — Semântica de Objetos e Métodos (Seções 4.6 e 4.6.1)
- [x] **6.6.1** `new NomeClasse()`: classe deve existir
- [x] **6.6.2** Chamada `obj.f(x,y)`: método deve existir na classe (ou herdado)
- [x] **6.6.3** Número e tipos dos argumentos devem bater com os parâmetros formais
- [x] **6.6.4** `this` referencia o objeto atual corretamente
- [x] **6.6.5** Tipo de retorno do método deve ser consistente

## Fase 7: Programas de Teste

- [x] **7.1** Criar 2 programas corretos baseados na nova gramática (`prog-factorial-v2.ling`, `prog-bubblesort-v2.ling`)
- [x] **7.2** Criar 1 programa com erros semânticos (`prog-semantic-error-v2.ling`)
- [ ] **7.3** Criar 1 programa com erro sintático para nova gramática
- [ ] **7.4** Testar todos com o compilador e validar saídas

## Fase 8: Integração Final e Entrega

- [x] **8.1** Pipeline: `.ling` → Lexer → Parser (gera AST + Tabela) → Análise Semântica → Saída
- [x] **8.2** Testar com programas existentes em `assets/`
- [x] **8.3** Flags funcionando corretamente
- [ ] **8.4** Preparar para correção (24–28 de junho): 5 códigos do professor, cada um vale 2.0 pontos
- [ ] **8.5** Entregar: compilador + 4 códigos de teste

---

## Resumo: O que muda vs. Unidade 1

| Aspecto | Unidade 1 | Unidade 2 |
|---|---|---|
| Gramática | Sem precedência, uma regra `Exp` genérica | Hierarquia: Andexp > Relexp > Addexp > Mulexp > Unexp > Psfexp > Priexp |
| `if` | `if (Exp) Cmd else Cmd` (else obrigatório) | `if (Exp) { Lcom } I` onde `I → else { Lcom } \| λ` (else opcional) |
| Corpo de if/while | Cmd simples ou bloco | Sempre `{ Lcom }` (chaves obrigatórias) |
| Erros léxicos | Linha | Linha + coluna |
| Parada em erro | Sempre para no primeiro | Configurável por flag |
| Saída do parser | "correto/erro" | Gera AST |
| AST | Não existia | Gerada e impressa via flag |
| Análise semântica | Não existia | Verificação de tipos, escopo, herança |
| Flags CLI | `--tokens`, `--help` | + `--ast`, `--symbols`, `--suggest`, `--stop-first-error` |
| Testes | Pré-existentes | Criar 4 novos (2 corretos, 2 com erro) |
