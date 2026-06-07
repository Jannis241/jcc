#include "../include/ast.h"
#include "../include/codegen.h"
#include <stdio.h>


void codegen(AST *ast, FILE* out) {
    if (ast == NULL || out == NULL) {
        printf("AST or Output Buffer are NULL \n");
        exit(-1);
    }
}

