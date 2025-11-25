#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "lexico.h"
#include "sintatico.h"
#include "declaracoes.h" 

/* === Funções Auxiliares do Parser ===
   Mantivemos estáticas aqui para uso interno.
*/

static const Token *cur(const Parser *p) {
    if (!p || !p->toks) return NULL;
    if (p->i < 0) return NULL;
    if (p->i >= p->n) return &p->toks[p->n-1];
    return &p->toks[p->i];
}

static void advance(Parser *p) {
    if (!p) return;
    if (p->i < p->n - 1) p->i++;
}

static void match(Parser *p, int expected) {
    const Token *t = cur(p);
    if (!t) {
        fprintf(stderr, "0:fim de arquivo nao esperado.\n");
        exit(EXIT_FAILURE);
    }
    if (t->type == expected) {
        advance(p);
        return;
    } else {
        /* Gestão de erros: mostra linha e o que veio errado */
        if (t->type == END_FILE) {
            fprintf(stderr, "%d:fim de arquivo nao esperado.\n", t->line);
        } else {
            const char *lex = t->lexeme ? t->lexeme : token_name(t->type);
            fprintf(stderr, "%d:token nao esperado [%s].\n", t->line, lex);
        }
        exit(EXIT_FAILURE);
    }
}

/* Sinônimo para match, só pra ficar legível que "esperamos" tal token */
static void expect(Parser *p, int expected) {
    match(p, expected);
}

/* --- Declaração antecipada das funções --- */
static void programa(Parser *p);
static void bloco(Parser *p);
static void comando_composto(Parser *p);
static void comando(Parser *p);
static void atribuicao(Parser *p);
static void comando_condicional(Parser *p);
static void comando_repetitivo(Parser *p);

/* Funções de Expressões (Matemática e Lógica) */
static void expressao(Parser *p);
static void expressao_simples(Parser *p);
static void termo(Parser *p);
static void fator(Parser *p);
static void relacao(Parser *p);
static void variavel(Parser *p);

/* === Implementação das Regras da Gramática === 
*/

/* Regra principal: programa começa com 'program', tem nome, e termina com ponto. */
static void programa(Parser *p) {
    printf("<programa> ::= program <identificador> ; <bloco> .\n");
    expect(p, PROGRAM_TOK);
    expect(p, ID);
    expect(p, SEMICOLON);
    bloco(p);
    expect(p, DOT);
}

/* O bloco junta as declarações (var) e os comandos (código em si) */
static void bloco(Parser *p) {
    parte_de_declaracoes_de_variaveis(p); 
    comando_composto(p);
}

/* O famoso bloco begin ... end */
static void comando_composto(Parser *p) {
    printf("<comando_composto> ::= begin <comando> ; { <comando> ; } end\n");
    expect(p, BEGIN_TOK);

    /* Tem que ter ao menos um comando */
    comando(p);
    expect(p, SEMICOLON);

    /* Aqui a gente fica rodando enquanto houver novos comandos.
       Como sabemos que é um comando? Se começar com ID, begin, if ou while.
    */
    while (cur(p)->type == ID || cur(p)->type == BEGIN_TOK || 
           cur(p)->type == IF_TOK || cur(p)->type == WHILE_TOK) {
        comando(p);
        expect(p, SEMICOLON);
    }

    expect(p, END_TOK);
}

/* Decide qual tipo de comando executar com base no token atual */
static void comando(Parser *p) {
    int t = cur(p)->type;

    if (t == ID) {
        atribuicao(p);        /* Ex: x := 10 */
    } else if (t == BEGIN_TOK) {
        comando_composto(p);  /* Ex: begin ... end */
    } else if (t == IF_TOK) {
        comando_condicional(p); /* Ex: if ... then */
    } else if (t == WHILE_TOK) {
        comando_repetitivo(p);  /* Ex: while ... do */
    } else {
        /* Se não for nenhum desses, temos um erro de sintaxe. */
        const Token *err = cur(p);
        const char *lex = err->lexeme ? err->lexeme : token_name(err->type);
        fprintf(stderr, "%d:token nao esperado [%s].\n", err->line, lex);
        exit(EXIT_FAILURE);
    }
}

/* Atribuição: coloca valor numa variável. Ex: a := b + 1 */
static void atribuicao(Parser *p) {
    printf("<atribuicao> ::= <variavel> := <expressao>\n");
    variavel(p);       /* O lado esquerdo (quem recebe) */
    expect(p, ASSIGN); /* O símbolo := */
    expressao(p);      /* O lado direito (o valor calculado) */
}

/* Estrutura IF ... THEN ... [ELSE] */
static void comando_condicional(Parser *p) {
    printf("<comando_condicional> ::= if <expressao> then <comando> [else <comando>]\n");
    expect(p, IF_TOK);
    expressao(p);      /* A condição */
    expect(p, THEN_TOK);
    comando(p);        /* O que fazer se for verdade */

    /* O ELSE é opcional, só entramos aqui se o token atual for 'else' */
    if (cur(p)->type == ELSE_TOK) {
        expect(p, ELSE_TOK);
        comando(p);
    }
}

/* Estrutura WHILE ... DO */
static void comando_repetitivo(Parser *p) {
    printf("<comando_repetitivo> ::= while <expressao> do <comando>\n");
    expect(p, WHILE_TOK);
    expressao(p);      /* Condição de parada */
    expect(p, DO_TOK);
    comando(p);        /* O que repetir */
}

/* === Análise de Expressões ===
   Aqui a precedência importa (quem é calculado primeiro).
*/

/* Expressão geral: pode ter comparação (ex: a < b) */
static void expressao(Parser *p){
    printf("<expressao> ::= <expressao_simples> [<relacao> <expressao_simples>]\n");
    expressao_simples(p);

    const Token *t = cur(p);
    if (!t) return;

    /* Se tiver operador relacional (=, <, >, etc), processa a segunda parte */
    switch (t->type){
        case EQ:
        case NE:
        case LT:
        case LE:
        case GT:
        case GE:
            relacao(p);
            expressao_simples(p);
            break;
    }
}

/* Verifica qual operador de comparação estamos usando */
static void relacao(Parser *p){
    printf("<relacao> ::= = | <> | < | <= | >= | >\n");
    const Token *t = cur(p);
    
    switch(t->type){
        case EQ: match(p, EQ); break;
        case NE: match(p, NE); break;
        case LT: match(p, LT); break;
        case LE: match(p, LE); break;
        case GT: match(p, GT); break;
        case GE: match(p, GE); break;
        default:
            fprintf(stderr, "%d: operador relacional esperado.\n", t->line);
            exit(EXIT_FAILURE);
    }
}

/* Expressão simples: somas e subtrações */
static void expressao_simples (Parser *p){
    printf("<expressao_simples> ::= [+|-] <termo> { (+|-) <termo> }\n");
    
    /* Verifica sinal unário opcional no começo (ex: -10 ou +5) */
    const Token *check = cur(p);
    if (check && (check->type == PLUS || check->type == MINUS)) {
        match(p, check->type);
    }

    termo(p);

    /* Processa cadeias de soma/subtração: a + b - c */
    const Token *t = cur(p);
    while (t && (t->type == PLUS || t->type == MINUS)){
        match(p, t->type);
        termo(p);
        t = cur(p);
    }
}

/* Termo: multiplicações e divisões (têm precedência sobre soma) */
static void termo (Parser *p){
    printf("<termo> ::= <fator> { (*|/) <fator> }\n");
    fator(p);

    const Token *t = cur(p);
    while (t && (t->type == MULT || t->type == DIV)){
        match(p, t->type);
        fator(p);
        t = cur(p);
    }
}

/* Fator: a unidade básica (número, variável ou expressão entre parênteses) */
static void fator(Parser *p){
    printf("<fator> ::= <variavel> | <numero> | (<expressao>)\n");
    const Token *t = cur(p);

    if (!t){
        printf("Erro: fator não esperado\n");
        exit(EXIT_FAILURE);
    }

    if (t->type == ID){
        variavel(p); 
    }
    else if (t->type == NUM){
        match(p, NUM);
    }
    else if (t->type == LPAREN){
        /* Se abrir parênteses, resolvemos a expressão interna primeiro */
        match(p, LPAREN);
        expressao(p);
        match(p, RPAREN);
    }
    else{
        fprintf(stderr, "%d:fator invalido [%s]\n",
        t->line, t->lexeme ? t->lexeme : token_name(t->type));
        exit(EXIT_FAILURE);
    }
}

/* Variável é apenas um identificador neste nível */
static void variavel(Parser *p) {
    expect(p, ID);
}

/* Função principal que dispara o parser */
void parse_program(const TokenVec *v) {
    if (!v) return;
    Parser p;
    p.toks = v->data;
    p.i = 0;
    p.n = v->size;

    if (p.n <= 0) {
        fprintf(stderr, "0:fim de arquivo nao esperado.\n");
        exit(EXIT_FAILURE);
    }

    programa(&p);
}