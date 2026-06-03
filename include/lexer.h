#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>



typedef enum {
    True,
    False,
    Ident,
    IntNumber,
    FloatNumber,
    StringLiteral,
    Char,

    If,
    Else,
    Return,
    While,
    Let,
    Fn,
    In,
    Struct,
    Const,
    For,
    Continue,
    Break,
    Enum,

    Underscore,

    AmpAmp,
    Amp,
    Pipe,
    PipePipe,

    LParen,
    RParen,
    LBrace,
    RBrace,
    Bang,
    LBracket,
    RBracket,
    Comma,
    Semicolon,
    Colon,
    Dot,

    Eq,
    PlusEq,
    MinusEq,
    StarEq,
    SlashEq,
    FatArrow,
    PlusPlus,
    MinusMinus,

    EqEq,
    BangEq,
    Lt,
    LtEq,
    Gt,
    GtEq,

    Plus,
    Minus,
    Star,
    Slash,
    Percent,

    Eof,

} TokenKind;

typedef struct {
    TokenKind kind;
    const char* value;
} Token;

typedef struct {
    Token* data;
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
    LEXER_ERR_INTERNAL_PUSH_ERROR,
    LEXER_ERR_UNTERMINATED_BLOCK,
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
    const char* input;
    TokenVec tokens;
    LexError err_status;
} Lexer;

LexResult generate_tokens(const char* str_input);
const char* token_kind_name(TokenKind kind);
void print_token_kind(TokenKind kind);
int free_token_vec(TokenVec* vec);

#endif
