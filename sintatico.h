#ifndef SINTATICO_H
#define SINTATICO_H

#include "lexico.h"



typedef enum {
    AST_NUM, AST_ADD, AST_SUB, AST_MUL, AST_DIV
} ASTKind;

typedef struct AST {
    ASTKind kind;
    double  num;          
    struct AST *left;     
    struct AST *right;    
} AST;

typedef struct {
    const Token *toks;
    int i, n;
} Parser;


void parse_program(const TokenVec *v); 


void  ast_print(const AST *t, int depth);
void  ast_to_dot(const AST *t, const char *path);
void  ast_free(AST *t);

#endif