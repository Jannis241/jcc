#include "../include/semantic.h"
#include "../include/typed_ast.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void print_type(Type* type) {
    if (type == NULL) {
        printf("Got NULL as type \n");
        return;
    }

    switch (type->kind) {
    case INT:
        printf("INT \n");
        break;

    case FLOAT:
        printf( "FLOAT \n");
        break;

    case BOOL:
        printf( "BOOl \n");
        break;

    case VOID:
        printf( "VOID \n");
        break;

    case ARRAY:
        printf( "ARRAY");
        print_type(type->info.array_element_type);
        printf("\n");
        break;
    case POINTER:
        printf( "POINTER: ");
        print_type(type->info.pointed_type);
        printf("\n");
        break;
    case CHAR:
        printf( "CHAR \n");
        break;
    case NAMED:
        printf( "NAMED: '%s' \n", type->info.named_name);
        break;
    }
}

static Type type_from_string(const char *str) {
    if (str == NULL) {
        printf("Recieved NULL Str for type (semantic.c) \n");
        exit(-1);
    }

    size_t str_len = strlen(str);

    bool is_array = false;

    if (str[0] == '[') {
        is_array = true;
    }
    bool is_pointer = str[str_len - 1] == '*';

    int start = is_array ? 1 : 0;
    int end = is_pointer ? str_len - 1 : str_len;

    char new_slice[end - start + 1];

    for (int i = start; i < end; i++) {
        new_slice[i] = str[i];
    }
    new_slice[end - start] = '\0';

    if (strcmp("int", new_slice) == 0) {
        if (is_pointer) {
            Type *t = malloc(sizeof(Type));
            t->kind = INT;
            return (Type){.kind = POINTER, .info.pointed_type = t};
        }
        if (is_array) {
            Type *t = malloc(sizeof(Type));
            t->kind = INT;
            return (Type){.kind = ARRAY, .info.pointed_type = t};
        }
        return (Type){.kind = INT};
    } else if (strcmp("float", new_slice) == 0) {
        if (is_pointer) {
            Type *t = malloc(sizeof(Type));
            t->kind = FLOAT;
            return (Type){.kind = POINTER, .info.pointed_type = t};
        }
        if (is_array) {
            Type *t = malloc(sizeof(Type));
            t->kind = FLOAT;
            return (Type){.kind = ARRAY, .info.pointed_type = t};
        }
        return (Type){.kind = FLOAT};
    }

    else if (strcmp("bool", new_slice) == 0) {
        if (is_pointer) {
            Type *t = malloc(sizeof(Type));
            t->kind = BOOL;
            return (Type){.kind = POINTER, .info.pointed_type = t};
        }
        if (is_array) {
            Type *t = malloc(sizeof(Type));
            t->kind = BOOL;
            return (Type){.kind = ARRAY, .info.pointed_type = t};
        }
        return (Type){.kind = BOOL};

    } else if (strcmp("void", new_slice) == 0) {
        if (is_pointer) {
            Type *t = malloc(sizeof(Type));
            t->kind = VOID;
            return (Type){.kind = POINTER, .info.pointed_type = t};
        }
        if (is_array) {
            Type *t = malloc(sizeof(Type));
            t->kind = VOID;
            return (Type){.kind = ARRAY, .info.pointed_type = t};
        }
        return (Type){.kind = VOID};

    } else if (strcmp("char", new_slice) == 0) {
        if (is_pointer) {
            Type *t = malloc(sizeof(Type));
            t->kind = CHAR;
            return (Type){.kind = POINTER, .info.pointed_type = t};
        }
        if (is_array) {
            Type *t = malloc(sizeof(Type));
            t->kind = CHAR;
            return (Type){.kind = ARRAY, .info.pointed_type = t};
        }
        return (Type){.kind = CHAR};
    } else {
        return (Type){.kind = NAMED, .info.named_name = new_slice};
    }
}

static TopLevelItemVec collect_top_level_items(AST *ast) {
    TopLevelItemVec items;
    TopLevelItemVec_init(&items);
    for (int i = 0; i < ast->functions.len; i++) {
        ASTFunction *f = ast->functions.data[i];

        TopLevelItem new_item;
        new_item.name = f->name;
        new_item.type = TLT_FUNC;
        new_item.info.Func.return_type = type_from_string(f->return_type);

        TypedParamVec pv;
        TypedParamVec_init(&pv);
        for (int j = 0; j < f->params.len; j++) {
            ASTTypeName p = *f->params.data[j];

            Type t = type_from_string(p.type);
            TypedParam tp = {.type = t, .name = p.name};
            TypedParamVec_push(&pv, tp);
        }
        new_item.info.Func.params = pv;

        TopLevelItemVec_push(&items, new_item);
    }
    return items;
}

SemanticResult lower_ast(AST *ast) {
    TopLevelItemVec top_level_items = collect_top_level_items(ast);

    for (int i = 0; i < top_level_items.len; i++) {
        if (top_level_items.data[i].type == TLT_FUNC) {
            TopLevelItem ti = top_level_items.data[i];

            printf("Top Level function: \n");
            printf("Name: %s \n", ti.name);
            printf("Return type: ");
            print_type(&ti.info.Func.return_type);
        }
    }

    return (SemanticResult){0};
}
