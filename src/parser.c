#include"../include/lexer.h"
#include"../include/parser.h"
#include"../include/ast.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static bool contains_tokenkind(TokenKind value, TokenKind *arr, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (arr[i] == value) {
            return true;
        }
    }
    return false;
}

static ASTExpr* parse_expr(Parser* parser);
static ASTStmtBlock parse_block(Parser* parser);
static ASTStmt* parse_statement(Parser* parser);

static bool expect(Parser *parser, TokenKind kind) {
    if (parser->current_token->kind != kind) {
        parser->parser_error = (ParserError) {.expected = kind, .got = parser->current_token->kind, .token_pos = parser->pos, .status = PARSER_ERR_UNEXPECTED_TOKEN};
        return false;
    }
    return true;
}

static inline Token *peek(Parser *parser) {
    return &parser->tokens->data[parser->pos + 1];
}
static inline Token *peek2(Parser *parser) {
    return &parser->tokens->data[parser->pos + 2];
}


static inline void advance(Parser *parser) {
    parser->pos += 1;
    parser->current_token = &parser->tokens->data[parser->pos];
}

static const char* parse_type(Parser *parser) {
    if (parser->current_token->kind == TOKEN_LBRACKET) {
        advance(parser);

        const char* inner_type = parse_type(parser);
        if (inner_type == NULL) {
            return NULL;
        }

        if (!expect(parser, TOKEN_RBRACKET)) {
            return NULL;
        }
        advance(parser);

        char* array_type = malloc(strlen(inner_type) + 3);
        if (array_type == NULL) {
            printf("Malloc failed \n");
            exit(-1);
        }
        sprintf(array_type, "[%s]", inner_type);
        return array_type;
    }

    if (!expect(parser, TOKEN_IDENT)) {
        return NULL;
    }

    const char* type = parser->current_token->value;
    advance(parser);
    return type;
}

static ASTExpr* parse_primary(Parser *parser) {
    ASTExpr* node = malloc(sizeof(ASTExpr));

    if (node == NULL) {
        printf("Malloc failed \n");
        exit(-1);
    }

    switch(parser->current_token->kind) {
        case TOKEN_INT:
            *node = (ASTExpr){.kind = AST_EXPR_INT_LITERAL, .value.literal_value = parser->current_token->value};   
            advance(parser);
        break;
        case TOKEN_CHAR:
            *node = (ASTExpr){.kind = AST_EXPR_CHAR_LITERAL, .value.literal_value = parser->current_token->value};   
            advance(parser);
        break;
        case TOKEN_STRING:
            *node = (ASTExpr){.kind = AST_EXPR_STRING_LITERAL, .value.literal_value = parser->current_token->value};   
            advance(parser);
        break;
        case TOKEN_FLOAT:
            *node = (ASTExpr){.kind = AST_EXPR_FLOAT_LITERAL, .value.literal_value = parser->current_token->value};   
            advance(parser);
        break;
        case TOKEN_TRUE:
            *node = (ASTExpr){.kind = AST_EXPR_BOOL_LITERAL, .value.literal_value = parser->current_token->value};   
            advance(parser);
        break;
        case TOKEN_FALSE:
            *node = (ASTExpr){.kind = AST_EXPR_BOOL_LITERAL, .value.literal_value = parser->current_token->value};   
            advance(parser);
        break;
        case TOKEN_IDENT:
            // ident { ==> struct
            if (peek(parser)->kind == TOKEN_LBRACE) {
                // parsing struct literal 
                ASTExprStructLiteral struct_literal = {.name = parser->current_token->value};

                ASTNameExprVec fields;
                ASTNameExprVec_init(&fields);


                advance(parser); // auf dem {
                advance(parser); // eventuell auf einem field_name, kann aber auch schon } sein. 

                while (1) {
                    if (parser->current_token->kind == TOKEN_RBRACE) {
                        advance(parser);
                        break;
                    }
                    if (!expect(parser, TOKEN_IDENT)) return NULL;
                    const char* field_name = parser->current_token->value;
                    advance(parser);

                    if (!expect(parser, TOKEN_COLON)) return NULL;
                    advance(parser);

                    ASTExpr* value = parse_expr(parser);

                    if (value == NULL) {
                        return NULL;
                    }

                    ASTNameExpr *p = malloc(sizeof(ASTNameExpr));
                    *p = (ASTNameExpr) {.name = field_name, .expr = value};
                    if (!ASTNameExprVec_push(&fields, p)) {
                        printf("Pushing Vec element failed. \n");
                        exit(-1);
                    }

                    if (parser->current_token->kind == TOKEN_RBRACE) {
                        advance(parser);
                        break;
                    }
                    if (!expect(parser, TOKEN_COMMA)) return NULL;
                    advance(parser);
                }
                struct_literal.fields = fields;
                *node = (ASTExpr){.kind = AST_EXPR_STRUCT_LITERAL, .value.struct_literal = struct_literal};   


            }
            // ident::ident ==> enum
            else if (peek(parser)->kind == TOKEN_COLONCOLON && peek2(parser)->kind == TOKEN_IDENT) {
                const char* enum_name = parser->current_token->value; 
                advance(parser);
                advance(parser);
                const char* case_name = parser->current_token->value; 
                ASTExprEnumLiteral elit = {.enum_name = enum_name, .case_name = case_name};
               *node = (ASTExpr){.kind = AST_EXPR_ENUM_LITERAL, .value.enum_literal = elit};   
                advance(parser);
            }
            // ident ==> variable
            else {
                *node = (ASTExpr){.kind = AST_EXPR_VARIABLE, .value.variable_name = parser->current_token->value};   
                advance(parser);
            }
        break;
        case TOKEN_LPARENT:
            advance(parser);
            ASTExpr* inner = parse_expr(parser);
            if (inner == NULL) return NULL;
            if (!expect(parser, TOKEN_RPARENT)) return NULL;
            *node = (ASTExpr){.kind = AST_EXPR_GROUPING, .value.grouping_inner = inner};   
            advance(parser);
        break;
        case TOKEN_LBRACKET:
            advance(parser);
            ASTExprVec elements;
            ASTExprVec_init(&elements);

            if (parser->current_token->kind != TOKEN_RBRACKET) {
                ASTExpr* element = parse_expr(parser);
                if (element == NULL) return NULL;

                if (!ASTExprVec_push(&elements, element)) {
                    printf("Pushing Vec element failed. \n");
                    exit(-1);
                }
                

                while(parser->current_token->kind == TOKEN_COMMA) {
                    advance (parser);
                    ASTExpr* element = parse_expr(parser);
                    if (element == NULL) return NULL;
                    ASTExprVec_push(&elements, element);
                }
            }
            if (!expect(parser, TOKEN_RBRACKET)) return NULL;
            advance(parser);
            *node = (ASTExpr){.kind = AST_EXPR_LIST_LITERAL, .value.list_literal = elements};   

        break;
        default: 
            parser->parser_error = (ParserError){.got = parser->current_token->kind, .token_pos = parser->pos, .status = PARSER_ERR_UNEXPECTED_EXPR_START} ;
            return NULL;
    }

    return node;
}
static ASTExpr* finish_call(Parser *parser, ASTExpr* callee) {
    ASTExprVec args;
    ASTExprVec_init(&args);

    if (parser->current_token->kind != TOKEN_RPARENT) {
        ASTExpr* expr = parse_expr(parser);
        if (expr == NULL) return NULL;
        if (!ASTExprVec_push(&args, expr)) {
            printf("Pushing ast expr vec failed.. \n");
            exit(-1);
        };

        while (parser->current_token->kind == TOKEN_COMMA) {
            advance(parser);
            ASTExpr* expr = parse_expr(parser);
            if (expr == NULL) return NULL;
            if (!ASTExprVec_push(&args, expr)) {
                printf("Pushing ast expr vec failed.. \n");
                exit(-1);
            };
        }
    }
    if (!expect(parser, TOKEN_RPARENT)) return NULL;
    advance(parser);

    ASTExpr* new_expr = malloc(sizeof(ASTExpr));

    if (new_expr == NULL) {
        printf("Malloc failed.. \n");
        exit(-1);
    }

    new_expr->kind = AST_EXPR_CALL;
    new_expr->value.call.function_name = callee;
    new_expr->value.call.params = args;
    
    return new_expr;
}

static ASTExpr* parse_postfix(Parser *parser) {
    ASTExpr* expr = parse_primary(parser);
    if (expr == NULL) return NULL;

    while (1) {
        // <expr>(
        if (parser->current_token->kind == TOKEN_LPARENT) {
            advance(parser);
            ASTExpr* func_res = finish_call(parser, expr); 
            if (func_res == NULL) return NULL;
            expr = func_res;
        }
        // <expr>.
        else if (parser->current_token->kind == TOKEN_DOT) {
            advance(parser);
            if (!expect(parser, TOKEN_IDENT)) return NULL;
            const char* field_name = parser->current_token->value;

            advance(parser);

            ASTExpr* new_expr = malloc(sizeof(ASTExpr));

            if (new_expr == NULL) {
                printf("Malloc failed \n");
                exit(-1);
            }

            new_expr->kind = AST_EXPR_FIELD_ACCESS;
            new_expr->value.field_access.field_name = field_name;
            new_expr->value.field_access.obj = expr;
            expr = new_expr;
        }
        // <expr>[
        else if (parser->current_token->kind == TOKEN_LBRACKET) {
            advance(parser);
            ASTExpr* index = parse_expr(parser);
            if (index == NULL) return NULL;

            if (!expect(parser, TOKEN_RBRACKET)) return NULL;

            advance(parser);

            ASTExpr* new_expr = malloc(sizeof(ASTExpr));

            if (new_expr == NULL) {
                printf("Malloc failed \n");
                exit(-1);
            }

            new_expr->kind = AST_EXPR_POSTFIX;
            new_expr->value.postfix.op = POSTFIX_OP_BRACKETS;
            new_expr->value.postfix.obj = expr;
            new_expr->value.postfix.value = index;
            expr = new_expr;
        }
        else if (parser->current_token->kind == TOKEN_PLUSPLUS) {
            advance(parser);

            ASTExpr* new_expr = malloc(sizeof(ASTExpr));

            if (new_expr == NULL) {
                printf("Malloc failed \n");
                exit(-1);
            }

            new_expr->kind = AST_EXPR_POSTFIX;
            new_expr->value.postfix.op = POSTFIX_OP_PLUSPLUS;
            new_expr->value.postfix.obj = expr;
            new_expr->value.postfix.value = NULL;
            expr = new_expr;
        }
        else if (parser->current_token->kind == TOKEN_MINUSMINUS) {
            advance(parser);

            ASTExpr* new_expr = malloc(sizeof(ASTExpr));

            if (new_expr == NULL) {
                printf("Malloc failed \n");
                exit(-1);
            }

            new_expr->kind = AST_EXPR_POSTFIX;
            new_expr->value.postfix.op = POSTFIX_OP_MINUSMINUS;
            new_expr->value.postfix.obj = expr;
            new_expr->value.postfix.value = NULL;
            expr = new_expr;
        }
        else {
            break;
        }
    }
    return expr;
}


static ASTExpr* parse_unary(Parser *parser) {
    if (parser->current_token->kind == TOKEN_PLUS) {
        // 3 - +3 => + ist egal
        advance(parser);
    }

    UnaryOp op;

    
    if (parser->current_token->kind == TOKEN_MINUS) {
        op = UNARY_NEG;
    }
    else if(parser->current_token->kind == TOKEN_BANG) {
        op = UNARY_NOT;
    }
    else {
        return parse_postfix(parser);
    }

    advance(parser);

    ASTExpr* value = parse_unary(parser);

    if (value == NULL) return NULL;

    ASTExpr* unary_expr = malloc(sizeof(ASTExpr));

    if (unary_expr == NULL) {
        printf("Malloc failed.. \n");
        exit(-1);
    }

    unary_expr->kind = AST_EXPR_UNARY; 
    unary_expr->value.unary.op = op;
    unary_expr->value.unary.expr = value;

    return unary_expr;
}
static ASTExpr* parse_multiplacative(Parser *parser) {
    ASTExpr *lhs = parse_unary(parser);

    if (lhs == NULL) {
        return NULL;
    }

    while (parser->current_token->kind == TOKEN_STAR || parser->current_token->kind == TOKEN_SLASH ||parser->current_token->kind == TOKEN_PERCENT) {
        BinOp op;
        if (parser->current_token->kind == TOKEN_STAR) {
            op = BINOP_MUL;
        }
        else if (parser->current_token->kind == TOKEN_SLASH){
            op = BINOP_DIV;
        }
        else {
            op = BINOP_MOD;
        }
        advance(parser);

        ASTExpr *rhs = parse_unary(parser);

        if (rhs == NULL) {
            return NULL;
        }

        ASTExpr* new_lhs = malloc(sizeof(ASTExpr)); 
        if (new_lhs == NULL) {
            printf("Malloc failed \n");
            exit(-1);
        }
        new_lhs->kind = AST_EXPR_BINARY;
        new_lhs->value.binary.lhs = lhs;
        new_lhs->value.binary.rhs = rhs;
        new_lhs->value.binary.op = op;
        lhs = new_lhs;
    }
    return lhs;
}

static ASTExpr* parse_additive(Parser *parser) {
    ASTExpr *lhs = parse_multiplacative(parser);

    if (lhs == NULL) {
        return NULL;
    }

    while (parser->current_token->kind == TOKEN_PLUS || parser->current_token->kind == TOKEN_MINUS) {
        BinOp op;
        if (parser->current_token->kind == TOKEN_PLUS) {
            op = BINOP_ADD;
        }
        else {
            op = BINOP_SUB;
        }
        advance(parser);

        ASTExpr *rhs = parse_multiplacative(parser);

        if (rhs == NULL) {
            return NULL;
        }

        ASTExpr* new_lhs = malloc(sizeof(ASTExpr)); 
        if (new_lhs == NULL) {
            printf("Malloc failed \n");
            exit(-1);
        }
        new_lhs->kind = AST_EXPR_BINARY;
        new_lhs->value.binary.lhs = lhs;
        new_lhs->value.binary.rhs = rhs;
        new_lhs->value.binary.op = op;
        lhs = new_lhs;
    }
    return lhs;
}



static ASTExpr* parse_comparison(Parser *parser) {
    ASTExpr *lhs = parse_additive(parser);

    if (lhs == NULL) {
        return NULL;
    }

    TokenKind comparisons[4] = {TOKEN_LT, TOKEN_GT, TOKEN_GTEQ, TOKEN_LTEQ};

    while (contains_tokenkind(parser->current_token->kind, comparisons, 4)) {
        BinOp op;

        switch (parser->current_token->kind) {
            case TOKEN_LT:
                op = BINOP_LT;
            break;
            case TOKEN_GT:
                op = BINOP_GT;
            break;
            case TOKEN_GTEQ:
                op = BINOP_GE;
            break;
            case TOKEN_LTEQ:
                op = BINOP_LE;
            break;
            default: 
                printf("Shouldnt be here (parser.c, parse_comparison()) \n");
                exit(-1);
            break;
        }
        advance(parser);

        ASTExpr *rhs = parse_additive(parser);

        if (rhs == NULL) {
            return NULL;
        }

        ASTExpr* new_lhs = malloc(sizeof(ASTExpr)); 
        if (new_lhs == NULL) {
            printf("Malloc failed \n");
            exit(-1);
        }
        new_lhs->kind = AST_EXPR_BINARY;
        new_lhs->value.binary.lhs = lhs;
        new_lhs->value.binary.rhs = rhs;
        new_lhs->value.binary.op = op;
        lhs = new_lhs;
    }
    return lhs;
}


static ASTExpr* parse_equality(Parser *parser) {
    ASTExpr *lhs = parse_comparison(parser);
    if (lhs == NULL) {
        return lhs;
    }

    while (parser->current_token->kind == TOKEN_EQEQ || parser->current_token->kind == TOKEN_BANGEQ) {
        BinOp op;
        if (parser->current_token->kind == TOKEN_EQEQ) {
            op = BINOP_EQ;
        }
        else {
            op = BINOP_NE;
        }

        advance(parser);
        ASTExpr *rhs = parse_comparison(parser);

        if (rhs == NULL) {
            return NULL;
        }

        ASTExpr* new_lhs = malloc(sizeof(ASTExpr)); 
        if (new_lhs == NULL) {
            printf("Malloc failed \n");
            exit(-1);
        }
        new_lhs->kind = AST_EXPR_BINARY;
        new_lhs->value.binary.lhs = lhs;
        new_lhs->value.binary.rhs = rhs;
        new_lhs->value.binary.op = op;
        lhs = new_lhs;
    }
    return lhs;
}

static ASTExpr* parse_and(Parser *parser) {
    ASTExpr* lhs = parse_equality(parser);

    if (lhs == NULL) {
        return lhs;
    }

    while (parser->current_token->kind == TOKEN_AMP_AMP) {
        advance(parser);
        ASTExpr* rhs = parse_equality(parser);

        if (rhs == NULL) {
            return NULL;
        }

        ASTExpr* new_lhs = malloc(sizeof(ASTExpr)); 
        if (new_lhs == NULL) {
            printf("Malloc failed \n");
            exit(-1);
        }
        new_lhs->kind = AST_EXPR_BINARY;
        new_lhs->value.binary.lhs = lhs;
        new_lhs->value.binary.rhs = rhs;
        new_lhs->value.binary.op = BINOP_AND;
        lhs = new_lhs;

    }
    return lhs;
}

static ASTExpr* parse_or(Parser *parser) {
    ASTExpr* lhs = parse_and(parser);

    if (lhs == NULL) {
        return lhs;
    }

    while (parser->current_token->kind == TOKEN_PIPE_PIPE) {
        advance(parser);
        ASTExpr* rhs = parse_and(parser);

        if (rhs == NULL) {
            return NULL;
        }

        ASTExpr* new_lhs = malloc(sizeof(ASTExpr)); 
        if (new_lhs == NULL) {
            printf("Malloc failed \n");
            exit(-1);
        }
        new_lhs->kind = AST_EXPR_BINARY;
        new_lhs->value.binary.lhs = lhs;
        new_lhs->value.binary.rhs = rhs;
        new_lhs->value.binary.op = BINOP_OR;

        lhs = new_lhs;
    }
    return lhs;
}

static ASTExpr* parse_expr(Parser *parser) {
    ASTExpr* node = parse_or(parser);
    return node;
}

// -----------------------------
//          Alle Top level 
// -----------------------------


static bool parse_const(Parser *parser) {

    if (!expect(parser, TOKEN_IDENT)) return false;
    // V1 zeigt einfach direkt auf die value,
    // da TokenVec in main eh lang genug lebt
    // (könnte man optimieren)
    const char* name = parser->current_token->value;
    advance(parser);

    if (!expect(parser, TOKEN_COLON)) return false; 
    advance(parser);

    const char* type = parse_type(parser);
    if (type == NULL) return false;

    if (!expect(parser, TOKEN_EQ)) return false; 
    advance(parser);

    ASTExpr* value = parse_expr(parser);

    if (value == NULL) {
        return false;
    }

    if (!expect(parser, TOKEN_SEMICOLON)) return false;
    advance(parser);

    // memory auf dem heap allocaten, damit diese constant
    // länger lebt als eine lokale stack variable (lifetimes)
    ASTConst *c = malloc(sizeof *c);

    // nicht sauber aber für v1 okay
    if (c == NULL) {
        printf("Malloc failed in parse_const() \n");
        exit(1);
    }
    *c = (ASTConst){.value = value, .name = name, .type = type};

    if (!ASTConstVec_push(&parser->ast.constants, c)) {
        printf("Pushing Vec failed.. \n");
        exit(-1);
    }
    return true;
}

static ASTStmt* parse_if(Parser* parser) {
    if (!expect(parser, TOKEN_IF)) return NULL;
    advance(parser);

    if (!expect(parser, TOKEN_LPARENT)) return NULL;
    advance(parser);

    ASTExpr* cond = parse_expr(parser);
    if (cond == NULL) return NULL;

    if (!expect(parser, TOKEN_RPARENT)) return NULL;
    advance(parser);

    if (!expect(parser, TOKEN_LBRACE)) return NULL;
    advance(parser);

    ASTStmtBlock block = parse_block(parser);

    if (!expect(parser, TOKEN_RBRACE)) return NULL;
    advance(parser);

    ASTStmt* stmt = malloc(sizeof(ASTStmt));
    if (stmt == NULL) {
        printf("Malloc failed \n");
        exit(-1);
    }
    *stmt = (ASTStmt) {.kind = AST_STMT_IF};
    stmt->value.if_stmt.code_block = block;
    stmt->value.if_stmt.condition = cond;
    stmt->value.if_stmt.has_else = false;

    if (parser->current_token->kind == TOKEN_ELSE) {
        advance(parser);

        if (parser->current_token->kind == TOKEN_IF) {
            parse_if(parser);
        }
        else {
            if (!expect(parser, TOKEN_LBRACE)) return NULL;
            advance(parser);

            ASTStmtBlock block = parse_block(parser);

            if (!expect(parser, TOKEN_RBRACE)) return NULL;
            advance(parser);
            stmt->value.if_stmt.has_else = true;
            stmt->value.if_stmt.optional_else_block = block;
        }
    }

    return stmt;
}
static ASTStmt* parse_for(Parser* parser) {
    if (!expect(parser, TOKEN_FOR)) return NULL;
    advance(parser);

    if (!expect(parser, TOKEN_LPARENT)) return NULL;
    advance(parser);

    ASTStmt* init = parse_statement(parser);
    if (init == NULL) return NULL;
    ASTExpr* cond = parse_expr(parser);
    if (cond == NULL) return NULL;
    ASTStmt* action = parse_statement(parser);
    if (action == NULL) return NULL;

    if (!expect(parser, TOKEN_RPARENT)) return NULL;
    advance(parser);

    if (!expect(parser, TOKEN_LBRACE)) return NULL;
    advance(parser);

    ASTStmtBlock block = parse_block(parser);

    if (!expect(parser, TOKEN_RBRACE)) return NULL;
    advance(parser);

    ASTStmt* stmt = malloc(sizeof(ASTStmt));
    if (stmt == NULL) {
        printf("Malloc failed \n");
        exit(-1);
    }

    *stmt = (ASTStmt) {.kind = AST_STMT_FOR};
    stmt->value.for_stmt.condition = cond;
    stmt->value.for_stmt.init = init;
    stmt->value.for_stmt.code_block = block;
    stmt->value.for_stmt.action = action;

    return stmt;

}
static ASTStmt* parse_while(Parser* parser) {
    if (!expect(parser, TOKEN_WHILE)) return NULL;
    advance(parser);


    if (!expect(parser, TOKEN_LPARENT)) return NULL;
    advance(parser);

    ASTExpr* condition = parse_expr(parser);

    if (!expect(parser, TOKEN_RPARENT)) return NULL;
    advance(parser);

    if (!expect(parser, TOKEN_LBRACE)) return NULL;
    advance(parser);

    ASTStmtBlock block = parse_block(parser);

    if (!expect(parser, TOKEN_RBRACE)) return NULL;
    advance(parser);

    ASTStmt* stmt = malloc(sizeof(ASTStmt));
    if (stmt == NULL) {
        printf("Malloc failed \n");
        exit(-1);
    }

    *stmt = (ASTStmt) {.kind = AST_STMT_WHILE, .value.while_stmt.condition = condition, .value.while_stmt.code_block = block};
    return stmt;

}
static ASTStmt* parse_continue(Parser* parser) {
    if (!expect(parser, TOKEN_CONTINUE)) return NULL;
    advance(parser);
    
    if (!expect(parser, TOKEN_SEMICOLON)) return NULL;
    advance(parser);

    ASTStmt* stmt = malloc(sizeof(ASTStmt));
    if (stmt == NULL) {
        printf("Malloc failed \n");
        exit(-1);
    }
    *stmt = (ASTStmt) {.kind = AST_STMT_CONTINUE};
    return stmt;
}
static ASTStmt* parse_break(Parser* parser) {
    if (!expect(parser, TOKEN_BREAK)) return NULL;
    advance(parser);
    
    if (!expect(parser, TOKEN_SEMICOLON)) return NULL;
    advance(parser);

    ASTStmt* stmt = malloc(sizeof(ASTStmt));
    if (stmt == NULL) {
        printf("Malloc failed \n");
        exit(-1);
    }
    *stmt = (ASTStmt) {.kind = AST_STMT_BREAK};
    return stmt;
}
static ASTStmt* parse_new_scope(Parser* parser) {
    if (!expect(parser, TOKEN_LBRACE)) return NULL;
    advance(parser);

    ASTStmtBlock block = parse_block(parser);

    if (!expect(parser, TOKEN_RBRACE)) return NULL;
    advance(parser);

    ASTStmt* stmt = malloc(sizeof(ASTStmt));
    if (stmt == NULL) {
        printf("Malloc failed \n");
        exit(-1);
    }
    *stmt = (ASTStmt) {.kind = AST_STMT_BLOCK, .value.block_stmt = block};

    return stmt;
}

static ASTStmt* parse_return(Parser* parser) {
    if (!expect(parser, TOKEN_RETURN)) return NULL;
    advance(parser);

    bool has_return = true;

    ASTStmt* stmt = malloc(sizeof(ASTStmt));
    if (stmt == NULL) {
        printf("Malloc failed \n");
        exit(-1);
    }

    if (parser->current_token->kind == TOKEN_SEMICOLON) {
        has_return = false;
        advance(parser);
        stmt->value.return_stmt.value = NULL;
    }
    else {
        ASTExpr* value = parse_expr(parser);
        if (value == NULL)  return NULL;

        if (!expect(parser, TOKEN_SEMICOLON)) return NULL;
        advance(parser);

        stmt->value.return_stmt.value = value;
    }

    stmt->kind = AST_STMT_RETURN;
    stmt->value.return_stmt.has_return_value = has_return;

    return stmt;
}

static ASTStmt* parse_let(Parser* parser) {
    if (!expect(parser, TOKEN_LET)) return NULL;
    advance(parser);

    if (!expect(parser, TOKEN_IDENT)) return NULL;
    const char* name = parser->current_token->value;
    advance(parser);

    if (!expect(parser, TOKEN_COLON)) return NULL;
    advance(parser);

    // parse type handelt selber die errors und advanced selber
    const char* type = parse_type(parser);
    if (type == NULL) return NULL;


    if (!expect(parser, TOKEN_EQ)) return NULL;
    advance(parser);

    ASTExpr* value = parse_expr(parser);
    if (value == NULL)  return NULL;

    if (!expect(parser, TOKEN_SEMICOLON)) return NULL;
    advance(parser);

    ASTStmt* stmt = malloc(sizeof(ASTStmt));

    if (stmt == NULL) {
        printf("Malloc failed \n");
        exit(-1);
    }

    stmt->kind = AST_STMT_LET;
    stmt->value.let_stmt.value = value;
    stmt->value.let_stmt.var_name = name;
    stmt->value.let_stmt.var_type = type;

    return stmt;
}


static ASTStmt* parse_statement(Parser* parser) {
    switch (parser->current_token->kind) {
        case TOKEN_LET:
            return parse_let(parser);
        case TOKEN_IF:
            return parse_if(parser);
        break;
        case TOKEN_FOR:
            return parse_for(parser);
        break;
        case TOKEN_WHILE:
            return parse_while(parser);
        break;
        case TOKEN_RETURN:
            return parse_return(parser);
        break;
        case TOKEN_LBRACE:
            return parse_new_scope(parser);
        break;
        case TOKEN_CONTINUE:
            return parse_continue(parser);
        break;
        case TOKEN_BREAK:
            return parse_break(parser);
        break;
        default: 
            parser->parser_error = (ParserError){.got = parser->current_token->kind, .status = PARSER_ERR_UNEXPECTED_STMT_START, .token_pos = parser->pos};
            return NULL;
        break;
    }
}

static bool current_is_statement(Parser *parser) {
    TokenKind stmt_kinds[] = {TOKEN_LET, TOKEN_IF, TOKEN_WHILE, TOKEN_FOR, TOKEN_BREAK, TOKEN_CONTINUE, TOKEN_RETURN, TOKEN_LBRACE};
    return contains_tokenkind(parser->current_token->kind, stmt_kinds, 8); 
}

static ASTStmtBlock parse_block(Parser* parser) {
    ASTStmtVec statements;
    ASTStmtVec_init(&statements);


    while (parser->current_token->kind != TOKEN_RBRACE) {
        if (current_is_statement(parser)) {
            ASTStmt* stmt = parse_statement(parser);
            if (stmt == NULL) break;
            if (!ASTStmtVec_push(&statements, stmt)) {
                printf("pushing vec failed \n");
                exit(-1);
            }
        }
        else {
            // Statement expr, zb: 1+1;
            ASTExpr* expr = parse_expr(parser);
            if (expr == NULL) break;
            if (!expect(parser, TOKEN_SEMICOLON)) break;
            advance(parser);
            ASTStmt* stmt = malloc(sizeof(ASTStmt));
            if (stmt == NULL) {
                printf("malloc failed \n");
                exit(-1);
            }
            *stmt = (ASTStmt) {.kind = AST_STMT_EXPR, .value.expr_stmt.expr = expr};
            if (!ASTStmtVec_push(&statements, stmt)) {
                printf("Pushing vec failed \n");
                exit(-1);
            } 
        }
    }

    return (ASTStmtBlock) {.statements = statements};
}

static bool parse_fn(Parser *parser) {
    // fn add(int a, int b) => int {..}
    if (!expect(parser, TOKEN_IDENT)) return false;
    const char* function_name = parser->current_token->value;
    advance(parser);

    if (!expect(parser, TOKEN_LPARENT)) return false;
    advance(parser);

    ASTTypeNameVec params;
    ASTTypeNameVec_init(&params);

    while (parser->current_token->kind != TOKEN_RPARENT) {
        // name
        if (!expect(parser, TOKEN_IDENT)) return false;
        const char* param_name = parser->current_token->value;
        advance(parser);

        // :
        if (!expect(parser, TOKEN_COLON)) return false;
        advance(parser);

        const char* type = parse_type(parser);
        if (type == NULL) return false;

        ASTTypeName* tn = malloc(sizeof(ASTTypeName));
        if (tn == NULL)  {
            printf("Malloc failed \n");
            exit(-1);
        }

        *tn = (ASTTypeName) {.type = type, .name = param_name};

        if (!ASTTypeNameVec_push(&params, tn)) {
            printf("Vec push failed \n");
            exit(-1);
        }

        if (parser->current_token->kind == TOKEN_RPARENT) {
            break;
        }
        if (!expect(parser, TOKEN_COMMA)) return false;
        advance(parser);
    }

    if (!expect(parser, TOKEN_RPARENT)) return false;
    advance(parser);

    if (!expect(parser, TOKEN_FATARROW)) return false;
    advance(parser);

    const char* return_type = parse_type(parser);
    if (return_type == NULL) return false;


    if (!expect(parser, TOKEN_LBRACE)) return false;
    advance(parser);

    ASTStmtBlock block = parse_block(parser);

    if (parser->parser_error.status != PARSER_OK) {
        return false;
    }

    if (!expect(parser, TOKEN_RBRACE)) return false;
    advance(parser);

    ASTFunction *f = malloc(sizeof (ASTFunction));

    if (f == NULL) {
        printf("Malloc failed in parse_fn() \n");
        exit(1);
    }
    *f = (ASTFunction) {.name = function_name, .block = block, .params = params, .return_type = return_type};

    if (!ASTFunctionVec_push(&parser->ast.functions, f)) {
        printf("Pushing Vec failed.. \n");
        exit(-1);
    }
    return true;

}

static bool parse_struct(Parser *parser) {
    if (!expect(parser, TOKEN_IDENT)) return false;
    const char* struct_name = parser->current_token->value;
    advance(parser);

    if (!expect(parser, TOKEN_LBRACE)) return false;
    advance(parser);

    ASTTypeNameVec fields;
    ASTTypeNameVec_init(&fields);

    while (parser->current_token->kind != TOKEN_RBRACE) {
        if (!expect(parser, TOKEN_IDENT)) return false;
        const char* field_name = parser->current_token->value;
        advance(parser);

        // :
        if (!expect(parser, TOKEN_COLON)) return false;
        advance(parser);

        const char* field_type = parse_type(parser);
        if (field_type == NULL) return false;

        ASTTypeName* tn = malloc(sizeof(ASTTypeName));
        if (tn == NULL)  {
            printf("Malloc failed \n");
            exit(-1);
        }

        *tn = (ASTTypeName) {.type = field_type, .name = field_name};

        if (!ASTTypeNameVec_push(&fields, tn)) {
            printf("Vec push failed \n");
            exit(-1);
        }

        if (parser->current_token->kind == TOKEN_RBRACE) {
            break;
        }
        if (!expect(parser, TOKEN_COMMA)) return false;
        advance(parser);

    }

    if (!expect(parser, TOKEN_RBRACE)) return false;
    advance(parser);

    ASTStructDef *struct_def = malloc(sizeof (ASTStructDef));

    if (struct_def == NULL) {
        printf("Malloc failed in parse_fn() \n");
        exit(1);
    }
    *struct_def = (ASTStructDef) {.fields = fields, .name=struct_name};

    if (!ASTStructDefVec_push(&parser->ast.struct_defs, struct_def)) {
        printf("Pushing Vec failed.. \n");
        exit(-1);
    }
    return true;
}
static bool parse_enum(Parser *parser) {
    if (!expect(parser, TOKEN_IDENT)) return false;
    const char* enum_name = parser->current_token->value;
    advance(parser);

    if (!expect(parser, TOKEN_LBRACE)) return false;
    advance(parser);


    StrVec case_names;
    StrVec_init(&case_names);

    while (parser->current_token->kind != TOKEN_RBRACE) {
        if (!expect(parser, TOKEN_IDENT)) return false;
        const char* case_name = parser->current_token->value;
        advance(parser);

        StrVec_push(&case_names, case_name);

        if (parser->current_token->kind == TOKEN_RBRACE) {
            break;
        }

        if (!expect(parser, TOKEN_COMMA)) return false;
        advance(parser);
    }

    if (!expect(parser, TOKEN_RBRACE)) return false;
    advance(parser);

    ASTEnumDef *enum_def = malloc(sizeof (ASTEnumDef));

    if (enum_def == NULL) {
        printf("Malloc failed in parse_fn() \n");
        exit(1);
    }
    *enum_def = (ASTEnumDef) {.cases = case_names, .name = enum_name};

    if (!ASTEnumDefVec_push(&parser->ast.enum_defs, enum_def)) {
        printf("Pushing Vec failed.. \n");
        exit(-1);
    }
    return true;
}

ParserResult parse_tokens(const TokenVec* tokens) {
    ASTConstVec const_vec;
    ASTConstVec_init(&const_vec);

    ASTFunctionVec func_vec;
    ASTFunctionVec_init(&func_vec);

    ASTStructDefVec structdef_vec;
    ASTStructDefVec_init(&structdef_vec);

    ASTEnumDefVec enumdef_vec;
    ASTEnumDefVec_init(&enumdef_vec);
    
    AST new_ast = {.constants = const_vec, .functions = func_vec, .struct_defs = structdef_vec,.enum_defs = enumdef_vec};

    Parser parser = {.tokens = tokens, .ast = new_ast, .parser_error = {0},.current_token = &tokens->data[0], .pos = 0};


    while (parser.current_token->kind != TOKEN_EOF) {
         switch (parser.current_token->kind) {
            case TOKEN_FN:
                advance(&parser);
                if (!parse_fn(&parser)) {
                    return (ParserResult) {.ast = parser.ast, .error = parser.parser_error};
                }
            break;
            case TOKEN_CONST:
                advance(&parser);

                // parse const returnt ob alles gut gelaufen ist oder nicht,
                // falls false => error wurde in parser.parser_error geschrieben
                if (!parse_const(&parser)) {
                    return (ParserResult) {.ast = parser.ast, .error = parser.parser_error};
                }
            break;
            case TOKEN_STRUCT:
                advance(&parser);
                if (!parse_struct(&parser)) {
                    return (ParserResult) {.ast = parser.ast, .error = parser.parser_error};
                }
            break;
            case TOKEN_ENUM:
                advance(&parser);
                if (!parse_enum(&parser)) {
                    return (ParserResult) {.ast = parser.ast, .error = parser.parser_error};
                }
            break;
            default:
                return (ParserResult) {.ast = parser.ast, .error = {.token_pos = parser.pos, .status = PARSER_ERR_UNEXPECTED_TOP_LEVEL, .got = parser.current_token->kind}};
            break;
        }
    }

    return (ParserResult) {.ast = parser.ast, .error = parser.parser_error};
}
