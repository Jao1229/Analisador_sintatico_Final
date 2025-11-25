#ifndef DECLARACOES_H
#define DECLARACOES_H

#include "sintatico.h"  // para ter acesso ao Parser e currentToken

void parte_de_declaracoes_de_variaveis(Parser *p);
void declaracao_de_variaveis(Parser *p);
void lista_identificadores(Parser *p);
void tipo(Parser *p);

#endif
