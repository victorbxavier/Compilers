# Fluxo de Compilação — Unidade 3 (Geração de Código Intermediário)

## Visão Geral do Pipeline

```
┌─────────────┐   ┌──────────┐   ┌──────────┐   ┌─────────┐   ┌──────────┐   ┌──────────────┐
│ Código Fonte│──▶│  Lexer   │──▶│  Parser  │──▶│   AST   │──▶│ Análise  │──▶│  Geração de  │
│ (.ling)     │   │  (Flex)  │   │(Rec.Desc)│   │(árvore) │   │Semântica │   │  Código (3AC)│
└─────────────┘   └──────────┘   └──────────┘   └─────────┘   └──────────┘   └──────────────┘
                       │               │              │              │                │
                  Lista de        Constrói AST    Programa em    Valida tipos    Percorre AST,
                  Tokens          + Tab.Símbolos  memória        e escopo        gera instruções
```

O compilador agora executa **5 etapas sequenciais**:

1. **Análise Léxica** — texto → tokens
2. **Análise Sintática** — tokens → AST + tabela de símbolos
3. **Análise Semântica** — percorre AST, verifica tipos e escopo
4. **Geração de Código** — percorre AST, gera instruções 3AC
5. **Saída** — imprime código intermediário

---

## O Que Mudou da Unidade 2 para a Unidade 3

### Unidade 2: Front-end completo (validação)

```
.ling → Lexer → Parser (AST) → Análise Semântica → "ok/erros"
```

### Unidade 3: + Back-end (geração de código)

```
.ling → Lexer → Parser (AST) → Análise Semântica → Geração 3AC → Código Intermediário
```

A grande novidade: o compilador agora **produz saída executável** (em forma de código intermediário), não apenas valida.

---

## O Que é o Código de Três Endereços (3AC)

É uma representação intermediária linear onde cada instrução faz **uma única operação** sobre **no máximo 3 endereços**:

```
resultado = operação operando1 operando2
```

Exemplos:

```
t1 = add x y       // t1 recebe x + y
t2 = lt t1 10      // t2 recebe t1 < 10 (boolean)
ifFalse t2 goto L1  // se t2 é false, pula para L1
num_aux = t3        // cópia simples
```

### Por que usar 3AC?

- **Desacoplamento**: separa a lógica do programa (AST) do formato da máquina alvo
- **Otimizável**: é mais fácil otimizar instruções lineares do que a AST
- **Portável**: o mesmo 3AC pode ser traduzido para x86, ARM, TAM, etc.

---

## Componentes Novos

### Temporários (`t0`, `t1`, `t2`, ...)

Variáveis criadas pelo compilador para guardar resultados intermediários.

Expressão `num * (this.ComputeFac(num - 1))` vira:

```
t6 = 1
t7 = sub num t6         // num - 1
param this
param t7
t8 = call ComputeFac, 2  // this.ComputeFac(num-1)
t9 = mul num t8          // num * resultado
```

### Labels (`L0`, `L1`, `L2`, ...)

Rótulos que marcam pontos no código para desvios (goto/if).

```
L0:                      // início do while
t14 = lt aux02 i         // condição
ifFalse t14 goto L1      // se falso, sai do while
... corpo do loop ...
goto L0                  // volta pro início
L1:                      // fim do while
```

---

## Templates de Geração

### Expressão binária: `a + b`

```
(gera código de a → resultado em left)
(gera código de b → resultado em right)
temp = add left right
```

### If/Else:

```
(gera código da condição → cond)
ifFalse cond goto Lelse
(gera código do bloco then)
goto Lfim
Lelse:
(gera código do bloco else)
Lfim:
```

### While:

```
Linicio:
(gera código da condição → cond)
ifFalse cond goto Lfim
(gera código do corpo)
goto Linicio
Lfim:
```

### Chamada de método: `obj.method(arg1, arg2)`

```
(gera código de obj → t_obj)
param t_obj
(gera código de arg1 → t_arg1)
param t_arg1
(gera código de arg2 → t_arg2)
param t_arg2
temp = call method, 3
```

### Atribuição em array: `a[i] = E`

```
(gera código de i → t_idx)
(gera código de E → t_val)
a[t_idx] = t_val
```

---

## Exemplo Completo: Factorial

### Código fonte:

```java
class Factorial {
    public static void main(String[] a) {
        System.out.println(new Fac().ComputeFac(10));
    }
}
class Fac {
    public int ComputeFac(int num) {
        int num_aux;
        if (num < 1) { num_aux = 1; }
        else { num_aux = num * (this.ComputeFac(num - 1)); }
        return num_aux;
    }
}
```

### Código 3AC gerado:

```
main:
    t0 = new Fac
    param t0
    t1 = 10
    param t1
    t2 = call ComputeFac, 2
    print t2
    halt
Fac.ComputeFac:
    t3 = 1
    t4 = lt num t3
    ifFalse t4 goto L0
    t5 = 1
    num_aux = t5
    goto L1
L0:
    param this
    t6 = 1
    t7 = sub num t6
    param t7
    t8 = call ComputeFac, 2
    t9 = mul num t8
    num_aux = t9
L1:
    return num_aux
```

---

## Fluxo no `main.cpp` (Unidade 3)

```
1.  Parseia argumentos (--tokens, --ast, --symbols, --no-ir, --help)
2.  Análise léxica → tokenList
3.  Análise sintática → AST + tabela de símbolos
4.  Análise semântica → verifica tipos/escopo
5.  Geração de código → CodeGenerator recebe AST + tabela no construtor, generate() produz lista de instruções 3AC
6.  Imprime código intermediário (a menos que --no-ir)
7.  "Compilação concluída com sucesso"
```

---

## Relação entre Arquivos (Unidade 3)

```
token.h ◄─────────────────────────────────────────────────────┐
   ▲                                                           │
   ├── lexer.l                                                 │
   ├── ast.h ◄────────────────────────────────┐                │
   ├── parser.h (constrói AST + tabela)       │                │
   │       └── symbol_table.h                 │                │
   ├── semantic_analyzer.h (percorre AST)     │                │
   │       └── ast.h + symbol_table.h         │                │
   ├── three_address_code.h (estrutura 3AC)   │                │
   ├── codegen.h (percorre AST, gera 3AC) ────┘                │
   │       └── three_address_code.h + ast.h + symbol_table.h   │
   └── main.cpp (orquestra tudo) ──────────────────────────────┘
```

---

## Resumo Rápido

| Arquivo                | Etapa        | Entrada       | Saída                       |
| ---------------------- | ------------ | ------------- | --------------------------- |
| `token.h`              | —            | —             | Definições de tokens        |
| `lexer.l`              | Léxica       | Chars         | Tokens                      |
| `ast.h`                | —            | —             | Definição dos nós da AST    |
| `parser.h`             | Sintática    | Tokens        | AST + Tabela                |
| `symbol_table.h`       | —            | —             | Registro de declarações     |
| `semantic_analyzer.h`  | Semântica    | AST + Tabela  | Erros semânticos            |
| `three_address_code.h` | —            | —             | Estrutura de instruções 3AC |
| `codegen.h`            | Geração      | AST + Tabela  | Lista de instruções 3AC     |
| `main.cpp`             | Orquestração | CLI + arquivo | Pipeline completo           |

---

## Comparação: Unidade 1 vs 2 vs 3

```
UNIDADE 1:  .ling → Lexer → Parser (valida)              → "ok/erro"
UNIDADE 2:  .ling → Lexer → Parser (AST) → Semântica     → "ok/erros semânticos"
UNIDADE 3:  .ling → Lexer → Parser (AST) → Semântica → CodeGen → Código Intermediário (3AC)
```
