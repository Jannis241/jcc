#include "../include/ast.h"
#include "../include/lexer.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>

static void print_indent(size_t indent) {
    for (size_t i = 0; i < indent; i++) {
        putchar(' ');
    }
}

static void print_line(size_t indent, const char *format, ...) {
    print_indent(indent);

    va_list args;
    va_start(args, format);
    vprintf(format, args);
    va_end(args);

    putchar('\n');
}

static void print_quoted_string(const char *value) {
    if (value == NULL) {
        printf("(null)");
        return;
    }

    putchar('"');
    for (size_t i = 0; value[i] != '\0'; i++) {
        unsigned char c = (unsigned char)value[i];

        switch (c) {
        case '\n':
            printf("\\n");
            break;
        case '\r':
            printf("\\r");
            break;
        case '\t':
            printf("\\t");
            break;
        case '\\':
            printf("\\\\");
            break;
        case '"':
            printf("\\\"");
            break;
        default:
            if (isprint(c)) {
                putchar(c);
            } else {
                printf("\\x%02x", (unsigned int)c);
            }
            break;
        }
    }
    putchar('"');
}

static void print_string_field(size_t indent, const char *name,
                               const char *value) {
    print_indent(indent);
    printf("%s: ", name);
    print_quoted_string(value);
    putchar('\n');
}

static const char *bool_name(bool value) {
    return value ? "true" : "false";
}

static const char *bin_op_name(BinOp op) {
    switch (op) {
    case BINOP_ADD:
        return "BINOP_ADD";
    case BINOP_SUB:
        return "BINOP_SUB";
    case BINOP_MUL:
        return "BINOP_MUL";
    case BINOP_DIV:
        return "BINOP_DIV";
    case BINOP_MOD:
        return "BINOP_MOD";
    case BINOP_EQ:
        return "BINOP_EQ";
    case BINOP_NE:
        return "BINOP_NE";
    case BINOP_LT:
        return "BINOP_LT";
    case BINOP_GT:
        return "BINOP_GT";
    case BINOP_GE:
        return "BINOP_GE";
    case BINOP_LE:
        return "BINOP_LE";
    case BINOP_BITOR:
        return "BINOP_BITOR";
    case BINOP_BITXOR:
        return "BINOP_BITXOR";
    case BINOP_BITAND:
        return "BINOP_BITAND";
    case BINOP_BITSHIFTLEFT:
        return "BINOP_BITSHIFTLEFT";
    case BINOP_BITSHIFTRIGHT:
        return "BINOP_BITSHIFTRIGHT";
    case BINOP_AND:
        return "BINOP_AND";
    case BINOP_OR:
        return "BINOP_OR";
    }

    return "BINOP_UNKNOWN";
}

static const char *unary_op_name(UnaryOp op) {
    switch (op) {
    case UNARY_NEG:
        return "UNARY_NEG";
    case UNARY_NOT:
        return "UNARY_NOT";
    }

    return "UNARY_UNKNOWN";
}

static const char *expr_kind_name(ASTExprKind kind) {
    switch (kind) {
    case AST_EXPR_STRING_LITERAL:
        return "AST_EXPR_STRING_LITERAL";
    case AST_EXPR_INT_LITERAL:
        return "AST_EXPR_INT_LITERAL";
    case AST_EXPR_BOOL_LITERAL:
        return "AST_EXPR_BOOL_LITERAL";
    case AST_EXPR_FLOAT_LITERAL:
        return "AST_EXPR_FLOAT_LITERAL";
    case AST_EXPR_CHAR_LITERAL:
        return "AST_EXPR_CHAR_LITERAL";
    case AST_EXPR_LIST_LITERAL:
        return "AST_EXPR_LIST_LITERAL";
    case AST_EXPR_STRUCT_LITERAL:
        return "AST_EXPR_STRUCT_LITERAL";
    case AST_EXPR_ENUM_LITERAL:
        return "AST_EXPR_ENUM_LITERAL";
    case AST_EXPR_BINARY:
        return "AST_EXPR_BINARY";
    case AST_EXPR_UNARY:
        return "AST_EXPR_UNARY";
    case AST_EXPR_CALL:
        return "AST_EXPR_CALL";
    case AST_EXPR_FIELD_ACCESS:
        return "AST_EXPR_FIELD_ACCESS";
    case AST_EXPR_POSTFIX:
        return "AST_EXPR_POSTFIX";
    case AST_EXPR_VARIABLE:
        return "AST_EXPR_VARIABLE";
    case AST_EXPR_ASSIGN:
        return "AST_EXPR_ASSIGN";
    case AST_EXPR_BINARY_ASSIGN:
        return "AST_EXPR_BINARY_ASSIGN";
    case AST_EXPR_CAST:
        return "AST_EXPR_CAST";
    case AST_EXPR_GROUPING:
        return "AST_EXPR_GROUPING";
    }

    printf("Unknown expr found in print ast: %d \n", kind );

    return "AST_EXPR_UNKNOWN";
}

static const char *stmt_kind_name(ASTStmtKind kind) {
    switch (kind) {
    case AST_STMT_BLOCK:
        return "AST_STMT_BLOCK";
    case AST_STMT_EXPR:
        return "AST_STMT_EXPR";
    case AST_STMT_LET:
        return "AST_STMT_LET";
    case AST_STMT_IF:
        return "AST_STMT_IF";
    case AST_STMT_WHILE:
        return "AST_STMT_WHILE";
    case AST_STMT_FOR:
        return "AST_STMT_FOR";
    case AST_STMT_RETURN:
        return "AST_STMT_RETURN";
    case AST_STMT_BREAK:
        return "AST_STMT_BREAK";
    case AST_STMT_CONTINUE:
        return "AST_STMT_CONTINUE";
    case AST_STMT_CONST:
        return "AST_STMT_CONST";
    case AST_STMT_TYPE:
        return "AST_STMT_TYPE";
    case AST_STMT_MATCH:
        return "AST_STMT_MATCH";
    }

    return "AST_STMT_UNKNOWN";
}

static const char *postfix_op_name(PostFixOp op) {
    switch (op) {
    case POSTFIX_OP_BRACKETS:
        return "POSTFIX_OP_BRACKETS";
    case POSTFIX_OP_MINUSMINUS:
        return "POSTFIX_OP_MINUSMINUS";
    case POSTFIX_OP_PLUSPLUS:
        return "POSTFIX_OP_PLUSPLUS";
    }

    return "POSTFIX_OP_UNKNOWN";
}

static void print_expr(const ASTExpr *expr, size_t indent);
static void print_stmt_block(const ASTStmtBlock *block, size_t indent);

static void print_type_name(const ASTTypeName *type_name, size_t indent) {
    if (type_name == NULL) {
        print_line(indent, "(null)");
        return;
    }

    print_line(indent, "TypeName");
    print_string_field(indent + 2, "name", type_name->name);
    print_string_field(indent + 2, "type", type_name->type);
}

static void print_type_name_vec(const ASTTypeNameVec *vec, size_t indent) {
    if (vec->len > 0 && vec->data == NULL) {
        print_line(indent, "(invalid vector: len=%zu, data=null)", vec->len);
        return;
    }

    for (size_t i = 0; i < vec->len; i++) {
        print_line(indent, "[%zu]", i);
        print_type_name(vec->data[i], indent + 2);
    }
}

static void print_expr_vec(const ASTExprVec *vec, size_t indent) {
    if (vec->len > 0 && vec->data == NULL) {
        print_line(indent, "(invalid vector: len=%zu, data=null)", vec->len);
        return;
    }

    for (size_t i = 0; i < vec->len; i++) {
        print_line(indent, "[%zu]", i);
        print_expr(vec->data[i], indent + 2);
    }
}

static void print_struct_literal_fields(const ASTNameExprVec *fields,
                                        size_t indent) {
    if (fields->len > 0 && fields->data == NULL) {
        print_line(indent, "(invalid vector: len=%zu, data=null)", fields->len);
        return;
    }

    for (size_t i = 0; i < fields->len; i++) {
        ASTNameExpr *field = fields->data[i];

        print_line(indent, "[%zu]", i);
        if (field == NULL) {
            print_line(indent + 2, "(null)");
            continue;
        }

        print_string_field(indent + 2, "name", field->name);
        print_line(indent + 2, "expr:");
        print_expr(field->expr, indent + 4);
    }
}

static void print_expr(const ASTExpr *expr, size_t indent) {
    if (expr == NULL) {
        print_line(indent, "Expr: (null)");
        return;
    }

    print_line(indent, "Expr");
    print_line(indent + 2, "kind: %s", expr_kind_name(expr->kind));

    switch (expr->kind) {
    case AST_EXPR_STRING_LITERAL:
    case AST_EXPR_INT_LITERAL:
    case AST_EXPR_BOOL_LITERAL:
    case AST_EXPR_FLOAT_LITERAL:
    case AST_EXPR_CHAR_LITERAL:
        print_string_field(indent + 2, "literal_value",
                           expr->value.literal_value);
        break;
    case AST_EXPR_LIST_LITERAL:
        print_line(indent + 2, "elements: %zu",
                   expr->value.list_literal.len);
        print_expr_vec(&expr->value.list_literal, indent + 4);
        break;
    case AST_EXPR_STRUCT_LITERAL:
        print_string_field(indent + 2, "name",
                           expr->value.struct_literal.name);
        print_line(indent + 2, "fields: %zu",
                   expr->value.struct_literal.fields.len);
        print_struct_literal_fields(&expr->value.struct_literal.fields,
                                    indent + 4);
        break;
    case AST_EXPR_ENUM_LITERAL:
        print_string_field(indent + 2, "enum_name",
                           expr->value.enum_literal.enum_name);
        print_string_field(indent + 2, "case_name",
                           expr->value.enum_literal.case_name);
        break;
    case AST_EXPR_BINARY:
        print_line(indent + 2, "op: %s",
                   bin_op_name(expr->value.binary.op));
        print_line(indent + 2, "lhs:");
        print_expr(expr->value.binary.lhs, indent + 4);
        print_line(indent + 2, "rhs:");
        print_expr(expr->value.binary.rhs, indent + 4);
        break;
    case AST_EXPR_UNARY:
        print_line(indent + 2, "op: %s", unary_op_name(expr->value.unary.op));
        print_line(indent + 2, "expr:");
        print_expr(expr->value.unary.expr, indent + 4);
        break;
    case AST_EXPR_CALL:
        print_line(indent + 2, "function_name:");
        print_expr(expr->value.call.function_name, indent + 4);
        print_line(indent + 2, "params: %zu", expr->value.call.params.len);
        print_expr_vec(&expr->value.call.params, indent + 4);
        break;
    case AST_EXPR_FIELD_ACCESS:
        print_string_field(indent + 2, "field_name",
                           expr->value.field_access.field_name);
        print_line(indent + 2, "obj:");
        print_expr(expr->value.field_access.obj, indent + 4);
        break;
    case AST_EXPR_VARIABLE:
        print_string_field(indent + 2, "variable_name",
                           expr->value.variable_name);
        break;
    case AST_EXPR_ASSIGN:
        print_line(indent + 2, "target:");
        print_expr(expr->value.assign.target, indent + 4);
        print_line(indent + 2, "value:");
        print_expr(expr->value.assign.value, indent + 4);
        break;
    case AST_EXPR_BINARY_ASSIGN:
        print_line(indent + 2, "op: %s",
                   bin_op_name(expr->value.bin_assign.op));
        print_line(indent + 2, "target:");
        print_expr(expr->value.bin_assign.target, indent + 4);
        print_line(indent + 2, "value:");
        print_expr(expr->value.bin_assign.value, indent + 4);
        break;
    case AST_EXPR_CAST:
        print_string_field(indent + 2, "type", expr->value.cast.type);
        print_line(indent + 2, "expr:");
        print_expr(expr->value.cast.expr, indent + 4);
        break;
    case AST_EXPR_POSTFIX:
        print_line(indent + 2, "op: %s",
                   postfix_op_name(expr->value.postfix.op));
        print_line(indent + 2, "obj:");
        print_expr(expr->value.postfix.obj, indent + 4);
        if (expr->value.postfix.value != NULL) {
            print_line(indent + 2, "value:");
            print_expr(expr->value.postfix.value, indent + 4);
        }
        break;
    case AST_EXPR_GROUPING:
        print_line(indent + 2, "inner:");
        print_expr(expr->value.grouping_inner, indent + 4);
        break;
    }
}

static void print_match_cases(const ASTStmtMatchCaseVec *cases,
                              size_t indent) {
    if (cases->len > 0 && cases->data == NULL) {
        print_line(indent, "(invalid vector: len=%zu, data=null)", cases->len);
        return;
    }

    for (size_t i = 0; i < cases->len; i++) {
        ASTStmtMatchCase *match_case = cases->data[i];

        print_line(indent, "[%zu]", i);
        if (match_case == NULL) {
            print_line(indent + 2, "(null)");
            continue;
        }

        print_line(indent + 2, "expr:");
        print_expr(match_case->expr, indent + 4);
        print_line(indent + 2, "block:");
        print_stmt_block(&match_case->block, indent + 4);
    }
}

static void print_stmt(const ASTStmt *stmt, size_t indent) {
    if (stmt == NULL) {
        print_line(indent, "Stmt: (null)");
        return;
    }

    print_line(indent, "Stmt");
    print_line(indent + 2, "kind: %s", stmt_kind_name(stmt->kind));

    switch (stmt->kind) {
    case AST_STMT_BLOCK:
        print_line(indent + 2, "block:");
        print_stmt_block(&stmt->value.block_stmt, indent + 4);
        break;
    case AST_STMT_EXPR:
        print_line(indent + 2, "expr:");
        print_expr(stmt->value.expr_stmt.expr, indent + 4);
        break;
    case AST_STMT_LET:
        print_string_field(indent + 2, "var_name",
                           stmt->value.let_stmt.var_name);
        print_string_field(indent + 2, "var_type",
                           stmt->value.let_stmt.var_type);
        print_line(indent + 2, "value:");
        print_expr(stmt->value.let_stmt.value, indent + 4);
        break;
    case AST_STMT_IF:
        print_line(indent + 2, "condition:");
        print_expr(stmt->value.if_stmt.condition, indent + 4);
        print_line(indent + 2, "code_block:");
        print_stmt_block(&stmt->value.if_stmt.code_block, indent + 4);
        print_line(indent + 2, "has_else: %s",
                   bool_name(stmt->value.if_stmt.has_else));
        if (stmt->value.if_stmt.has_else) {
            print_line(indent + 2, "else_stmt:");
            print_stmt(stmt->value.if_stmt.else_stmt, indent + 4);
        }
        break;
    case AST_STMT_WHILE:
        print_line(indent + 2, "condition:");
        print_expr(stmt->value.while_stmt.condition, indent + 4);
        print_line(indent + 2, "code_block:");
        print_stmt_block(&stmt->value.while_stmt.code_block, indent + 4);
        break;
    case AST_STMT_FOR:
        print_line(indent + 2, "init:");
        print_stmt(stmt->value.for_stmt.init, indent + 4);
        print_line(indent + 2, "condition:");
        print_expr(stmt->value.for_stmt.condition, indent + 4);
        print_line(indent + 2, "action:");
        print_stmt(stmt->value.for_stmt.action, indent + 4);
        print_line(indent + 2, "code_block:");
        print_stmt_block(&stmt->value.for_stmt.code_block, indent + 4);
        break;
    case AST_STMT_RETURN:
        print_line(indent + 2, "has_return_value: %s",
                   bool_name(stmt->value.return_stmt.has_return_value));
        if (stmt->value.return_stmt.has_return_value) {
            print_line(indent + 2, "value:");
            print_expr(stmt->value.return_stmt.value, indent + 4);
        }
        break;
    case AST_STMT_CONST:
        print_string_field(indent + 2, "name",
                           stmt->value.const_stmt.name);
        print_string_field(indent + 2, "type",
                           stmt->value.const_stmt.type);
        print_line(indent + 2, "value:");
        print_expr(stmt->value.const_stmt.value, indent + 4);
        break;
    case AST_STMT_TYPE:
        print_string_field(indent + 2, "name",
                           stmt->value.type_stmt.name);
        print_string_field(indent + 2, "type_name",
                           stmt->value.type_stmt.type_name);
        break;
    case AST_STMT_MATCH:
        print_line(indent + 2, "expr:");
        print_expr(stmt->value.match_stmt.expr, indent + 4);
        print_line(indent + 2, "cases: %zu",
                   stmt->value.match_stmt.cases.len);
        print_match_cases(&stmt->value.match_stmt.cases, indent + 4);
        break;
    case AST_STMT_BREAK:
    case AST_STMT_CONTINUE:
        break;
    }
}

static void print_stmt_block(const ASTStmtBlock *block, size_t indent) {
    if (block == NULL) {
        print_line(indent, "Block: (null)");
        return;
    }

    print_line(indent, "Block");
    print_line(indent + 2, "statements: %zu", block->statements.len);

    if (block->statements.len > 0 && block->statements.data == NULL) {
        print_line(indent + 4, "(invalid vector: len=%zu, data=null)",
                   block->statements.len);
        return;
    }

    for (size_t i = 0; i < block->statements.len; i++) {
        print_line(indent + 4, "[%zu]", i);
        print_stmt(block->statements.data[i], indent + 6);
    }
}

static void print_constants(const ASTConstVec *constants, size_t indent) {
    print_line(indent, "constants: %zu", constants->len);

    if (constants->len > 0 && constants->data == NULL) {
        print_line(indent + 2, "(invalid vector: len=%zu, data=null)",
                   constants->len);
        return;
    }

    for (size_t i = 0; i < constants->len; i++) {
        ASTConst *constant = constants->data[i];

        print_line(indent + 2, "[%zu] Const", i);
        if (constant == NULL) {
            print_line(indent + 4, "(null)");
            continue;
        }

        print_string_field(indent + 4, "name", constant->name);
        print_string_field(indent + 4, "type", constant->type);
        print_line(indent + 4, "value:");
        print_expr(constant->value, indent + 6);
    }
}

static void print_functions(const ASTFunctionVec *functions, size_t indent) {
    print_line(indent, "functions: %zu", functions->len);

    if (functions->len > 0 && functions->data == NULL) {
        print_line(indent + 2, "(invalid vector: len=%zu, data=null)",
                   functions->len);
        return;
    }

    for (size_t i = 0; i < functions->len; i++) {
        ASTFunction *function = functions->data[i];

        print_line(indent + 2, "[%zu] Function", i);
        if (function == NULL) {
            print_line(indent + 4, "(null)");
            continue;
        }

        print_string_field(indent + 4, "name", function->name);
        print_string_field(indent + 4, "return_type", function->return_type);
        print_line(indent + 4, "params: %zu", function->params.len);
        print_type_name_vec(&function->params, indent + 6);
        print_line(indent + 4, "block:");
        print_stmt_block(&function->block, indent + 6);
    }
}

static void print_struct_defs(const ASTStructDefVec *struct_defs,
                              size_t indent) {
    print_line(indent, "struct_defs: %zu", struct_defs->len);

    if (struct_defs->len > 0 && struct_defs->data == NULL) {
        print_line(indent + 2, "(invalid vector: len=%zu, data=null)",
                   struct_defs->len);
        return;
    }

    for (size_t i = 0; i < struct_defs->len; i++) {
        ASTStructDef *struct_def = struct_defs->data[i];

        print_line(indent + 2, "[%zu] StructDef", i);
        if (struct_def == NULL) {
            print_line(indent + 4, "(null)");
            continue;
        }

        print_string_field(indent + 4, "name", struct_def->name);
        print_line(indent + 4, "fields: %zu", struct_def->fields.len);
        print_type_name_vec(&struct_def->fields, indent + 6);
    }
}

static void print_enum_defs(const ASTEnumDefVec *enum_defs, size_t indent) {
    print_line(indent, "enum_defs: %zu", enum_defs->len);

    if (enum_defs->len > 0 && enum_defs->data == NULL) {
        print_line(indent + 2, "(invalid vector: len=%zu, data=null)",
                   enum_defs->len);
        return;
    }

    for (size_t i = 0; i < enum_defs->len; i++) {
        ASTEnumDef *enum_def = enum_defs->data[i];

        print_line(indent + 2, "[%zu] EnumDef", i);
        if (enum_def == NULL) {
            print_line(indent + 4, "(null)");
            continue;
        }

        print_string_field(indent + 4, "name", enum_def->name);
        print_line(indent + 4, "cases: %zu", enum_def->cases.len);

        if (enum_def->cases.len > 0 && enum_def->cases.data == NULL) {
            print_line(indent + 6, "(invalid vector: len=%zu, data=null)",
                       enum_def->cases.len);
            continue;
        }

        for (size_t case_index = 0; case_index < enum_def->cases.len;
             case_index++) {
            print_indent(indent + 6);
            printf("[%zu] ", case_index);
            print_quoted_string(enum_def->cases.data[case_index]);
            putchar('\n');
        }
    }
}

static void print_type_aliases(const ASTTypeAliasVec *type_aliases,
                               size_t indent) {
    print_line(indent, "types_aliases: %zu", type_aliases->len);

    if (type_aliases->len > 0 && type_aliases->data == NULL) {
        print_line(indent + 2, "(invalid vector: len=%zu, data=null)",
                   type_aliases->len);
        return;
    }

    for (size_t i = 0; i < type_aliases->len; i++) {
        ASTTypeAlias *type_alias = type_aliases->data[i];

        print_line(indent + 2, "[%zu] TypeAlias", i);
        if (type_alias == NULL) {
            print_line(indent + 4, "(null)");
            continue;
        }

        print_string_field(indent + 4, "name", type_alias->name);
        print_string_field(indent + 4, "type", type_alias->type);
    }
}

void print_ast(const AST *ast) {
    if (ast == NULL) {
        printf("AST: (null)\n");
        return;
    }

    print_line(0, "AST");
    print_constants(&ast->constants, 2);
    print_functions(&ast->functions, 2);
    print_struct_defs(&ast->struct_defs, 2);
    print_enum_defs(&ast->enum_defs, 2);
    print_type_aliases(&ast->types_aliases, 2);
}
