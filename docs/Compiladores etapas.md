- [ ] Email para confirmar a realização do
- [ ] Terminar o lexer
	- [ ] Transformar o código de C para C++
	- [ ] Gerar uma lista de Tokens
- [ ] Terminar o parser
	- [ ] Funções de variaveis
	- [ ] Tabela de simbolos
	- [ ] Grammar Transformation
	- [ ] Consumir as gramáticas
Divisão de tarefas
Here's a clean way to divide the project among 5 members:

---

## 👥 Team Division (5 members)

### João Guilherme — Technical Report + Parser (30%)

- Write the **technical report** (data structures used, parsing technique, design decisions)
- **Output:** populated symbol table + report document

---

### Victor — Lexical Analyzer (Lexer)

- Tokenize the string from the pre-processor
- Recognize keywords (`class`, `if`, `while`, `new`, `true`, `false`, etc.), identifiers, numbers, operators, punctuation
- Handle lexical errors with suggestions for identifiers
- **Output:** list of tokens

---

### Davi — Grammar Transformation

- Remove **left recursion** from the grammar (especially `Exp`, `ListExp`, `Word`)
- Apply **left factoring** wherever needed
- Document all new variables (γ rules) introduced
- **Output:** transformed grammar ready for parser implementation

---

### Juscelino (Todos) — Syntactic Analyzer (Parser)

- Implement a **Recursive Descent** or **Predictive (LL(1))** parser
- Based on the transformed grammar from Member 3
- Detect and report syntax errors with location and type
- **Output:** success message or precise error info

---

### Thiago — Symbol Table

- Design and implement the **symbol table** (fields: name, type, scope, line, etc.)
- Integrate it with the lexer/parser output
- Write the **technical report** (data structures used, parsing technique, design decisions)
- **Output:** populated symbol table + report document

---

## 🔗 Integration Flow

```
Member 1        Member 2        Member 4
Pre-processor → Lexer → Tokens → Parser → ✅ / ❌
                  ↕                 ↕
              Member 3          Member 5
           Grammar Transform   Symbol Table + Report
```

---

## 💡 Tips

- Members 3 and 4 should work **closely together** since the parser depends directly on the transformed grammar
- Member 5 should start the symbol table design **early** so Members 2 and 4 know what data to feed into it
- Member 1 is relatively independent and can finish early to help others

# The grammar is already in LL or we must transform?

Looking at the grammar, you **must transform it**. It has two problems:

---

## ❌ Problem 1 — Left Recursion

The `Exp` rule is heavily left-recursive:

```
Exp → Exp '&&' Exp
    | Exp '>' Exp
    | Exp '+' Exp
    | Exp '-' Exp
    | Exp '*' Exp
    | Exp '[' Exp ']'
    | Exp '.' 'length'
    | Exp '.' Exp '(' ListExp ')'
    ...
```

Every rule starting with `Exp → Exp ...` is **direct left recursion** — this will cause infinite loops in a recursive descent parser.

---

## ❌ Problem 2 — Left Factoring Needed

`DefMet` has two alternatives that start the same way:

```
DefMet → 'public' Type Id '(' Args ')' '{' ...
       | 'public' Type Id '(' ')' '{' ...
```

Both start with `'public' Type Id '('` — the parser can't decide which rule to use without factoring.

Same issue in `DefCl`:

```
DefCl → 'class' Id '{' DefVar DefMet '}' DefCl
       | 'class' Id 'extends' Id '{' ...
```

Both start with `'class' Id`.

---

## ✅ What Needs to Be Done

|Rule|Problem|Action|
|---|---|---|
|`Exp`|Left recursion|Rewrite with `Exp'` tail rule|
|`DefMet`|Common prefix `'public' Type Id '('`|Left factor|
|`DefCl`|Common prefix `'class' Id`|Left factor|
|`Word`|Left recursion (`Number Word`, `Char Word`)|Check & fix|

---

## Quick Example — How to Fix `Exp`

```
Exp  → atom Exp'

Exp' → '&&' Exp Exp'
     | '>' Exp Exp'
     | '+' Exp Exp'
     | '-' Exp Exp'
     | '*' Exp Exp'
     | '[' Exp ']' Exp'
     | '.' 'length' Exp'
     | '.' Exp '(' ListExp ')' Exp'
     | λ

atom → '!' Exp
     | '(' Exp ')'
     | 'true' | 'false' | Id | Number | 'this'
     | 'new' Id '(' ')'
     | 'new' 'int' '[' Exp ']'
```

---

So **Member 3's job is critical** — the parser (Member 4) cannot start until the grammar transformation is done and reviewed.

# What do you mean with left factor?

Great question! Left factoring is a transformation you apply when **two or more rules share a common prefix**, and the parser can't decide which one to use just by looking at the next token.

---

## 🤔 The Problem

Imagine you're the parser and you see the token `class`. You look at `DefCl`:

```
DefCl → 'class' Id '{' DefVar DefMet '}' DefCl
       | 'class' Id 'extends' Id '{' DefVar DefMet '}' DefCl
       | λ
```

Both alternatives start with `'class' Id`. The parser reads `class`, then reads the `Id`... and then **doesn't know which rule to follow**. Should it expect `'{'` or `'extends'`? It's stuck.

---

## ✅ The Fix — Left Factoring

You **factor out the common prefix** into a single rule, and create a new variable for the "diverging" part:

```
DefCl → 'class' Id DefCl'
       | λ

DefCl' → '{' DefVar DefMet '}' DefCl
        | 'extends' Id '{' DefVar DefMet '}' DefCl
```

Now the parser reads `class`, then `Id`, then looks at the **next token**:

- If it sees `{` → follows first rule
- If it sees `extends` → follows second rule
- ✅ No ambiguity!

---

## 🧠 Simple Mental Model

Think of it like a **decision tree**:

```
Before factoring:               After factoring:

DefCl → class Id { ...          DefCl → class Id DefCl'
DefCl → class Id extends ...
                                DefCl' → { ...        ← sees '{'
                                         | extends ... ← sees 'extends'
```

You're just **delaying the decision** until you have enough information to choose.

---

## The Same Problem in `DefMet`

```
DefMet → 'public' Type Id '(' Args ')' ...
        | 'public' Type Id '(' ')' ...
```

Common prefix: `'public' Type Id '('`. After factoring:

```
DefMet → 'public' Type Id '(' DefMet'  ...
        | λ

DefMet' → Args ')' '{' DefVar Cmd 'return' Exp ';' '}' DefMet
         | ')' '{' DefVar Cmd 'return' Exp ';' '}' DefMet
```

Now the parser sees `(` and then looks ahead:

- If next token is a **Type** → there are args
- If next token is `)` → no args

---

## Summary

|Situation|Solution|
|---|---|
|Two rules start with the same tokens|Extract the common prefix|
|Create a new variable for the rest|Name it `A'` or `A_tail`|
|The new variable covers the diverging parts|Including `λ` if needed|

It's essentially making the grammar **unambiguous for a top-down parser** by ensuring every decision can be made with just one token of lookahead.