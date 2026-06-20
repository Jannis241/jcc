#ifndef TYPED_AST_H
#define TYPED_AST_H
#include "ast.h"
#include "vector.h"

typedef struct {

} TypedAST;

typedef enum {
    TLT_FUNC,
    TLT_CONST,
    TLT_STRUCT,
    TLT_ENUM,
    TLT_TYPEALIAS,
} TopLevelType;

enum TypeKind {
    INT,
    FLOAT,
    BOOL,
    ARRAY,
    POINTER,
    CHAR,
    VOID,
    NAMED,
};

typedef struct Type Type;

struct Type {
    enum TypeKind kind;
    union {
        const char *named_name;
        Type *pointed_type;
        Type *array_element_type;
    } info;
};
typedef struct {
    const char *name;
    Type type;
} TypedParam;

VECTOR_DEFINE(TypedParam, TypedParamVec)

typedef struct {
    const char *name;
    TopLevelType type;
    union {
        struct {
            Type return_type;
            TypedParamVec params;
        } Func;
        struct {
            Type type;
        } Constant;
        struct {
            TypedParamVec fields;
        } Struct;
        struct {
            StrVec case_names;
        } Enum;
        struct {
            Type aliased_type;
        } TypeAlias;
    } info;
} TopLevelItem;

VECTOR_DEFINE(TopLevelItem, TopLevelItemVec)

#endif /* ifndef TYPED_AST_H */
