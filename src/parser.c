#include"../include/lexer.h"
#include"../include/parser.h"
#include"../include/ast.h"


ParserResult parse_tokens(const TokenVec* tokens) {
    ParserError e = {0};
    e.status = PARSER_OK;
    ASTNode root = {.kind =  ASTKIND_ROOT, .value = {{0}}};
    Parser parser = {.tokens = tokens, .parser_error = e, .pos = 0, .current_token = &tokens->data[0], .root = &root};

    return (ParserResult) {.ast_root = parser.root, .error = parser.parser_error};
}

