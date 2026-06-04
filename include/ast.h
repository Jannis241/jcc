#ifndef AST_H
#define AST_H
#include "vector.h"
#include<stdbool.h>

typedef struct {
    char* type;
    char* name;
} ASTTypeName;

VECTOR_DEFINE(ASTTypeName*, ASTTypeNameVec)

typedef struct ASTExpr ASTExpr;

typedef enum {
    BINOP_ADD,
    BINOP_SUB,
    BINOP_MUL,
    BINOP_DIV,
    BINOP_EQ,
    BINOP_NE,
    BINOP_LT,
    BINOP_GT,
    BINOP_GE,
    BINOP_LE,

    BINOP_AND,
    BINOP_OR,
} BinOp;

typedef enum {
    UNARY_NEG,
    UNARY_NOT,
} UnaryOp;

typedef enum {
    AST_EXPR_STRING_LITERAL,
    AST_EXPR_INT_LITERAL,
    AST_EXPR_BOOL_LITERAL,
    AST_EXPR_FLOAT_LITERAL,
    AST_EXPR_CHAR_LITERAL,

    AST_EXPR_LIST_LITERAL,

    AST_EXPR_STRUCT_LITERAL,
    AST_EXPR_ENUM_LITERAL,

    AST_EXPR_BINARY,
    AST_EXPR_UNARY,
    AST_EXPR_CALL,  
    AST_EXPR_FIELD_ACCESS,
    AST_EXPR_VARIABLE,

    AST_EXPR_GROUPING,   
} ASTExprKind;

typedef enum {
    AST_STMT,
    AST_FUNCTION,
    AST_CONST,
    AST_EXPR,
    AST_STRUCT_DEF,
    AST_ENUM_DEF,
} ASTKind;

typedef enum {
    AST_STMT_BLOCK,
    AST_STMT_EXPR,
    AST_STMT_LET,
    AST_STMT_IF,
    AST_STMT_WHILE,
    AST_STMT_FOR,
    AST_STMT_RETURN,
    AST_STMT_BREAK,
    AST_STMT_CONTINUE,
    AST_STMT_ASSIGN,
    AST_STMT_BINARY_ASSIGN,
} ASTStmtKind;


typedef struct ASTStmt ASTStmt;


VECTOR_DEFINE(ASTStmt*, ASTStmtVec)

typedef struct {
    ASTStmtVec statements;
} ASTStmtBlock;

typedef struct {
    ASTExpr* expr; 
} ASTStmtExpr;

typedef struct {
    char* var_name;
    char* var_type;
    ASTExpr* value;
} ASTStmtLet;



typedef struct {
    ASTExpr* condition;
    ASTStmtBlock code_block;
    bool has_else; 
    ASTStmtBlock optional_else_block;
} ASTStmtIf;

typedef struct {
    ASTExpr* condition;
    ASTStmtBlock code_block;
} ASTStmtWhile;

typedef struct {
    char* var_name;
    ASTExpr* condition;
    ASTStmtBlock code_block;
} ASTStmtFor;

typedef struct {
    bool has_return_value;
    ASTExpr* value; // kann auch void sein
} ASTStmtReturn;

typedef struct {
    ASTExpr* target;
    ASTExpr* value;
} ASTStmtAssign;

typedef struct {
    ASTExpr* target;
    BinOp op;
    ASTExpr* value;
} ASTStmtBinaryAssign;

struct ASTStmt {
    ASTStmtKind kind;
    union {
        ASTStmtBlock block_stmt;
        ASTStmtExpr expr_stmt;
        ASTStmtLet let_stmt;
        ASTStmtIf if_stmt;
        ASTStmtAssign assign;
        ASTStmtBinaryAssign bin_assign;
        ASTStmtWhile while_stmt;
        ASTStmtFor for_stmt;
        ASTStmtReturn return_stmt;
        // break und continue brauchen keine value
    } value;
};





typedef struct {
    char* name;
    ASTExpr* expr;
} ASTNameExpr;

VECTOR_DEFINE(ASTNameExpr*, ASTNameExprVec)

typedef struct {
    char* enum_name;
    char* case_name;
} ASTExprEnumLiteral;


typedef struct {
    char* name;
    ASTNameExprVec fields; // field name, expr => x: 2+3 -> x (field name) 2+3 (value -> expr)
} ASTExprStructLiteral;


typedef struct {
    ASTExpr* lhs; 
    ASTExpr* rhs; 
    BinOp op;
} ASTExprBinary;

typedef struct {
    ASTExpr* expr; 
    UnaryOp op;
} ASTExprUnary;



VECTOR_DEFINE(ASTExpr*, ASTExprVec)

typedef struct ASTExprCall {
    char* function_name;
    ASTExprVec params;
} ASTExprCall;


typedef struct ASTExprFieldAccess {
    ASTExpr* obj;
    char* field_name;
} ASTExprFieldAccess;


struct ASTExpr{
    ASTExprKind kind;
    union {
        char* literal_value;
        ASTExprStructLiteral struct_literal;
        ASTExprEnumLiteral enum_literal;
        ASTExprBinary binary;
        ASTExprUnary unary;
        ASTExprCall call;
        ASTExprFieldAccess field_access;
        ASTExpr* grouping_inner;
        char* variable_name;
        ASTExprVec list_literal;
    } value;
} ;


typedef struct {
    char *name;
    char *type;
    ASTExpr* value;
} ASTConst;

VECTOR_DEFINE(char*, StrVec)
typedef struct {
    char* name;
    StrVec cases;
} ASTEnumDef;


typedef struct {
    char* name;
    ASTTypeNameVec fields; 
} ASTStructDef;

typedef struct {
    char* name;
    char* return_type;
    ASTTypeNameVec params;
    ASTStmtBlock block;
} ASTFunction;

VECTOR_DEFINE(ASTFunction*, ASTFunctionVec)
VECTOR_DEFINE(ASTStructDef*, ASTStructDefVec)
VECTOR_DEFINE(ASTEnumDef*, ASTEnumDefVec)
VECTOR_DEFINE(ASTConst*, ASTConstVec)

// Alle top level items kommen hier rein
typedef struct {
    ASTFunctionVec functions;
    ASTStructDefVec struct_defs;
    ASTEnumDefVec enum_defs;
    ASTConstVec  constants;
} AST;



#endif
