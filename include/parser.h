#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include <stddef.h>

typedef enum {
    PARSER_OK,
    PARSER_ERR_UNEXPECTED_TOP_LEVEL,
    PARSER_ERR_UNEXPECTED_EOF,
    PARSER_ERR_UNEXPECTED_TOKEN,
    PARSER_ERR_UNEXPECTED_EXPR_START,
    PARSER_ERR_INVALID_CHAR_LITERAL,
    PARSER_ERR_UNEXPECTED_STMT_START,

} ParserStatus;

typedef struct {
    ParserStatus status;
    size_t token_pos;
    TokenKind expected;
    TokenKind got;
} ParserError;

typedef struct {
    const TokenVec* tokens;
    size_t pos;
    const Token* current_token;
    ParserError parser_error;
    AST ast;
} Parser;

typedef struct {
    ParserError error;
    AST ast;
} ParserResult;

ParserResult parse_tokens(const TokenVec* tokens);

#endif
