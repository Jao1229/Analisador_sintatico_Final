#ifndef LEXICO_H
#define LEXICO_H

/* -------------------- Definições de tokens -------------------- */

// Terminais já existentes para Expressões
#define NUM    256
#define PLUS   '+'
#define MINUS  '-'
#define MULT   '*'
#define DIV    '/'
#define LPAREN '('
#define RPAREN ')'

// Novos tokens para MicroPascal (Palavras Reservadas e Símbolos)
#define ID               257 
#define PROGRAM_TOK      258 
#define VAR_TOK          259 
#define INTEGER_TOK      260 
#define REAL_TOK         261 
#define BEGIN_TOK        262 
#define END_TOK          263 
#define IF_TOK           264 
#define THEN_TOK         265 
#define ELSE_TOK         266 
#define WHILE_TOK        267 
#define DO_TOK           268 

// Símbolos de Pontuação e Operadores
#define DOT              '.'   
#define SEMICOLON        ';'   
#define COLON            ':'  
#define COMMA            ','  
#define ASSIGN           269   

// Tokens de Relação (adaptados da gramática MicroPascal)
#define EQ               300   // =
#define NE               301   // <> (Não igual)
#define LT               '<'   // Menor que
#define LE               302   // <= (Menor ou igual)
#define GT               '>'   // Maior que
#define GE               303   // >= (Maior ou igual)

#define END_FILE         0     // Fim do arquivo

typedef struct {
    int type;
    double value;   
    char *lexeme;   
    int line;       
} Token;

/* Estrutura de vetor de tokens */
typedef struct {
    Token *data;
    int size;
    int cap;
} TokenVec;

/* -------------------- Assinaturas -------------------- */
TokenVec tokenize_to_vector(const char *src);
void tv_free(TokenVec *v);
const char *token_name(int t); 

#endif