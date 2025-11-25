#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "declaracoes.h"
#include "lexico.h"     
#include "sintatico.h"  

/* Aqui fica a lógica pra lidar com as variáveis e tipos.
   É a parte que entende "var a, b: integer;" e afins.
*/

/* --- Utilitários pra facilitar a vida --- */

/* Dá uma espiada no token atual sem consumir ele. Se acabou, devolve o último. */
const Token *cur(Parser *p){
    if (!p) return NULL;
    if (p->i < 0) return NULL;
    if (p->i >= p->n) return &p->toks[p->n-1]; /* Proteção contra estouro de array */
    return &p->toks[p->i];
}

/* Só avança o índice pro próximo. */
void advance(Parser *p){
    if (p->i < p->n - 1) p->i++;
}

/* Verifica se o token é o que a gente quer. 
   Se for, beleza, passa. Se não, derruba o programa com erro.
*/
void match(Parser *p, int expected){
    const Token *t = cur(p);
    if (!t) {
        fprintf(stderr, "0:fim de arquivo não esperado.\n");
        exit(EXIT_FAILURE);
    }
    if (t->type == expected){
        /* Bateu! Segue o baile. */
        advance(p);
        return;
    } else {
        if (t->type == END_FILE) {
            fprintf(stderr, "%d:fim de arquivo não esperado.\n", t->line);
            exit(EXIT_FAILURE);
        } else {
            /* Veio coisa errada na linha tal */
            const char *lex = t->lexeme ? t->lexeme : token_name(t->type);
            fprintf(stderr, "%d:token nao esperado [%s].\n", t->line, lex);
            exit(EXIT_FAILURE);
        }
    }
}

void declaracao_de_variaveis(Parser *p);
void lista_identificadores(Parser *p);
void tipo(Parser *p);

/* Processa o bloco de variáveis.
   Lembrando: 'var' é opcional. Se não tiver, a gente só sai de fininho.
   Se tiver, tem que ler as declarações até achar algo que não seja variável (geralmente o begin).
*/
void parte_de_declaracoes_de_variaveis(Parser *p){
    const Token *t = cur(p);
    if (!t) return;

    /* Se não começa com 'var', não tem nada pra ver aqui. */
    if (t->type != VAR_TOK){
        return;
    }

    printf("<parte_de_declaracoes_de_variaveis> ::= var <declaracao_de_variaveis> { ; <declaracao_de_variaveis> } ;\n");
    match(p, VAR_TOK);

    /* Obrigatório ter pelo menos uma declaração depois do 'var' */
    declaracao_de_variaveis(p);

    /* Loop pra pegar declarações extras separadas por ponto e vírgula */
    while (cur(p) && cur(p)->type == SEMICOLON){
        match(p, SEMICOLON);
        
        /* Cuidado aqui: O ';' pode ser só o separador final antes do 'begin'.
           Se o próximo token for o início do programa, paramos de ler variáveis.
        */
        if (cur(p)->type == BEGIN_TOK) {
            break;
        }
        
        if (cur(p)->type == ID) {
            declaracao_de_variaveis(p);
        } else {
            /* Tinha um ';' mas veio lixo depois */
            const Token *errt = cur(p);
            if (errt->type == END_FILE) {
                fprintf(stderr, "%d:fim de arquivo não esperado.\n", errt->line);
                exit(EXIT_FAILURE);
            } else {
                const char *lex = errt->lexeme ? errt->lexeme : token_name(errt->type);
                fprintf(stderr, "%d:token nao esperado [%s].\n", errt->line, lex);
                exit(EXIT_FAILURE);
            }
        }
    }
}

/* Exemplo: x, y, z : integer */
void declaracao_de_variaveis(Parser *p){
    printf("<declaracao_de_variaveis> ::= <lista_de_identificadores> : <tipo>\n");
    lista_identificadores(p);
    match(p, COLON); /* Os dois pontos são cruciais */
    tipo(p);
}

/* Pega a lista de nomes: id, id, id... */
void lista_identificadores(Parser *p){
    printf("<lista_de_identificadores> ::= <identificador> { , <identificador> }\n");
    const Token *t = cur(p);
    if (!t) {
        fprintf(stderr, "0:fim de arquivo não esperado.\n");
        exit(EXIT_FAILURE);
    }
    
    /* Tem que começar com um ID */
    if (t->type != ID){
        if (t->type == END_FILE) {
            fprintf(stderr, "%d:fim de arquivo não esperado.\n", t->line);
        } else {
            const char *lex = t->lexeme ? t->lexeme : token_name(t->type);
            fprintf(stderr, "%d:token nao esperado [%s].\n", t->line, lex);
        }
        exit(EXIT_FAILURE);
    }
    match(p, ID);

    /* Consome vírgula e o próximo ID repetidamente */
    while (cur(p) && cur(p)->type == COMMA){
        match(p, COMMA);
        if (cur(p)->type != ID){
            /* Vírgula sem nome depois é erro de sintaxe */
            const Token *errt = cur(p);
            if (errt->type == END_FILE) {
                fprintf(stderr, "%d:fim de arquivo não esperado.\n", errt->line);
            } else {
                const char *lex = errt->lexeme ? errt->lexeme : token_name(errt->type);
                fprintf(stderr, "%d:token nao esperado [%s].\n", errt->line, lex);
            }
            exit(EXIT_FAILURE);
        }
        match(p, ID);
    }
}

/* Valida se é integer ou real */
void tipo(Parser *p){
    printf("<tipo> ::= integer | real\n");
    const Token *t = cur(p);
    if (!t) {
        fprintf(stderr, "0:fim de arquivo não esperado.\n");
        exit(EXIT_FAILURE);
    }
    if (t->type == INTEGER_TOK){
        match(p, INTEGER_TOK);
    } else if (t->type == REAL_TOK){
        match(p, REAL_TOK);
    } else {
        /* Tipo desconhecido */
        if (t->type == END_FILE) {
            fprintf(stderr, "%d:fim de arquivo não esperado.\n", t->line);
        } else {
            const char *lex = t->lexeme ? t->lexeme : token_name(t->type);
            fprintf(stderr, "%d:token nao esperado [%s].\n", t->line, lex);
        }
        exit(EXIT_FAILURE);
    }
}