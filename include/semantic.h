#ifndef SEMANTIC_H
#define SEMANTIC_H

#include "lexer.h"
#include "ast.h"
#include "vector.h"

typedef enum {
    SEMANTIC_OK,
} Status;

typedef struct {
    Status status;
    size_t token_pos;
    SourceSpan span;
    char *message;
} SemanticError;

typedef enum {
    SYMBOLKKIND_VAR,
} SymbolKind;

typedef enum {
    TYPE_INT,
    TYPE_BOOL,
    TYPE_CHAR,
    TYPE_FLOAT,
    TYPE_NAMED,
} Type;

VECTOR_DEFINE(Type, TypeVec);

typedef struct {
    char* func_name;
    Type return_type;
    TypeVec params;
}
FuncSig;

typedef struct Symbol {
    char *name;
    SymbolKind kind;

    union {
        struct {
            Type return_type;
            TypeVec params;
        } func;

        Type var_type;
    };
} Symbol;

VECTOR_DEFINE(Symbol, SymbolVec);

typedef struct SymbolTable {
    struct SymbolTable *parent;
    SymbolVec symbols;
} SymbolTable;


typedef struct TypedExpr {
    Type type;
} TypedExpr;

typedef enum {
    TDECL_FUNCTION,
    TDECL_STRUCT,
    TDECL_ENUM,
    TDECL_CONST,
    TDECL_TYPE_ALIAS
} TypedDeclKind;

typedef struct {
    Symbol *symbol;
    TypedExpr *value;
} TypedConstDecl;

typedef struct TypedParam{
    Type type;
    char *name;
} TypeName;

VECTOR_DEFINE(TypeName, TypedTypeNameVec);

typedef struct {
    Symbol *symbol;
    Type return_type;
    ASTStmtVec body;
    TypedTypeNameVec params;
} TypedFunctionDecl;

typedef struct {
    Symbol *symbol;
    TypedTypeNameVec fields;
} TypedStructDecl;

typedef struct {
    TypedDeclKind kind;

    union {
        TypedFunctionDecl function;
        TypedStructDecl struct_decl;
        TypedConstDecl constant;
    };
} TypedDecl;

VECTOR_DEFINE(TypedDecl, TypedDeclVec)

typedef struct {
    TypedDeclVec declarations;
} TypedAST;

typedef struct {
    SemanticError err;
    TypedAST ast;
} SemanticResult;

SemanticResult lower_ast(AST *ast);

#endif
