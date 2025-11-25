#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexico.h"
#include "sintatico.h"

/* Lê o arquivo do disco pra RAM de uma vez */
char *read_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "Erro: não consegui abrir '%s'\n", path);
        exit(1);
    }

    /* Vai pro fim do arquivo pra pegar o tamanho */
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *buffer = malloc(size + 1);
    if (!buffer) {
        fprintf(stderr, "Erro: faltou memória pra ler o arquivo\n");
        exit(1);
    }

    fread(buffer, 1, size, f);
    buffer[size] = '\0'; /* Fecha a string */
    fclose(f);
    return buffer;
}

int main(int argc, char **argv) {

    if (argc < 2) {
        printf("Uso: %s <arquivo>\n", argv[0]);
        return 1;
    }

    char *src = read_file(argv[1]);

    /* Passa o scanner e depois o parser */
    TokenVec tv = tokenize_to_vector(src);
    parse_program(&tv);
    
    /* Faxina na saída */
    tv_free(&tv);
    free(src);
    
    return 0;
}