# Guia do Código de Três Endereços (3AC) — Referência e Decisões de Projeto

## O Que é o Código de Três Endereços (3AC)

O Código de Três Endereços é uma **representação intermediária** usada em compiladores entre a AST (que representa a estrutura do programa) e o código de máquina final (assembly). Cada instrução manipula **no máximo 3 endereços** (um destino e dois operandos) e realiza **uma única operação**.

O nome "três endereços" vem do formato geral:

```
resultado = operação operando1 operando2
   ↑              ↑        ↑         ↑
 endereço 1    opcode   endereço 2  endereço 3
```

Por exemplo, a expressão `a + b * c` no código fonte se torna:

```
t1 = mul b c      ← primeira operação (multiplicação)
t2 = add a t1     ← segunda operação (soma com resultado anterior)
```

O 3AC "explode" expressões complexas em passos atômicos, cada um com uma operação e seus endereços.

---

## Por Que Usar 3AC? (Motivação)

### O problema: a AST não é executável

A AST representa a **estrutura** do programa (árvore hierárquica), mas não descreve a **sequência de passos** que uma máquina precisa executar. Por exemplo, uma árvore `Plus(Times(b, c), a)` não diz explicitamente "primeiro calcule b*c, guarde num lugar, depois some com a".

### A solução: uma representação linear e explícita

O 3AC transforma a árvore numa **lista sequencial de instruções simples** que descrevem exatamente o que fazer e em que ordem. É o "passo intermediário" entre a linguagem do programador e a linguagem da máquina.

---

## Vantagens do 3AC

### 1. Desacoplamento (portabilidade)

O 3AC é **independente de máquina**. O mesmo código intermediário pode ser traduzido para x86, ARM, TAM ou qualquer outra arquitetura. Isso separa o compilador em:
- **Front-end** (léxico + sintático + semântico → 3AC): específico da linguagem fonte
- **Back-end** (3AC → assembly): específico da arquitetura alvo

Se quiser portar o compilador pra outra máquina, só reescreve o back-end.

### 2. Facilita otimizações

É muito mais fácil otimizar uma lista linear de instruções simples do que uma árvore aninhada. Exemplos de otimizações aplicáveis ao 3AC:
- **Eliminação de subexpressões comuns**: se `t1 = add x y` já existe, não precisa calcular de novo
- **Propagação de constantes**: se `t1 = 10` e depois `t2 = add t1 5`, substitui por `t2 = 15`
- **Eliminação de código morto**: instruções cujo resultado nunca é usado podem ser removidas

### 3. Simplicidade

Cada instrução faz **uma coisa só**. Não há expressões aninhadas, precedência de operadores ou ambiguidade. A tradução pra assembly fica quase mecânica: cada instrução 3AC vira 3-4 instruções de máquina seguindo templates fixos.

### 4. Abordagem genérica e didática

O 3AC prioriza clareza sobre eficiência. Pode gerar mais temporários do que o necessário, mas o código é correto e legível. Otimizações podem ser aplicadas depois, antes da geração do código de máquina final.

---

## Como o 3AC se Encaixa no Pipeline do Compilador

```
Código Fonte (.ling)
       │
       ▼ (Front-end)
    Lexer → Parser → AST → Análise Semântica
       │
       ▼ (Geração de código intermediário)
    Percorre a AST recursivamente → gera lista de instruções 3AC
       │
       ▼ (Back-end — futuro/opcional)
    3AC → Otimizações → Assembly da máquina alvo (TAM, x86, ARM...)
```

No nosso compilador, o 3AC é a **saída final**. Um compilador completo teria mais uma etapa traduzindo o 3AC pra código de máquina executável.

---

## Componentes do 3AC

### Endereços (o que pode aparecer nas instruções)

1. **Nomes do programa** — variáveis declaradas pelo programador (`total`, `i`, `num`)
2. **Constantes** — valores literais representados em temporários (`t3 = 10`)
3. **Temporários** — variáveis criadas pelo compilador (`t0`, `t1`, `t2`...) para guardar resultados intermediários

### Temporários (`t0`, `t1`, `t2`, ...)

São variáveis inventadas pelo compilador porque o 3AC só permite uma operação por instrução. O programador escreveu `num * (this.ComputeFac(num - 1))` como uma expressão só, mas o compilador precisa decompor em passos:

```
t6 = 1                        ← constante 1
t7 = sub num t6               ← num - 1
param this                    ← prepara this
param t7                      ← prepara argumento
t8 = call Fac.ComputeFac, 2   ← chama método, resultado em t8
t9 = mul num t8               ← num * resultado
```

### Labels (`L0`, `L1`, `L2`, ...)

Marcam pontos no código para onde instruções de desvio (`goto`, `ifFalse`) podem saltar. São usados para implementar:
- **if/else**: pula o bloco then ou else
- **while**: volta ao início do loop ou sai dele

### Diferença entre `call` e `goto`

| Instrução | Vai pra outro lugar? | Volta automaticamente? | Uso |
|-----------|---------------------|----------------------|-----|
| `goto L` | Sim | Não | Controle de fluxo (while, if) |
| `call F, N` | Sim | Sim (quando F faz `return`) | Chamada de método |

`call` é como ligar pra alguém — você fala, espera a resposta e continua. `goto` é como pegar outra estrada — não tem volta automática.

---

## O Que é Cada Instrução 3AC

### Operações Aritméticas

| Instrução | Significado | Exemplo no código fonte |
|-----------|-------------|------------------------|
| `t1 = add x y` | t1 recebe x + y | `x + y` |
| `t1 = sub x y` | t1 recebe x - y | `x - y` |
| `t1 = mul x y` | t1 recebe x * y | `x * y` |

Sempre geram um temporário novo com o resultado. Os operandos `x` e `y` podem ser variáveis do programa, constantes (em temporários) ou outros temporários.

### Operações Relacionais e Lógicas

| Instrução | Significado | Resultado |
|-----------|-------------|-----------|
| `t1 = lt x y` | t1 recebe x < y | 1 (true) ou 0 (false) |
| `t1 = and x y` | t1 recebe x && y | 1 ou 0 |
| `t1 = not x` | t1 recebe !x | 1 ou 0 |

O resultado é sempre 0 ou 1 (representação numérica de boolean).

### Cópia e Literais

| Instrução | Significado | Quando é gerado |
|-----------|-------------|-----------------|
| `x = t1` | Copia o valor de t1 para x | Atribuição `x = expressão;` |
| `t1 = 10` | Carrega constante 10 em t1 | Literal inteiro no código |
| `t1 = 1` | Carrega 1 (true) em t1 | Literal `true` |
| `t1 = 0` | Carrega 0 (false) em t1 | Literal `false` |

**Por que literais geram um temporário?** Porque no 3AC, operações como `add` precisam de endereços (nomes) nos operandos. O número `10` não tem endereço — precisa ser armazenado num temporário primeiro pra poder ser referenciado.

### Controle de Fluxo

| Instrução | Significado | Quando é usado |
|-----------|-------------|----------------|
| `L0:` | Define um rótulo (label) — marca um ponto no código | Início de while, bloco else, fim de if |
| `goto L0` | Salta incondicionalmente para L0 | Volta ao início do while, pula o else |
| `ifFalse t1 goto L0` | Se t1 é 0 (false), salta para L0 | Condição do if/while é falsa → pula bloco |
| `if t1 goto L0` | Se t1 é 1 (true), salta para L0 | (não usado atualmente, mas disponível) |

**Decisão de projeto:** Usamos `ifFalse` em vez de `ifTrue` porque o padrão natural é "se a condição falha, pule o bloco". Isso simplifica os templates — o código do bloco "then" fica logo após o teste, sem precisar inverter a lógica.

### Chamada de Funções

| Instrução | Significado | Quando é usado |
|-----------|-------------|----------------|
| `param t0` | Prepara t0 como argumento da próxima call | Antes de cada chamada de método |
| `t2 = call metodo, 2` | Chama "metodo" com 2 argumentos preparados, resultado em t2 | `obj.metodo(arg)` |
| `return t1` | Retorna o valor de t1 para o chamador | `return expressão;` |

**Como funciona a sequência param/call:**
```
param t0          ← 1º argumento (o objeto, será o "this" do método)
param t1          ← 2º argumento (argumento real passado)
t2 = call F, 2   ← chama F, dizendo que tem 2 params preparados
```

O número após a vírgula (2) diz quantos `param` foram emitidos antes.

### I/O

| Instrução | Significado | Quando é usado |
|-----------|-------------|----------------|
| `print t1` | Imprime o valor de t1 | `System.out.println(expressão)` |

### Arrays

| Instrução | Significado | Quando é usado |
|-----------|-------------|----------------|
| `t1 = new int[t0]` | Aloca array de tamanho t0, endereço em t1 | `new int[tamanho]` |
| `t1 = a[t0]` | Lê posição t0 do array a, valor em t1 | `a[i]` como expressão |
| `a[t0] = t1` | Escreve t1 na posição t0 do array a | `a[i] = valor;` |
| `t1 = length a` | Obtém tamanho do array a | `a.length` |

### Objetos

| Instrução | Significado | Quando é usado |
|-----------|-------------|----------------|
| `t0 = new Classe` | Aloca novo objeto da classe, endereço em t0 | `new NomeClasse()` |

### Programa

| Instrução | Significado | Quando é usado |
|-----------|-------------|----------------|
| `halt` | Encerra execução do programa | Fim do main |

---

## Decisões de Projeto

### 1. Labels de método: `Classe.Metodo`

**Decisão:** Nomeamos os labels dos métodos como `Classe.Metodo` (ex: `Fac.ComputeFac`).

**Justificativa:** Se duas classes tiverem métodos com o mesmo nome (ex: `Fac.compute` e `BBS.compute`), precisamos distinguir. Usando `Classe.Metodo` garantimos unicidade. Alternativas seriam numeração (`_func_0`, `_func_1`) mas perderíamos legibilidade.

### 2. Objeto como primeiro `param` em chamadas de método

**Decisão:** Em `obj.metodo(arg1, arg2)`, geramos `param obj` como primeiro argumento.

**Justificativa:** Em linguagens orientadas a objetos, todo método precisa saber em qual objeto está operando (o `this`). O padrão em compiladores é passar o objeto como primeiro parâmetro implícito. Assim `obj.metodo(arg)` vira conceitualmente `metodo(obj, arg)`.

### 3. Identificadores não geram instrução

**Decisão:** Quando uma expressão é simplesmente um identificador (variável), retornamos o nome dela sem gerar nenhuma instrução 3AC.

**Justificativa:** Seria desperdício gerar `t5 = x` toda vez que `x` aparece. O nome `x` já é um endereço válido no 3AC. Só geramos temporário quando há uma **operação** a ser feita.

### 4. `ifFalse` em vez de `ifTrue`

**Decisão:** Usamos `ifFalse condição goto L` (salta se falso).

**Justificativa:** O padrão natural é: se a condição é verdadeira, executa o bloco (que vem logo em seguida no código). Se é falsa, pula pro label. Isso evita inverter a lógica da condição e mantém o template simples:
```
(testa condição)
ifFalse → pula bloco
(bloco normal aqui)
```

### 5. Literais sempre em temporários

**Decisão:** `10` no código fonte vira `t3 = 10` no 3AC, nunca usamos o literal diretamente em operações.

**Justificativa:** Mantém a uniformidade — todo endereço no 3AC é um nome (variável ou temporário). Facilita a futura tradução pra TAM/assembly onde tudo precisa ser carregado de algum lugar. Também é o padrão dos slides do professor (na tabela de templates, constantes passam por um `LOADL` antes de serem usadas).

### 6. `print` como instrução simplificada

**Decisão:** Usamos `print t1` em vez de `param t1` + `call println, 1`.

**Justificativa:** `System.out.println` é uma operação de I/O da linguagem, não um método normal que o programa define. Tratamos como instrução especial pra manter o 3AC limpo. Alternativamente poderíamos ter usado o formato param/call, mas seria adicionar complexidade sem ganho didático.

### 7. Contadores globais para temporários e labels

**Decisão:** Temporários (t0, t1, t2...) e labels (L0, L1, L2...) usam contadores incrementais globais.

**Justificativa:** Garante unicidade sem esforço. Cada `newTemp()` retorna um nome que nunca foi usado antes. Não resetamos por método porque simplifica a implementação e não há conflito.

---

## Resumo: Fluxo Completo de Um Programa

Dado `System.out.println(new Fac().ComputeFac(10))`:

```
1. new Fac()           →  t0 = new Fac
2. O objeto é "this"   →  param t0
3. Argumento 10        →  t1 = 10  +  param t1
4. Chama o método      →  t2 = call ComputeFac, 2
5. Imprime resultado   →  print t2
6. Fim                 →  halt
```

Cada expressão do código fonte é decomposta em operações atômicas. O 3AC é a forma "explodida" e linearizada do programa — uma instrução por operação, um temporário por resultado intermediário.
