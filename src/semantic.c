#include "../include/ast.h"
#include <stdio.h>
#include "../include/semantic.h"

SemanticResult lower_ast(AST *ast){
    if (ast == NULL) {
        printf("Ast is null.. \n");
        exit(-1);
    }
    return (SemanticResult){};
}
