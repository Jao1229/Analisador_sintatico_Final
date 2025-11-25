#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include "lexico.h"

static const char *input;
static int current_line = 1;

/* Vetor dinâmico
   Implementação simples pra guardar os tokens sem saber a quantidade exata antes.
*/
static void tv_init(TokenVec *v) { v->data=NULL; v->size=v->cap=0; }

static void tv_reserve(TokenVec *v, size_t n){
    if(n <= v->cap) return;
    size_t cap = v->cap ? v->cap : 16;
    while(cap < n) cap *= 2; /* Cresce exponencialmente pra não ficar realocando toda hora */
    Token *p = realloc(v->data, cap * sizeof(Token));
    if(!p){ perror("realloc"); exit(1); }
    v->data=p; v->cap=cap;
}

static void tv_push(TokenVec *v, Token t){
    if(v->size+1 > v->cap) tv_reserve(v, v->size+1);
    v->data[v->size++] = t;
}

void tv_free(TokenVec *v){ 
    /* Limpa a sujeira da memória */
    for(int i=0; i<v->size; i++) {
        free(v->data[i].lexeme); 
    }
    free(v->data); v->data=NULL; v->size=v->cap=0; 
}


/* Lexer */

static void skip_ws_and_newlines(void){
    while(*input != '\0') {
        /* Ignora espaços, tabs, e conta as linhas pra ajudar no debug depois */
        if (*input == ' ' || *input == '\t' || *input == '\n' || *input == '\r') {
            if (*input == '\n') current_line++;
            input++;
            continue;
        }
        /* Ignora caracteres de controle estranhos se aparecerem */
        if ((unsigned char)*input <= 0x1F) {
            if (*input == '\n') current_line++;
            input++;
            continue;
        }
        break;
    }
}

static int check_keyword(const char *s){
    if (strcmp(s, "program") == 0) return PROGRAM_TOK;
    if (strcmp(s, "var") == 0)     return VAR_TOK;
    if (strcmp(s, "integer") == 0) return INTEGER_TOK;
    if (strcmp(s, "real") == 0)    return REAL_TOK;
    if (strcmp(s, "begin") == 0)   return BEGIN_TOK;
    if (strcmp(s, "end") == 0)     return END_TOK;
    if (strcmp(s, "if") == 0)      return IF_TOK;
    if (strcmp(s, "then") == 0)    return THEN_TOK;
    if (strcmp(s, "else") == 0)    return ELSE_TOK;
    if (strcmp(s, "while") == 0)   return WHILE_TOK;
    if (strcmp(s, "do") == 0)      return DO_TOK;
    return ID; /* Se não for palavra reservada, é variável/ID */
}

/* Fiz meu próprio strndup pq nem todo compilador/SO (tipo Windows antigo) suporta nativamente.
*/
static char *my_strndup(const char *s, size_t n) {
    char *p = malloc(n + 1);
    if (!p) return NULL;
    memcpy(p, s, n);
    p[n] = '\0';
    return p;
}

static Token getToken(void){
    Token tok = {0, 0.0, NULL, current_line};
    skip_ws_and_newlines();

    // Acabou o arquivo
    if(*input=='\0'){ tok.type=END_FILE; tok.line=current_line; return tok; }

    // Identificadores e Palavras Chave
    if(isalpha((unsigned char)*input)){
        const char *start = input;
        /* Vai engolindo caracteres alfanuméricos */
        while(isalnum((unsigned char)*input) || *input == '_') input++;
        int len = input - start;
        char *lex = my_strndup(start, len);
        tok.type = check_keyword(lex);
        tok.lexeme = lex;
        return tok;
    }
    
    // Números
    if(isdigit((unsigned char)*input)){
        char *endptr;
        tok.value = strtod(input, &endptr);
        int len = endptr - input;
        tok.lexeme = my_strndup(input, len);
        input = endptr;
        tok.type=NUM; 
        return tok;
    }

    // Símbolos compostos (tipo :=, <=, etc)
    if (*input == ':') {
        input++;
        if (*input == '=') { tok.type = ASSIGN; input++; } // Achou :=
        else { tok.type = COLON; } // Só :
    }
    else if (*input == '<') {
        input++;
        if (*input == '=') { tok.type = LE; input++; } // <=
        else if (*input == '>') { tok.type = NE; input++; } // <>
        else { tok.type = LT; } // <
    }
    else if (*input == '>') {
        input++;
        if (*input == '=') { tok.type = GE; input++; } // >=
        else { tok.type = GT; } // >
    }
    
    // Símbolos simples
    else {
        switch(*input){
            case '+': tok.type=PLUS; break;
            case '-': tok.type=MINUS; break;
            case '*': tok.type=MULT; break;
            case '/': tok.type=DIV; break;
            case '(': tok.type=LPAREN; break;
            case ')': tok.type=RPAREN; break;
            case ';': tok.type=SEMICOLON; break;
            case ',': tok.type=COMMA; break;
            case '=': tok.type=EQ; break;
            case '.': tok.type=DOT; break;
            default:
                fprintf(stderr,"Erro léxico na linha %d: caractere estranho '%c'\n", current_line, *input);
                exit(1);
        }
        input++;
    }

    /* Aloca lexema pra símbolos simples, senão dá pau na hora de imprimir erro depois */
    if (!tok.lexeme && tok.type != 0 && tok.type != NUM && tok.type != ID) {
        tok.lexeme = (char*)malloc(2);
        tok.lexeme[0] = (char)(tok.type);
        tok.lexeme[1] = '\0';
    }

    return tok;
}

/* Gera o vetorzão com todos os tokens */
TokenVec tokenize_to_vector(const char *src){

    /* Remove BOM se tiver (aqueles bytes chatos do UTF-8 no início) */
    if (src && src[0] == (char)0xEF && src[1] == (char)0xBB && src[2] == (char)0xBF) {
        input = src + 3;
    } else {
        input = src;
    }

    current_line = 1;
    TokenVec v; tv_init(&v);
    for(;;){
        Token t = getToken();
        tv_push(&v, t);
        if(t.type == END_FILE) break;
    }
    return v;
}

/* Converte o enum pra string legível (pra debug) */
const char *token_name(int t){
    switch(t){
        case NUM: return "NUMERO";
        case ID: return "IDENTIFICADOR";
        case PROGRAM_TOK: return "PROGRAM";
        case VAR_TOK: return "VAR";
        case INTEGER_TOK: return "INTEGER";
        case REAL_TOK: return "REAL";
        case BEGIN_TOK: return "BEGIN";
        case END_TOK: return "END";
        case IF_TOK: return "IF";
        case THEN_TOK: return "THEN";
        case ELSE_TOK: return "ELSE";
        case WHILE_TOK: return "WHILE";
        case DO_TOK: return "DO";
        case PLUS: return "+";
        case MINUS: return "-";
        case MULT: return "*";
        case DIV: return "/";
        case LPAREN: return "(";
        case RPAREN: return ")";
        case DOT: return ".";
        case SEMICOLON: return ";";
        case COLON: return ":";
        case COMMA: return ",";
        case ASSIGN: return ":=";
        case EQ: return "=";
        case NE: return "<>";
        case LT: return "<";
        case LE: return "<=";
        case GT: return ">";
        case GE: return ">=";
        case END_FILE: return "FIM_DE_ARQUIVO";
        default: return "TOKEN_DESCONHECIDO";
    }
}