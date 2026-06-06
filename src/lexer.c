#include "../include/lexer.h"
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* token_kind_name(TokenKind kind) {
    switch (kind) {
        case TOKEN_TRUE: return "True";
        case TOKEN_LTLT: return "LtLt";
        case TOKEN_GTGT: return "GtGt";
        case TOKEN_TYPE: return "Type";
        case TOKEN_AS: return "As";
        case TOKEN_MATCH: return "Match";
        case TOKEN_FALSE: return "False";
        case TOKEN_IDENT: return "Ident";
        case TOKEN_INT: return "IntNumber";
        case TOKEN_FLOAT: return "FloatNumber";
        case TOKEN_STRING: return "StringLiteral";
        case TOKEN_CHAR: return "Char";
        case TOKEN_IF: return "If";
        case TOKEN_ELSE: return "Else";
        case TOKEN_RETURN: return "Return";
        case TOKEN_WHILE: return "While";
        case TOKEN_LET: return "Let";
        case TOKEN_FN: return "Fn";
        case TOKEN_STRUCT: return "Struct";
        case TOKEN_CONST: return "Const";
        case TOKEN_FOR: return "For";
        case TOKEN_CONTINUE: return "Continue";
        case TOKEN_BREAK: return "Break";
        case TOKEN_ENUM: return "Enum";
        case TOKEN_AMP_AMP: return "AmpAmp";
        case TOKEN_AMP: return "Amp";
        case TOKEN_CARET: return "Caret";
        case TOKEN_PIPE: return "Pipe";
        case TOKEN_PIPE_PIPE: return "PipePipe";
        case TOKEN_LPARENT: return "LParen";
        case TOKEN_RPARENT: return "RParen";
        case TOKEN_LBRACE: return "LBrace";
        case TOKEN_RBRACE: return "RBrace";
        case TOKEN_BANG: return "Bang";
        case TOKEN_LBRACKET: return "LBracket";
        case TOKEN_RBRACKET: return "RBracket";
        case TOKEN_COMMA: return "Comma";
        case TOKEN_SEMICOLON: return "Semicolon";
        case TOKEN_COLON: return "Colon";
        case TOKEN_COLONCOLON: return "ColonColon";
        case TOKEN_DOT: return "Dot";
        case TOKEN_EQ: return "Eq";
        case TOKEN_PLUSEQ: return "PlusEq";
        case TOKEN_MINUSEQ: return "MinusEq";
        case TOKEN_STAREQ: return "StarEq";
        case TOKEN_SLASHEQ: return "SlashEq";
        case TOKEN_FATARROW: return "FatArrow";
        case TOKEN_PLUSPLUS: return "PlusPlus";
        case TOKEN_MINUSMINUS: return "MinusMinus";
        case TOKEN_EQEQ: return "EqEq";
        case TOKEN_BANGEQ: return "BangEq";
        case TOKEN_LT: return "Lt";
        case TOKEN_LTEQ: return "LtEq";
        case TOKEN_GT: return "Gt";
        case TOKEN_GTEQ: return "GtEq";
        case TOKEN_PLUS: return "Plus";
        case TOKEN_MINUS: return "Minus";
        case TOKEN_STAR: return "Star";
        case TOKEN_SLASH: return "Slash";
        case TOKEN_PERCENT: return "Percent";
        case TOKEN_EOF: return "Eof";
    }

    return "Unknown";
}

void print_token_kind(TokenKind kind) {
    printf("%s\n", token_kind_name(kind));
}

static TokenVec create_token_Vec(void) {
    return (TokenVec) {.data = NULL, .token_capacity = 0, .num_of_tokens = 0};
}

static char* copy_token_value(const char* value) {
    size_t len = strlen(value);
    char* copy = malloc(len + 1);

    if (copy == NULL) {
        return NULL;
    }

    memcpy(copy, value, len + 1);
    return copy;
}
static LexError gen_lexerror(Lexer *lexer, LexStatus status) {
    return (LexError){
        .pos = lexer->pos,
        .span = {
            .start = lexer->pos,
            .end = lexer->pos + 1,
            .line = lexer->line,
            .column = lexer->column,
        },
        .character = lexer->current_char,
        .status = status,
    };
}


static int push_token(TokenVec* vec, Token* token) {
    if (vec == NULL || token == NULL) {
        printf("[ERROR] Recieved NULL Vec or Token in PushToken()");
        return 1;
    }

    if (vec->num_of_tokens + 1 > vec->token_capacity) {
        vec->token_capacity *= 2;

        if (vec->token_capacity == 0) {
            vec->token_capacity = 256;
        }

        Token* neue_memeory = realloc(vec->data, vec->token_capacity * sizeof(Token));

        if (neue_memeory == NULL) {
            printf("[ERROR] Failed to realloc memory while trying to push tokens to TokenVec \n");
            return 2;
        }

        vec->data = neue_memeory;
    }

    char* value = copy_token_value(token->value);
    if (value == NULL) {
        printf("[ERROR] Failed to allocate memory while trying to copy token value \n");
        return 2;
    }

    vec->data[vec->num_of_tokens] = *token;
    vec->data[vec->num_of_tokens].value = value;
    vec->num_of_tokens += 1;
    return 0;
}

static void push_lexer_token(Lexer* lexer, Token token) {
    if (token.span.line == 0) {
        token.span = (SourceSpan) {
            .start = lexer->token_start_pos,
            .end = lexer->pos,
            .line = lexer->token_start_line,
            .column = lexer->token_start_column,
        };

        if (token.span.end <= token.span.start) {
            token.span.end = token.span.start + 1;
        }
    }

    if (push_token(&lexer->tokens, &token) != 0) {
        lexer->err_status = gen_lexerror(lexer, LEXER_ERR_INTERNAL_PUSH_ERROR);
    }
}

static char peek(Lexer* lexer) {
    if (lexer->pos + 1 >= lexer->input_len) {
        return '\0';
    }
    return lexer->input[lexer->pos + 1];
}

static void advance(Lexer* lexer) {
    if (lexer->current_char == '\n') {
        lexer->line += 1;
        lexer->column = 1;
    }
    else {
        lexer->column += 1;
    }

    lexer->pos += 1;
    lexer->current_char = lexer->input[lexer->pos];
}


static void skip_whitespaces(Lexer* lexer) {
    while (lexer->current_char && isspace((unsigned char)lexer->current_char)) {
        advance(lexer);
    }
}

static bool handle_escape_char(Lexer* lexer, char* c) {
    switch (lexer->current_char) {
        case 'n':
            *c = '\n';
            return true;
        case 't':
            *c = '\t';
            return true;
        case 'r':
            *c = '\r';
            return true;
        case '\\':
            *c = '\\';
            return true;
        case '\'':
            *c = '\'';
            return true;
        case '"':
            *c = '"';
            return true;
        default:
            lexer->err_status = gen_lexerror(lexer, LEXER_ERR_INVALID_CHARACTER);
            return false;
    }
}

static void handle_char(Lexer* lexer){
    advance(lexer);

    if (!lexer->current_char) {
        lexer->err_status = gen_lexerror(lexer, LEXER_ERR_UNTERMINATED_CHAR);
        return;
    }

    if (lexer->current_char == '\'') {
        lexer->err_status = gen_lexerror(lexer, LEXER_ERR_INVALID_CHAR_LITERAL_SPAN);
        return;
    }

    char c[2];
    if (lexer->current_char == '\\') {
        advance(lexer);

        if (!lexer->current_char) {
            lexer->err_status = gen_lexerror(lexer, LEXER_ERR_UNTERMINATED_CHAR);
            return;
        }

        if (!handle_escape_char(lexer, &c[0])) {
            return;
        }
    }
    else {
        c[0] = lexer->current_char;
    }

    advance(lexer);

    if (!lexer->current_char) {
        lexer->err_status = gen_lexerror(lexer, LEXER_ERR_UNTERMINATED_CHAR);
        return;
    }

    if (lexer->current_char != '\'') {
        lexer->err_status = gen_lexerror(lexer, LEXER_ERR_INVALID_CHAR_LITERAL_SPAN);
        return;
    }

    c[1] = '\0';

    push_lexer_token(lexer, (Token) {.kind = TOKEN_CHAR, .value = c});
    advance(lexer);
}



static void handle_number(Lexer* lexer) {
    size_t len = 0;
    size_t start = lexer->pos;


    bool is_float = false; 

    while (isdigit((unsigned char) lexer->current_char) || lexer->current_char == '.') {
        if (lexer->current_char == '.') {
            if (is_float) {
                lexer->err_status = gen_lexerror(lexer, LEXER_ERR_INVALID_FLOAT);
                return;
            }
            is_float = true;
        }
        len += 1;
        advance(lexer);
    }


    char* buffer = malloc(len + 1);
    if (buffer == NULL) {
        lexer->err_status = gen_lexerror(lexer, LEXER_ERR_INTERNAL_PUSH_ERROR);
        return;
    }

    memcpy(buffer, lexer->input+start, len);
    buffer[len] = '\0';


    if (is_float) {
       push_lexer_token(lexer, (Token) {.kind = TOKEN_FLOAT, .value = buffer});
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_INT, .value = buffer});
    }
    free(buffer);
}

static void handle_string(Lexer* lexer) {
    advance(lexer);
    
    size_t len = 0;
    char* buffer = malloc(lexer->input_len - lexer->pos + 1);
    if (buffer == NULL) {
        lexer->err_status = gen_lexerror(lexer, LEXER_ERR_INTERNAL_PUSH_ERROR);
        return;
    }

    while(lexer->current_char && lexer->current_char != '"') {
        if (lexer->current_char == '\\') {
            advance(lexer);

            if (!lexer->current_char) {
                free(buffer);
                lexer->err_status = gen_lexerror(lexer, LEXER_ERR_UNTERMINATED_STRING);
                return;
            }

            if (!handle_escape_char(lexer, &buffer[len])) {
                free(buffer);
                return;
            }
        }
        else {
            buffer[len] = lexer->current_char;
        }

        len+=1;
        advance(lexer);
    }

    if (!lexer->current_char) {
        free(buffer);
        lexer->err_status = gen_lexerror(lexer, LEXER_ERR_UNTERMINATED_STRING);
        return;
    }

    buffer[len] = '\0';

    push_lexer_token(lexer, (Token) {.kind = TOKEN_STRING, .value = buffer});
    free(buffer);
    advance(lexer);
}
static void handle_ident(Lexer* lexer) {
    size_t start = lexer->pos;
    
    while(isalnum((unsigned char) lexer->current_char) || lexer->current_char=='_') {
        advance(lexer);
    }
    size_t ident_len = lexer->pos - start;
    char* ident = malloc(ident_len + 1);
    if (ident == NULL) {
        lexer->err_status = gen_lexerror(lexer, LEXER_ERR_INTERNAL_PUSH_ERROR);
        return;
    }

    memcpy(ident, lexer->input+start, ident_len);

    ident[ident_len] = '\0';

    if (strcmp(ident, "if") == 0) {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_IF, .value = ident});
    }
    else if (strcmp(ident, "type") == 0) {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_TYPE, .value = ident});
    } else if (strcmp(ident, "as") == 0) {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_AS, .value = ident});
    } else if (strcmp(ident, "match") == 0) {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_MATCH, .value = ident});
    } else if (strcmp(ident, "else") == 0) {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_ELSE, .value = ident});
    } else if (strcmp(ident, "return") == 0) {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_RETURN, .value = ident});
    } else if (strcmp(ident, "while") == 0) {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_WHILE, .value = ident});
    } else if (strcmp(ident, "true") == 0) {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_TRUE, .value = ident});
    } else if (strcmp(ident, "false") == 0) {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_FALSE, .value = ident});
    } else if (strcmp(ident, "let") == 0) {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_LET, .value = ident});
    } else if (strcmp(ident, "fn") == 0) {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_FN, .value = ident});
    } else if (strcmp(ident, "struct") == 0) {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_STRUCT, .value = ident});
    } else if (strcmp(ident, "const") == 0) {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_CONST, .value = ident});
    } else if (strcmp(ident, "for") == 0) {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_FOR, .value = ident});
    } else if (strcmp(ident, "continue") == 0) {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_CONTINUE, .value = ident});
    } else if (strcmp(ident, "break") == 0) {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_BREAK, .value = ident});
    } else if (strcmp(ident, "enum") == 0) {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_ENUM, .value = ident});
    } else {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_IDENT, .value = ident});
    }
    free(ident);
}


static void handle_star(Lexer* lexer){
    if (peek(lexer) == '=') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = TOKEN_STAREQ, .value = "*="});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_STAR, .value = "*"});
        advance(lexer);
    }
}

static void handle_pipe(Lexer* lexer){
    if (peek(lexer) == '|') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = TOKEN_PIPE_PIPE, .value = "||"});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_PIPE, .value = "|"});
        advance(lexer);
    }
}

static void handle_amp(Lexer* lexer){
    if (peek(lexer) == '&') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = TOKEN_AMP_AMP, .value = "&&"});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_AMP, .value = "&"});
        advance(lexer);
    }
}


static void handle_bang(Lexer* lexer){
    if (peek(lexer) == '=') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = TOKEN_BANGEQ, .value = "!="});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_BANG, .value = "!"});
        advance(lexer);
    }
}

static void skip_to_new_line(Lexer* lexer) {
    while(lexer->current_char && lexer->current_char != '\n') {
        advance(lexer);
    }
}
static void skip_block_comment(Lexer* lexer) {
    while(1) {
        if (!lexer->current_char) {
            lexer->err_status = gen_lexerror(lexer, LEXER_ERR_UNTERMINATED_BLOCKCOMMENT);
            return;
        }
        if (lexer->current_char == '*' && peek(lexer) == '/') {
            advance(lexer);
            advance(lexer);
            break;
        }
        advance(lexer);
    }
}

static void handle_slash(Lexer* lexer){
    if (peek(lexer) == '/') {
        advance(lexer);
        skip_to_new_line(lexer);
    }
    else if (peek(lexer) == '*') {
        advance(lexer);
        advance(lexer);
        skip_block_comment(lexer);
    }
    else if (peek(lexer) == '=') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = TOKEN_SLASHEQ, .value = "/="});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_SLASH, .value = "/"});
        advance(lexer);
    }
}
static void handle_gt(Lexer* lexer){
    if (peek(lexer) == '=') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = TOKEN_GTEQ, .value = ">="});
        advance(lexer);
    }
    else if (peek(lexer) == '>') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = TOKEN_GTGT, .value = ">>"});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_GT, .value = ">"});
        advance(lexer);
    }
}
static void handle_lt(Lexer* lexer){
    if (peek(lexer) == '=') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = TOKEN_LTEQ, .value = "<="});
        advance(lexer);
    }
    else if (peek(lexer) == '<') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = TOKEN_LTLT, .value = "<<"});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_LT, .value = "<"});
        advance(lexer);
    }
}

static void handle_eq(Lexer* lexer){
    if (peek(lexer) == '=') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = TOKEN_EQEQ, .value = "=="});
        advance(lexer);
    }
    else if (peek(lexer) == '>') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = TOKEN_FATARROW, .value = "=>"});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_EQ, .value = "="});
        advance(lexer);
    }
}

static void handle_plus(Lexer* lexer){
    if (peek(lexer) == '=') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = TOKEN_PLUSEQ, .value = "+="});
        advance(lexer);
    }
    else if (peek(lexer) == '+') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = TOKEN_PLUSPLUS, .value = "++"});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_PLUS, .value = "+"});
        advance(lexer);
    }
}

static void handle_minus(Lexer* lexer){
    if (peek(lexer) == '=') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = TOKEN_MINUSEQ, .value = "-="});
        advance(lexer);
    }
    else if (peek(lexer) == '-') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = TOKEN_MINUSMINUS, .value = "--"});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_MINUS, .value = "-"});
        advance(lexer);
    }
}
static void handle_colon(Lexer* lexer){
    if (peek(lexer) == ':') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = TOKEN_COLONCOLON, .value = "::"});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = TOKEN_COLON, .value = ":"});
        advance(lexer);
    }
}

static void generate_next_token(Lexer* lexer) {
    skip_whitespaces(lexer);

    if (!lexer->current_char) {
        return;
    }

    lexer->token_start_pos = lexer->pos;
    lexer->token_start_line = lexer->line;
    lexer->token_start_column = lexer->column;

    switch (lexer->current_char)  {
        case ',':
            push_lexer_token(lexer, (Token) {.kind=TOKEN_COMMA, .value = ","});
            advance(lexer);
            break;
        case '%':
            push_lexer_token(lexer, (Token) {.kind=TOKEN_PERCENT, .value = "%"});
            advance(lexer);
            break;
        case '>':
            handle_gt(lexer);
            break;
        case '<':
            handle_lt(lexer);
            break;
        case '*':
            handle_star(lexer);
            break;
        case '/':
            handle_slash(lexer);
            break;
        case '-':
            handle_minus(lexer);
            break;
        case '+':
            handle_plus(lexer);
            break;
        case '=':
            handle_eq(lexer);
            break;
        case ':':
            handle_colon(lexer);
            break;
        case ';':
            push_lexer_token(lexer, (Token) {.kind=TOKEN_SEMICOLON, .value = ";"});
            advance(lexer);
            break;
        case '.':
            push_lexer_token(lexer, (Token) {.kind=TOKEN_DOT, .value = "."});
            advance(lexer);
            break;
        case '^':
            push_lexer_token(lexer, (Token) {.kind=TOKEN_CARET, .value = "^"});
            advance(lexer);
            break;
        case '!':
            handle_bang(lexer);
            break;
        case '}':
            push_lexer_token(lexer, (Token) {.kind=TOKEN_RBRACE, .value = "}"});
            advance(lexer);
            break;
        case '"':
            handle_string(lexer);
            break;
        case '\'':
            handle_char(lexer);
            break;
        case '{':
            push_lexer_token(lexer, (Token) {.kind=TOKEN_LBRACE, .value = "{"});
            advance(lexer);
            break;
        case ']':
            push_lexer_token(lexer, (Token) {.kind=TOKEN_RBRACKET, .value = "]"});
            advance(lexer);
            break;
        case '[':
            push_lexer_token(lexer, (Token) {.kind=TOKEN_LBRACKET, .value = "["});
            advance(lexer);
            break;
        case ')':
            push_lexer_token(lexer, (Token) {.kind=TOKEN_RPARENT, .value = ")"});
            advance(lexer);
            break;
        case '(':
            push_lexer_token(lexer, (Token) {.kind=TOKEN_LPARENT, .value = "("});
            advance(lexer);
            break;
        case '|':
            handle_pipe(lexer);
            break;
        case '&':
            handle_amp(lexer);
            break;
        default: 
            if (isdigit((unsigned char)lexer->current_char)) {
                handle_number(lexer);
            }
            else if (isalpha((unsigned char)lexer->current_char) || lexer->current_char == '_'){ 
                handle_ident(lexer);
            }
            else {
                lexer->err_status = gen_lexerror(lexer, LEXER_ERR_INVALID_CHARACTER);
            }
    }
}

LexResult generate_tokens(const char* str_input) {
    Lexer lexer = {
        .pos = 0,
        .line = 1,
        .column = 1,
        .token_start_pos = 0,
        .token_start_line = 1,
        .token_start_column = 1,
        .current_char = *str_input,
        .input = str_input,
        .tokens = create_token_Vec(),
        .input_len = strlen(str_input),
    };
    lexer.err_status = gen_lexerror(&lexer, LEXER_OK);

    while (lexer.current_char) {
        // alle funktionen kümmern sich selber darum ein Token weiter zu gehen, 
        // man muss hier also nicht nochmal manuel advance() callen.
        generate_next_token(&lexer); 

        if (lexer.err_status.status != LEXER_OK) {
            return (LexResult) {.tokens = lexer.tokens, .error = lexer.err_status};
        }
    }

    lexer.token_start_pos = lexer.pos;
    lexer.token_start_line = lexer.line;
    lexer.token_start_column = lexer.column;
    push_lexer_token(&lexer, (Token) {.kind=TOKEN_EOF, .value = "EOF"});

    return (LexResult) {.tokens = lexer.tokens, .error = lexer.err_status};
}
