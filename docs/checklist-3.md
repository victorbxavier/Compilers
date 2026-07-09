# Checklist do Projeto — Compilador MiniJava (Unidade 3)

> Baseado no documento `Trabalho_3_Compiladores.pdf`, nos slides `Geracao_de_Codigo_2.pdf` e `Anlise_semntica_-_Gerao_de_Cdigo_Intermedirio.pdf`
> **Entrega e apresentação: até 10/07/2026**

---

## Fase 1: Estrutura do Código de Três Endereços (3AC)

O módulo 3AC é a representação intermediária entre a AST e o código de máquina (TAM).
Cada instrução tem: **operação + até 3 endereços** (destino, operando1, operando2).

- [x] **1.1** Criar `src/three_address_code.h` com a estrutura de uma instrução 3AC
  - [x] Enum de operações (OpCode): `ADD`, `SUB`, `MUL`, `LT`, `AND`, `NOT`, `COPY`, `GOTO`, `IF_TRUE`, `IF_FALSE`, `PARAM`, `CALL`, `RETURN`, `LABEL`, `PRINT`, `NEW_ARRAY`, `NEW_OBJ`, `ARRAY_LOAD`, `ARRAY_STORE`, `ARRAY_LENGTH`, `HALT`, `NOP`
  - [x] Struct `Instruction` com: opcode, result (destino), arg1, arg2
  - [x] Cada endereço (result/arg1/arg2) pode ser: nome de variável, constante numérica ou temporário — representado por string referenciando a tabela de símbolos
- [x] **1.2** Criar classe/struct `TAC` (Three Address Code) com:
  - [x] Lista de instruções (`vector<Instruction>`)
  - [x] Função `emit(opcode, result, arg1, arg2)` para adicionar instrução
  - [x] Função `print()` para imprimir o código intermediário gerado
  - [x] Função `concat(other)` para concatenar duas listas de instruções
- [x] **1.3** Implementar geração de temporários (`newTemp()`)
  - [x] Cria nomes únicos: `t0`, `t1`, `t2`, ...
  - [x] Registra na tabela de símbolos como variável temporária
- [x] **1.4** Implementar geração de labels (`newLabel()`)
  - [x] Cria rótulos únicos: `L0`, `L1`, `L2`, ...
  - [x] Usados para desvios condicionais/incondicionais (if, while, goto)

## Fase 2: Geração de Código (Percorrimento da AST → 3AC)

O módulo percorre a AST recursivamente (pós-ordem: primeiro os filhos, depois o nó pai) e gera instruções 3AC para cada nó.

- [x] **2.1** Criar `src/codegen.h` com classe `CodeGenerator`
  - [x] Recebe a AST e a tabela de símbolos
  - [x] Método principal: `generate(Program*)` → retorna lista de instruções 3AC
- [x] **2.2** Geração para expressões aritméticas e lógicas
  - [x] `a + b` → `t1 = add a b`
  - [x] `a - b` → `t1 = sub a b`
  - [x] `a * b` → `t1 = mul a b`
  - [x] `a < b` → `t1 = lt a b`
  - [x] `a && b` → `t1 = and a b`
  - [x] `!a` → `t1 = not a`
  - [x] Literais: `t1 = 10` (copy constante para temporário)
  - [x] Identificadores: retorna o próprio nome (sem gerar instrução)
- [x] **2.3** Geração para atribuições
  - [x] `x = E` → gera código de E (resultado em temp), depois `x = temp`
  - [x] `a[i] = E` → gera código de i e E, depois `a[i] = temp` (ARRAY_STORE)
- [x] **2.4** Geração para controle de fluxo (if)
  - [x] Template com ifFalse/goto/labels
  - [x] Se não tem else, simplifica (sem goto Lfim)
- [x] **2.5** Geração para repetição (while)
  - [x] Template com Linicio/ifFalse/goto
- [x] **2.6** Geração para `System.out.println(E)`
  - [x] Gera código de E, depois `print temp`
- [x] **2.7** Geração para chamada de métodos (`obj.method(args)`)
  - [x] Gera código para cada argumento → `param arg1`, `param arg2`, ...
  - [x] `t1 = call method, N` (N = número de argumentos)
- [x] **2.8** Geração para criação de objetos e arrays
  - [x] `new ClassName()` → `t1 = new ClassName`
  - [x] `new int[n]` → `t1 = new int[n]`
- [x] **2.9** Geração para acesso a arrays
  - [x] `a[i]` → `t1 = a[i]` (ARRAY_LOAD)
  - [x] `a.length` → `t1 = length a` (ARRAY_LENGTH)
- [x] **2.10** Geração para métodos e retorno
  - [x] Início do método: label Classe.Método
  - [x] `return E` → gera código de E, depois `return temp`
- [x] **2.11** Geração para `this`
  - [x] Retorna referência ao objeto atual

## Fase 3: Impressão do Código Intermediário

- [x] **3.1** Implementar impressão formatada do 3AC
  - [x] Formato legível: `t1 = add x y`, `goto L3`, `ifFalse t2 goto L1`, etc.
  - [x] Imprimir labels como `L0:`, `L1:`, etc.
- [x] **3.2** Adicionar flag `--ir` (intermediate representation) ao compilador
  - [x] Imprime o código de três endereços gerado
- [x] **3.3** Numerar as instruções (opcional, ajuda na depuração)

## Fase 4: Tradução de 3AC para TAM (Opcional/Bônus)

A máquina TAM (Triangle Abstract Machine) é baseada em pilha. A tradução segue templates fixos.

- [ ] **4.1** Criar `src/tam_codegen.h` (se requerido)
- [ ] **4.2** Traduzir operações aritméticas: LOAD operandos → CALL primitiva → STORE resultado
- [ ] **4.3** Traduzir desvios: JUMP, JUMPIF
- [ ] **4.4** Traduzir chamadas de função: CALL, RETURN
- [ ] **4.5** Impressão do código TAM

## Fase 5: Integração e Testes

- [x] **5.1** Integrar CodeGenerator no pipeline do `main.cpp` (após análise semântica)
- [x] **5.2** Testar com `prog-factorial-v2.ling` — verificar código intermediário correto
- [x] **5.3** Testar com `prog-bubblesort-v2.ling` — arrays, loops aninhados
- [ ] **5.4** Criar programas de teste específicos para:
  - [ ] Expressões complexas (precedência, aninhamento)
  - [ ] if/else aninhados
  - [ ] while com breaks lógicos
  - [ ] Chamadas de método com múltiplos argumentos
  - [ ] Criação de objetos e acesso a arrays
- [ ] **5.5** Validar que o código gerado é fiel à semântica do programa fonte

## Fase 6: Apresentação

- [ ] **6.1** Preparar demonstração do funcionamento
- [ ] **6.2** Explicar decisões de projeto (estrutura do 3AC, como temporários são geridos, como labels são usados)
- [ ] **6.3** Mostrar exemplos de programa fonte → AST → 3AC gerado
- [ ] **6.4** Apresentar até 10/07/2026

---

## Pipeline Completo (Unidade 3)

```
.ling → Lexer → Parser (AST + Tabela) → Análise Semântica → Geração de Código (3AC) → Saída
                                                                      │
                                                              Percorre a AST
                                                              recursivamente,
                                                              gera instruções
                                                              3AC para cada nó
```

## Critérios de Avaliação

1. **Qualidade do código intermediário** — correção, organização e fidelidade das instruções 3AC
2. **Apresentação** — clareza, demonstração prática, justificativa das decisões de projeto

---

## Exemplo: Código Fonte → 3AC

### Programa:
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

### Código de 3 endereços esperado:
```
main:
    t0 = new Fac           // cria objeto
    param t0               // objeto como contexto
    param 10               // argumento
    t1 = call ComputeFac, 2
    print t1
    halt

ComputeFac:
    t2 = lt num 1          // num < 1
    ifFalse t2 goto L0
    num_aux = 1            // bloco then
    goto L1
L0:                        // bloco else
    t3 = sub num 1
    param this
    param t3
    t4 = call ComputeFac, 2
    num_aux = mul num t4
L1:
    return num_aux
```
