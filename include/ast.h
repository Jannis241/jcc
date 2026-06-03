#ifndef AST_H
#define AST_H
#include "vector.h"

typedef struct AST_EXPR AST_EXPR;
typedef struct {
    char* type;
    char* name;
} AST_TYPE_NAME;

VECTOR_DEFINE(AST_TYPE_NAME*, ASTTypeNameVec)

typedef enum {
    EXPRKIND_STRING_LITERAL,
    EXPRKIND_INT_LITERAL,
    EXPRKIND_BOOL_LITERAL,
    EXPRKIND_FLOAT_LITERAL,
    EXPRKIND_CHAR_LITERAL,

    EXPRKIND_LIST_LITERAL,

    EXPRKIND_STRUCT_LITERAL,
    EXPRKIND_ENUM_LITERAL,

    EXPRKIND_BINARY,
    EXPRKIND_UNARY,
    EXPRKIND_ASSIGN, 
    EXPRKIND_BINARY_ASSIGN,
    EXPRKIND_CALL,  
    EXPRKIND_FIELD_ACCESS,
    EXPRKIND_VARIABLE,

    EXPRKIND_GROUPING,   
} ASTExprKind;

typedef enum {
    ASTKIND_ROOT,
    ASTKIND_STMT,
    ASTKIND_FUNC,
    ASTKIND_CONST,
    ASTKIND_EXPR,
    ASTKIND_STRUCTDEF,
    ASTKIND_ENUMDEF,
} ASTKind;

typedef enum {
    STMTKIND_BLOCK,
    STMTKIND_EXPR,
    STMTKIND_LET,
    STMTKIND_IF,
    STMTKIND_WHILE,
    STMTKIND_FOR,
    STMTKIND_RETURN,
    STMTKIND_BREAK,
    STMTKIND_CONTINUE,
} ASTStmtKind;


typedef struct AST_STMT AST_STMT;


VECTOR_DEFINE(AST_STMT*, ASTStmtVec)

typedef struct {
    ASTStmtVec statements;
} AST_STMT_BLOCK;

typedef struct {
    AST_EXPR* expr; 
} AST_STMT_EXPR;

typedef struct {
    char* var_name;
    char* var_type;
    AST_EXPR* value;
} AST_STMT_LET;



typedef struct {
    AST_EXPR* condition;
    AST_STMT_BLOCK code_block;
    int has_else; 
    AST_STMT_BLOCK optional_else_block;
} AST_STMT_IF;

typedef struct {
    AST_EXPR* condition;
    AST_STMT_BLOCK code_block;
} AST_STMT_WHILE;

typedef struct {
    char* var_name;
    AST_EXPR* condition;
    AST_STMT_BLOCK code_block;
} AST_STMT_FOR;

typedef struct {
    int has_return_value;
    AST_EXPR* value; // kann auch void sein
} AST_STMT_RETURN;

struct AST_STMT {
    ASTStmtKind kind;
    union {
        AST_STMT_BLOCK block_stmt;
        AST_STMT_EXPR expr_stmt;
        AST_STMT_LET let_stmt;
        AST_STMT_IF if_stmt;
        AST_STMT_WHILE while_stmt;
        AST_STMT_FOR for_stmt;
        AST_STMT_RETURN return_stmt;
        // break und continue brauchen keine value
    } value;
};





typedef struct {
    char* name;
    AST_EXPR* expr;
} AST_NAME_EXPR;

VECTOR_DEFINE(AST_NAME_EXPR*, ASTNameExprVec)

typedef struct {
    char* enum_name;
    char* case_name;
} EXPR_ENUM_LITERAL;


typedef struct {
    char* name;
    ASTNameExprVec fields; // field name, expr => x: 2+3 -> x (field name) 2+3 (value -> expr)
} EXPR_STRUCT_LITERAL;

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
    Neg,
    Not,
} UnaryOp;

typedef struct {
    AST_EXPR* lhs; 
    AST_EXPR* rhs; 
    BinOp op;
} EXPR_BINARY;

typedef struct {
    AST_EXPR* expr; 
    UnaryOp op;
} EXPR_UNARY;

typedef struct {
    AST_EXPR* target;
    AST_EXPR* value;
} EXPR_ASSIGN;

typedef struct {
    AST_EXPR* target;
    BinOp op;
    AST_EXPR* value;
} EXPR_BINARY_ASSIGN;


VECTOR_DEFINE(AST_EXPR*, AstExprVec)

typedef struct EXPR_CALL {
    char* function_name;
    AstExprVec params;
} EXPR_CALL;


typedef struct EXPR_FIELD_ACCESS {
    AST_EXPR* obj;
    char* field_name;
} EXPR_FIELD_ACCESS;


struct AST_EXPR{
    ASTExprKind kind;
    union {
        char* literal_value;
        EXPR_STRUCT_LITERAL struct_literal;
        EXPR_ENUM_LITERAL enum_literal;
        EXPR_BINARY binary;
        EXPR_UNARY unary;
        EXPR_ASSIGN assign;
        EXPR_BINARY_ASSIGN bin_assign;
        EXPR_CALL call;
        EXPR_FIELD_ACCESS field_access;
        AST_EXPR* grouping_inner;
        char* variable_name;
        AstExprVec list_literal;
    } value;
} ;


typedef struct {
    char *name;
    char *type;
    AST_EXPR* value;
} AST_CONST;

VECTOR_DEFINE(char*, StrVec)
typedef struct {
    char* name;
    StrVec cases;
} AST_ENUMDEF;


typedef struct {
    char* name;
    ASTTypeNameVec fields; 
} AST_STRUCTDEF;

typedef struct {
    char* name;
    char* return_type;
    ASTTypeNameVec params;
    AST_STMT_BLOCK block;
} AST_FUNCTION;

VECTOR_DEFINE(AST_FUNCTION*, ASTFunctionVec)
VECTOR_DEFINE(AST_STRUCTDEF*, ASTStructVec)
VECTOR_DEFINE(AST_ENUMDEF*, ASTEnumVec)
VECTOR_DEFINE(AST_CONST*, ASTConstVec)
typedef struct {
    ASTFunctionVec functions;
    ASTStructVec struct_defs;
    ASTEnumVec enum_defs;
    ASTConstVec  constants;
} AST_ROOT;

typedef struct {
    ASTKind kind;
    union {
        AST_ROOT root;
        AST_EXPR expr;
        AST_STMT stmt;
        AST_STRUCTDEF struct_def;
        AST_ENUMDEF enum_def;
        AST_FUNCTION function;
    } value;
} ASTNode;

#endif
