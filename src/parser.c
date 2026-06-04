#include"../include/lexer.h"
#include"../include/parser.h"
#include"../include/ast.h"
#include <math.h>

static bool expect(Parser *parser, TokenKind kind) {
    if (parser->current_token->kind != kind) {
        parser->parser_error = (ParserError) {.expected = kind, .got = parser->current_token->kind, .token_pos = parser->pos, .status = PARSER_ERR_UNEXPECTED_TOKEN};
        return false;
    }
    return true;
}

static Token *peek(Parser *parser) {
    return &parser->tokens->data[parser->pos + 1];
}

static void advance(Parser *parser) {
    parser->pos += 1;
    parser->current_token = &parser->tokens->data[parser->pos];
}


static void parse_fn(Parser *parser) {

}

static ASTExpr* parse_expr(Parser *parser) {

}

static void parse_const(Parser *parser) {
    if (!expect(parser, TOKEN_IDENT)) return;
    char* name = parser->current_token->value;
    advance(parser);

    if (!expect(parser, TOKEN_COLON)) return; 
    advance(parser);

    if (!expect(parser, TOKEN_IDENT)) return; 
    char* type = parser->current_token->value;
    advance(parser);

    if (!expect(parser, TOKEN_EQ)) return; 
    advance(parser);

    ASTExpr* value = parse_expr(parser);

    if (!expect(parser, TOKEN_SEMICOLON)) return;
    advance(parser);

    ASTConst c = {.value = value, .name = name, .type = type};
    ASTConstVec_push(&parser->ast.constants, &c);
}

static void parse_struct(Parser *parser) {

}
static void parse_enum(Parser *parser) {

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
                parse_fn(&parser);
            break;
            case TOKEN_CONST:
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
