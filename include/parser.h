#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"
#include <stddef.h>

typedef enum {
    PARSER_OK,
    PARSER_ERR_UNEXPECTED_TOP_LEVEL,
    PARSER_ERR_UNEXPECTED_EOF
} ParserStatus;

typedef struct {
    ParserStatus status;
    size_t token_pos;
    TokenKind expected;
    TokenKind got;
    const char* message;
} ParserError;

typedef struct {
    const TokenVec* tokens;
    size_t pos;
    Token* current_token;
    ParserError parser_error;
    ASTNode* root;
} Parser;

typedef struct {
    ParserError error;
    ASTNode* ast_root;
} ParserResult;

ParserResult parse_tokens(const TokenVec* tokens);


#endif
