#ifndef AST_H
#define AST_H

#include "vector.h"
#include <stdbool.h>

typedef struct {
    const char* type;
    const char* name;
} ASTTypeName;

VECTOR_DEFINE(ASTTypeName*, ASTTypeNameVec)

typedef struct ASTExpr ASTExpr;

typedef enum {
    BINOP_ADD,
    BINOP_SUB,
    BINOP_MUL,
    BINOP_DIV,
    BINOP_MOD,
    BINOP_EQ,
    BINOP_NE,
    BINOP_LT,
    BINOP_GT,
    BINOP_GE,
    BINOP_LE,

    BINOP_BITOR,
    BINOP_BITXOR,
    BINOP_BITAND,
    BINOP_BITSHIFTLEFT,
    BINOP_BITSHIFTRIGHT,

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
    AST_EXPR_ADDR_LITERAL,
    AST_EXPR_DEREFERENCE_LITERAL,

    AST_EXPR_LIST_LITERAL,

    AST_EXPR_STRUCT_LITERAL,
    AST_EXPR_ENUM_LITERAL,

    AST_EXPR_BINARY,
    AST_EXPR_UNARY,
    AST_EXPR_CALL,  
    AST_EXPR_FIELD_ACCESS,
    AST_EXPR_POSTFIX,
    AST_EXPR_VARIABLE,

    AST_EXPR_ASSIGN,
    AST_EXPR_BINARY_ASSIGN,
    
    AST_EXPR_CAST,

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
    AST_STMT_CONST,
    AST_STMT_TYPE,
    AST_STMT_MATCH,
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
    const char* var_name;
    const char* var_type;
    ASTExpr* value;
} ASTStmtLet;



typedef struct {
    ASTExpr* condition;
    ASTStmtBlock code_block;
    bool has_else; 
    ASTStmt *else_stmt;
} ASTStmtIf;

typedef struct {
    ASTExpr* condition;
    ASTStmtBlock code_block;
} ASTStmtWhile;
typedef struct {
    ASTExpr* target;
    ASTExpr* value;
} ASTStmtAssign;

typedef struct {
    ASTStmt* init;
    ASTExpr* condition;
    ASTStmt* action;
    ASTStmtBlock code_block;
} ASTStmtFor;

typedef struct {
    bool has_return_value;
    ASTExpr* value; // kann auch void sein
} ASTStmtReturn;


typedef struct {
    ASTExpr* target;
    BinOp op;
    ASTExpr* value;
} ASTStmtBinaryAssign;

typedef struct {
    const char* name;
    const char* type;
    ASTExpr* value;
} ASTStmtConst;

typedef struct {
    ASTExpr* expr;
    ASTStmtBlock block;
} ASTStmtMatchCase;

VECTOR_DEFINE(ASTStmtMatchCase*, ASTStmtMatchCaseVec)

typedef struct {
    ASTExpr* expr;
    ASTStmtMatchCaseVec cases;
} ASTStmtMatch;

typedef struct {
    const char* name;
    const char* type_name;
} ASTStmtType;

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
        ASTStmtConst const_stmt;
        ASTStmtMatch match_stmt;
        ASTStmtType type_stmt;
        ASTStmtReturn return_stmt;
        // break und continue brauchen keine value
    } value;
};





typedef struct {
    const char* name;
    ASTExpr* expr;
} ASTNameExpr;

VECTOR_DEFINE(ASTNameExpr*, ASTNameExprVec)

typedef struct {
    const char* enum_name;
    const char* case_name;
} ASTExprEnumLiteral;


typedef struct {
    const char* name;
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
    ASTExpr* function_name;
    ASTExprVec params;
} ASTExprCall;


typedef struct ASTExprFieldAccess {
    ASTExpr* obj;
    const char* field_name;
} ASTExprFieldAccess;


typedef enum PostFixOp {
    POSTFIX_OP_BRACKETS,
    POSTFIX_OP_MINUSMINUS,
    POSTFIX_OP_PLUSPLUS,
} PostFixOp;

typedef struct ASTExprPostfix {
    PostFixOp op;
    ASTExpr* obj;
    ASTExpr* value;
} ASTExprPostfix;

typedef struct ASTExprAssign {
    ASTExpr* target;
    ASTExpr* value;
} ASTExprAssign;

typedef struct ASTExprBinaryAssign {
    ASTExpr* target;
    BinOp op;
    ASTExpr* value;
} ASTExprBinaryAssign;

typedef struct ASTExprCast {
    ASTExpr* expr;
    const char* type;
} ASTExprCast;

struct ASTExpr{
    ASTExprKind kind;
    union {
        const char* literal_value;
        ASTExprStructLiteral struct_literal;
        ASTExprEnumLiteral enum_literal;
        ASTExprBinary binary;
        ASTExprUnary unary;
        ASTExprCast cast;
        ASTExprCall call;
        ASTExprAssign assign;
        ASTExprBinaryAssign bin_assign;
        ASTExprFieldAccess field_access;
        ASTExprPostfix postfix;
        ASTExpr* grouping_inner;
        const char* variable_name;
        ASTExprVec list_literal;
    } value;
} ;


typedef struct {
    const char *name;
    const char *type;
    ASTExpr* value;
} ASTConst;

VECTOR_DEFINE(const char*, StrVec)
typedef struct {
    const char* name;
    StrVec cases;
} ASTEnumDef;


typedef struct {
    const char* name;
    ASTTypeNameVec fields; 
} ASTStructDef;

typedef struct {
    const char* name;
    const char* return_type;
    ASTTypeNameVec params;
    ASTStmtBlock block;
} ASTFunction;

typedef struct {
    const char* name;
    const char* type;
} ASTTypeAlias;

VECTOR_DEFINE(ASTFunction*, ASTFunctionVec)
VECTOR_DEFINE(ASTStructDef*, ASTStructDefVec)
VECTOR_DEFINE(ASTEnumDef*, ASTEnumDefVec)
VECTOR_DEFINE(ASTConst*, ASTConstVec)
VECTOR_DEFINE(ASTTypeAlias*, ASTTypeAliasVec)

// Alle top level items kommen hier rein
typedef struct {
    ASTFunctionVec functions;
    ASTStructDefVec struct_defs;
    ASTEnumDefVec enum_defs;
    ASTConstVec  constants;
    ASTTypeAliasVec types_aliases;
} AST;

void print_ast(const AST *ast);

#endif
