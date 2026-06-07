#include "../../include/ast.h"
#include "../../include/codegen.h"
#include <stdio.h>



static TypedAST create_typed_ast(AST *ast) {

}

void generate_asm(AST *ast, FILE *out) {
    TypedAST typed_ast = create_typed_ast(ast); 
}
