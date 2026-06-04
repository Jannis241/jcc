#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>

typedef enum {
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_IDENT,
    TOKEN_INT,
    TOKEN_FLOAT,
    TOKEN_STRING,
    TOKEN_CHAR,

    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_RETURN,
    TOKEN_WHILE,
    TOKEN_LET,
    TOKEN_FN,
    TOKEN_IN,
    TOKEN_STRUCT,
    TOKEN_CONST,
    TOKEN_FOR,
    TOKEN_CONTINUE,
    TOKEN_BREAK,
    TOKEN_ENUM,

    TOKEN_AMP_AMP,
    TOKEN_AMP,
    TOKEN_PIPE,
    TOKEN_PIPE_PIPE,

    TOKEN_LPARENT,
    TOKEN_RPARENT,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_BANG,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_COLON,
    TOKEN_DOT,

    TOKEN_EQ,
    TOKEN_PLUSEQ,
    TOKEN_MINUSEQ,
    TOKEN_STAREQ,
    TOKEN_SLASHEQ,
    TOKEN_FATARROW,
    TOKEN_PLUSPLUS,
    TOKEN_MINUSMINUS,

    TOKEN_EQEQ,
    TOKEN_BANGEQ,
    TOKEN_LT,
    TOKEN_LTEQ,
    TOKEN_GT,
    TOKEN_GTEQ,

    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_STAR,
    TOKEN_SLASH,
    TOKEN_PERCENT,

    TOKEN_EOF,

} TokenKind;

typedef struct {
    TokenKind kind;
    char *value;
} Token;

typedef struct {
    Token *data;
    size_t num_of_tokens;
    size_t token_capacity;
} TokenVec;

typedef enum {
    LEXER_OK,
    LEXER_ERR_INVALID_CHARACTER,
    LEXER_ERR_UNEXPECTED_EOF,
    LEXER_ERR_INVALID_CHAR_LITERAL_SPAN,
    LEXER_ERR_UNTERMINATED_STRING,
    LEXER_ERR_UNTERMINATED_CHAR,
    LEXER_ERR_INVALID_FLOAT,
    LEXER_ERR_INTERNAL_PUSH_ERROR,
} LexStatus;

typedef struct {
    LexStatus status;
    size_t pos;
    char character;
} LexError;

typedef struct {
    TokenVec tokens;
    LexError error;
} LexResult;

typedef struct {
    size_t pos;
    char current_char;
    const char *input;
    size_t input_len;
    TokenVec tokens;
    LexError err_status;
} Lexer;

LexResult generate_tokens(const char *str_input);
const char *token_kind_name(TokenKind kind);
void print_token_kind(TokenKind kind);
int free_token_vec(TokenVec *vec);

#endif
