# Checklist do Projeto — Compilador MiniJava

## Fase 1: Analisador Léxico (Flex) ✅

- [x] **1.1** Definir todos os tokens da linguagem no `src/lexer.l`
  - [x] Palavras-chave: `class`, `public`, `static`, `void`, `main`, `String`, `return`, `int`, `boolean`, `if`, `else`, `while`, `System`, `out`, `println`, `true`, `false`, `this`, `new`, `extends`, `length`
  - [x] Operadores: `&&`, `<`, `>`, `+`, `-`, `*`, `=`, `!`
  - [x] Delimitadores: `(`, `)`, `[`, `]`, `{`, `}`, `;`, `,`, `.`
  - [x] Identificadores: `[a-zA-Z][a-zA-Z0-9_]*`
  - [x] Números inteiros: `[0-9]+`
- [x] **1.2** Integrar o pré-processador no Flex (remoção de comentários `//` e `/* */` + espaços em excesso já tratados pelo Flex)
- [x] **1.3** Gerar uma lista de tokens como saída (struct Token com tipo, lexema, linha)
- [x] **1.4** Implementar tratamento de erros léxicos
  - [x] Mensagem informando o caractere/token inválido e a linha
  - [x] Sugestões de correção para identificadores mal formados (distância de Levenshtein)
- [x] **1.5** Integrar o lexer com o parser (lista completa de tokens passada ao Parser)

## Fase 2: Gramática LL(1) ✅ (arquivo `docs/Gramatica-Update.pdf`)

- [x] **2.1** Remover recursão à esquerda (Exp, Word, Number)
- [x] **2.2** Fatoração à esquerda (DefCl, DefMet)
- [x] **2.3** Documentar novas variáveis introduzidas (Exp', DefCl', Cmd', Args', PrimExp', PostExpDot, etc.)

## Fase 3: Analisador Sintático (Parser Recursivo Descendente) ✅

- [x] **3.1** Implementar estrutura base do parser em C++
  - [x] Classe Parser que recebe a lista de tokens
  - [x] Funções auxiliares: `match(expectedToken)`, `currentToken()`, `advance()`, `error()`
- [x] **3.2** Implementar funções para cada não-terminal da gramática:
  - [x] `parseProg()` → MainC DefCl
  - [x] `parseMainC()` → 'class' Id '{' 'public' 'static' 'void' 'main' '(' 'String' '[' ']' Id ')' '{' CmdList '}' '}'
  - [x] `parseDefCl()` → 'class' Id DefCl' | λ
  - [x] `parseDefClTail()` → '{' DefVar DefMet '}' DefCl | 'extends' Id '{' DefVar DefMet '}' DefCl
  - [x] `parseDefVar()` → Type Id ';' DefVar | λ
  - [x] `parseDefMet()` → 'public' Type Id '(' DefMetArgs ')' '{' DefVar CmdList 'return' Exp ';' '}' DefMet | λ
  - [x] `parseDefMetArgs()` → Args | λ
  - [x] `parseType()` → 'int' '[' ']' | 'boolean' | 'int' | Id
  - [x] `parseArgs()` → Type Id Args'
  - [x] `parseArgsTail()` → ',' Args | λ
  - [x] `parseCmd()` → '{' CmdList '}' | 'if' ... | 'while' ... | 'System' ... | Id Cmd'
  - [x] `parseCmdTail()` → '=' Exp ';' | '[' Exp ']' '=' Exp ';'
  - [x] `parseCmdList()` → Cmd CmdList | λ
  - [x] `parseExp()` → PrimExp Exp'
  - [x] `parseExpTail()` → Op PrimExp Exp' | '[' Exp ']' Exp' | '.' PostExpDot | λ
  - [x] `parseOp()` → '&&' | '<' | '>' | '+' | '-' | '\*'
  - [x] `parsePostExpDot()` → 'length' Exp' | Id '(' ListExp ')' Exp'
  - [x] `parseListExp()` → Exp ListExp' | λ
  - [x] `parseListExpTail()` → ',' Exp ListExp' | λ
  - [x] `parsePrimExp()` → 'new' PrimExp' | '!' Exp | '(' Exp ')' | 'true' | 'false' | Id | Number | 'this'
  - [x] `parsePrimExpTail()` → Id '(' ')' | 'int' '[' Exp ']'
- [x] **3.3** Implementar tratamento de erros sintáticos
  - [x] Informar local exato do erro (linha e token encontrado)
  - [x] Informar tipo do erro (ex: "esperado ';' mas encontrou '}'")
- [x] **3.4** Saída de sucesso: mensagem "Código sintaticamente correto" (tabela de símbolos na Fase 4)

## Fase 4: Tabela de Símbolos ✅

- [x] **4.1** Definir estrutura da tabela (campos: nome, tipo, escopo, linha de declaração, categoria)
  - [x] Categorias: classe, método, variável local, parâmetro, variável de instância
- [x] **4.2** Preencher a tabela durante o parsing (ao encontrar declarações)
- [x] **4.3** Exibir a tabela de símbolos na saída do compilador

## Fase 5: Integração e Testes ✅

- [x] **5.1** Pipeline completo: arquivo .ling → Flex (pré-proc + lexer) → Parser → Saída
- [x] **5.2** Testar com `assets/prog-factorial.ling`
- [x] **5.3** Testar com `assets/prog-bubblesort.ling`
- [x] **5.4** Testar com programas sintaticamente incorretos (verificar mensagens de erro)

## Fase 6: Revisão e Domínio do Código

- [ ] **6.1** Entender o fluxo completo do compilador (main → lexer → parser → tabela de símbolos)
- [ ] **6.2** Entender como o Flex funciona (regras, prioridade, integração com C++)
- [ ] **6.3** Entender a lógica do parser (recursive descent, lookahead em DefVar, FIRST sets)
- [ ] **6.4** Entender a tabela de símbolos (quando e onde cada símbolo é inserido)
- [ ] **6.5** Melhorar código: adicionar mais comentários explicativos onde necessário
- [ ] **6.6** Melhorar código: refatorar trechos repetitivos (ex: lookahead em parseDefVar)
- [ ] **6.7** Melhorar mensagens de erro (mais contexto, sugestões de correção sintática)
- [ ] **6.8** Testar edge cases adicionais e corrigir bugs encontrados
- [ ] **6.9** Garantir que cada membro do grupo consiga explicar qualquer parte do código

## Fase 7: Relatório Técnico

- [ ] **7.1** Estruturas de dados utilizadas e sua utilidade
- [ ] **7.2** Técnica de análise sintática adotada (Recursive Descent)
- [ ] **7.3** Gramática transformada (referenciar Gramatica-Update)
- [ ] **7.4** Exemplos de execução (sucesso e erro)

## Fase 8: Entrega

- [ ] **8.1** Email para valdigleis@dimap.ufrn.br com relatório + fontes + data de apresentação
- [ ] **8.2** Apresentação (entre 11 de maio e 28 de maio)
