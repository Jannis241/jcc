#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "typed_ast.h"
#include "ast.h"

typedef enum {
    SEMA_OK,
} SemanticStatus;

typedef struct {
    SemanticStatus status;
} SemanticError;

typedef struct {
    SemanticError error;
    TypedAST typed_ast;
} SemanticResult;

SemanticResult lower_ast(AST *ast);

#endif



