#include"../include/lexer.h"
#include"../include/parser.h"
#include"../include/ast.h"
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static ASTExpr* parse_expr(Parser* parser);

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

static ASTExpr* parse_primary(Parser *parser) {
    printf("parse primary wurde erreicht. \n");
    ASTExpr* node = malloc(sizeof(ASTExpr));
    printf("Recieved current token kind: %s \n", token_kind_name(parser->current_token->kind));
    printf("token value: %s \n", parser->current_token->value);
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
            *node = (ASTExpr){.kind = AST_EXPR_STRING_LITERAL, .value.literal_value = parser->current_token->value};   
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

                ASTNameExprVec* fields = malloc(sizeof(ASTNameExprVec));
                ASTNameExprVec_init(fields);


                advance(parser); // auf dem {
                advance(parser); // eventuell auf einem field_name, kann aber auch schon } sein. 

                while (1) {
                    if (parser->current_token->kind == TOKEN_RBRACE) {
                        advance(parser);
                        break;
                    }
                    if (!expect(parser, TOKEN_IDENT)) return NULL;
                    char* field_name = parser->current_token->value;
                    advance(parser);

                    if (!expect(parser, TOKEN_COLON)) return NULL;
                    advance(parser);

                    if (!expect(parser, TOKEN_IDENT)) return NULL;
                    advance(parser);
                    ASTExpr* value = parse_expr(parser);

                    ASTNameExpr *p = malloc(sizeof(ASTNameExpr));
                    *p = (ASTNameExpr) {.name = field_name, .expr = value};
                    ASTNameExprVec_push(fields, p);

                    if (parser->current_token->kind == TOKEN_RBRACE) {
                        advance(parser);
                        break;
                    }
                    if (!expect(parser, TOKEN_COMMA)) return NULL;
                    advance(parser);
                }
                *node = (ASTExpr){.kind = AST_EXPR_STRUCT_LITERAL, .value.struct_literal = struct_literal};   


            }
            // ident::ident ==> enum
            else if (peek(parser)->kind == TOKEN_COLONCOLON && peek2(parser)->kind == TOKEN_IDENT) {
                char* enum_name = parser->current_token->value; 
                advance(parser);
                advance(parser);
                char* case_name = parser->current_token->value; 
                ASTExprEnumLiteral elit = {.enum_name = enum_name, .case_name = case_name};
               *node = (ASTExpr){.kind = AST_EXPR_ENUM_LITERAL, .value.enum_literal = elit};   
                advance(parser);
            }
            // ident ==> variable
            else {
                *node = (ASTExpr){.kind = AST_EXPR_VARIABLE, .value.variable_name = parser->current_token->value};   
            }
        break;
        case TOKEN_LPARENT:
            advance(parser);
            ASTExpr* inner = parse_expr(parser);
            if (!expect(parser, TOKEN_RPARENT)) return NULL;
            advance(parser);


        break;
        case TOKEN_LBRACKET:
            advance(parser);
            ASTExprVec elements;
            ASTExprVec_init(&elements);

            if (parser->current_token->kind != TOKEN_RBRACKET) {
                ASTExprVec_push(&elements, parse_expr(parser));

                while(parser->current_token->kind == TOKEN_COMMA) {
                    advance (parser);
                    ASTExprVec_push(&elements, parse_expr(parser));
                }
            }
            expect(parser, TOKEN_RBRACKET);
            advance(parser);
            *node = (ASTExpr){.kind = AST_EXPR_LIST_LITERAL, .value.list_literal = elements};   

        break;
        default: 
            printf("unknwon literal found... \n");
            parser->parser_error = (ParserError){.got = parser->current_token->kind, .token_pos = parser->pos, .status = PARSER_ERR_UNEXPECTED_EXPR_START} ;
            return NULL;
    }
    printf("parse_primary() hat folgende Node erstellt und allocated: kind = %d, value = %s", node->kind, node->value.literal_value);
    printf("Alles successful in primary, returning back now.. \n");

    if (node == NULL) {
        printf("node ist irgendwie null ist parse_primary.");
        exit(-1);
    }

    return node;
}
static ASTExpr* finish_call(Parser *parser) {
}
static ASTExpr* parse_postfix(Parser *parser) {
}
static ASTExpr* parse_unary(Parser *parser) {
}

static ASTExpr* parse_multiplacative(Parser *parser) {
}


static ASTExpr* parse_additive(Parser *parser) {
}


static bool contains_tokenkind(TokenKind value, TokenKind *arr, size_t len) {
    for (size_t i = 0; i < len; i++) {
        if (arr[i] == value) {
            return true;
        }
    }
    return false;
}

static ASTExpr* parse_comparison(Parser *parser) {
    ASTExpr *lhs = parse_primary(parser);

    if (lhs == NULL) {
        printf("lhs in parse_comparison ist null, retuning.. \n");
        return lhs;
    }

    TokenKind comparisons[4] = {TOKEN_LT, TOKEN_GT, TOKEN_GTEQ, TOKEN_LTEQ};

    while (contains_tokenkind(parser->current_token->kind, comparisons, 4)) {
        printf("Found comparison, parsing rhs now. \n");
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

        ASTExpr *rhs = parse_primary(parser);

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
        printf("lhs ist null in parse_equality, returning..\n");
        return lhs;
    }

    while (parser->current_token->kind == TOKEN_EQEQ || parser->current_token->kind == TOKEN_BANGEQ) {
        printf("Equality wurde gefunden, parse rhs.. \n");
        BinOp op;
        if (parser->current_token->kind == TOKEN_EQEQ) {
            op = BINOP_EQ;
        }
        else {
            op = BINOP_NE;
        }

        advance(parser);
        ASTExpr *rhs = parse_comparison(parser);

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
    printf("Wir sind jetzt in parse_and() \n");
    ASTExpr* lhs = parse_equality(parser);

    if (lhs == NULL) {
        printf("LHS in parse_and() ist null, retuning.. \n");
        return lhs;
    }

    while (parser->current_token->kind == TOKEN_AMP_AMP) {
        printf("Fond amp amp, parsing RHS in parse_and() now \n");
        advance(parser);
        ASTExpr* rhs = parse_equality(parser);

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
        printf("LHS ist NULL in parse_or(), returning.. \n");
        return lhs;
    }

    while (parser->current_token->kind == TOKEN_PIPE_PIPE) {
        printf("Pipe pipe wurde gefunden, parse rhs now.. \n");
        advance(parser);
        ASTExpr* rhs = parse_and(parser);

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
    printf("Wir sind in parse_expr, da diese methode nur ein wrapper ist wird einfach jetzt direkt parse_or aufgerufen! \n");
    ASTExpr* node = parse_or(parser);
    printf("Parse or ist fertig, wir sind zurück in parse_expr() \n");
    return node;
}

// -----------------------------
//          Alle Top level 
// -----------------------------

static void parse_fn(Parser *parser) {

}

static void parse_const(Parser *parser) {

    if (!expect(parser, TOKEN_IDENT)) return;
    // V1 zeigt einfach direkt auf die value,
    // da TokenVec in main eh lang genug lebt
    // (könnte man optimieren)
    char* name = parser->current_token->value;
    advance(parser);

    if (!expect(parser, TOKEN_COLON)) return; 
    advance(parser);

    if (!expect(parser, TOKEN_IDENT)) return; 
    char* type = parser->current_token->value;
    advance(parser);

    if (!expect(parser, TOKEN_EQ)) return; 
    advance(parser);

    printf("Syntax war okay bis expr, parse expr now! \n");
    ASTExpr* value = parse_expr(parser);
    printf("Parse expr ist jetzt fertig, wir sind wieder in parse_const angekommen. \n");

    if (value == NULL) {
        printf("Die expr value ist aber null?!?!? \n");
    }


    if (!expect(parser, TOKEN_SEMICOLON)) return;
    advance(parser);

    // memory auf dem heap allocaten, damit diese constant
    // länger lebt als eine lokale stack variable (lifetimes)
    ASTConst *c = malloc(sizeof *c);

    // nicht sauber aber für v1 okay
    if (c == NULL) {
        printf("Malloc failed in parse_const() \n");
        exit(1);
    }
    printf("memory für die ASTConst wurde allocated");
    *c = (ASTConst){.value = value, .name = name, .type = type};

    ASTConstVec_push(&parser->ast.constants, c);
    printf("In den ASTConst vec wurde der ASTConst pointer gepusht. \n");
}

static void parse_struct(Parser *parser) {

}
static void parse_enum(Parser *parser) {

}


// entry functionj
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
                parse_fn(&parser);
            break;
            case TOKEN_CONST:
                printf("Found token const on top level, parsing const now..\n");
                advance(&parser);
                parse_const(&parser);
            break;
            case TOKEN_STRUCT:
                advance(&parser);
                parse_struct(&parser);
            break;
            case TOKEN_ENUM:
                advance(&parser);
                parse_enum(&parser);
            break;
            default:
                return (ParserResult) {.ast = parser.ast, .error = {.token_pos = parser.pos, .status = PARSER_ERR_UNEXPECTED_TOP_LEVEL, .got = parser.current_token->kind}};
            break;
        }
    }

    return (ParserResult) {.ast = parser.ast, .error = parser.parser_error};
}
