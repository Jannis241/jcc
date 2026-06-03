#include "../include/lexer.h"
#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char* token_kind_name(TokenKind kind) {
    switch (kind) {
        case True: return "True";
        case False: return "False";
        case Ident: return "Ident";
        case IntNumber: return "IntNumber";
        case FloatNumber: return "FloatNumber";
        case StringLiteral: return "StringLiteral";
        case Char: return "Char";
        case If: return "If";
        case Else: return "Else";
        case Return: return "Return";
        case While: return "While";
        case Let: return "Let";
        case Fn: return "Fn";
        case In: return "In";
        case Struct: return "Struct";
        case Const: return "Const";
        case For: return "For";
        case Continue: return "Continue";
        case Break: return "Break";
        case Enum: return "Enum";
        case Underscore: return "Underscore";
        case AmpAmp: return "AmpAmp";
        case Amp: return "Amp";
        case Pipe: return "Pipe";
        case PipePipe: return "PipePipe";
        case LParen: return "LParen";
        case RParen: return "RParen";
        case LBrace: return "LBrace";
        case RBrace: return "RBrace";
        case Bang: return "Bang";
        case LBracket: return "LBracket";
        case RBracket: return "RBracket";
        case Comma: return "Comma";
        case Semicolon: return "Semicolon";
        case Colon: return "Colon";
        case Dot: return "Dot";
        case Eq: return "Eq";
        case PlusEq: return "PlusEq";
        case MinusEq: return "MinusEq";
        case StarEq: return "StarEq";
        case SlashEq: return "SlashEq";
        case FatArrow: return "FatArrow";
        case PlusPlus: return "PlusPlus";
        case MinusMinus: return "MinusMinus";
        case EqEq: return "EqEq";
        case BangEq: return "BangEq";
        case Lt: return "Lt";
        case LtEq: return "LtEq";
        case Gt: return "Gt";
        case GtEq: return "GtEq";
        case Plus: return "Plus";
        case Minus: return "Minus";
        case Star: return "Star";
        case Slash: return "Slash";
        case Percent: return "Percent";
        case Eof: return "Eof";
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
        .character = lexer->current_char,
        .status = status,
    };
}

int free_token_vec(TokenVec* vec) {
    if (vec == NULL) {
        return 1;
    }

    if (vec->data != NULL) {
        for (size_t i = 0; i < vec->num_of_tokens; i++) {
            free((void*)vec->data[i].value);
        }
        free(vec->data);
        vec->data = NULL;
    }

    vec->num_of_tokens = 0;
    vec->token_capacity = 0;
    return 0;
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
    if (push_token(&lexer->tokens, &token) != 0) {
        lexer->err_status = gen_lexerror(lexer, LEXER_ERR_INTERNAL_PUSH_ERROR);
    }
}

static char peek(Lexer* lexer) {
    if (lexer->pos + 1 >= strlen(lexer->input)) {
        return '\0';
    }
    return lexer->input[lexer->pos + 1];
}

static void advance(Lexer* lexer) {
    lexer->pos += 1;
    lexer->current_char = lexer->input[lexer->pos];
}


static void skip_whitespaces(Lexer* lexer) {
    while (lexer->current_char && isspace((unsigned char)lexer->current_char)) {
        advance(lexer);
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

    if (peek(lexer) == '\0') {
        lexer->err_status = gen_lexerror(lexer, LEXER_ERR_UNTERMINATED_CHAR);
        return;
    }

    if (peek(lexer) != '\'') {
        lexer->err_status = gen_lexerror(lexer, LEXER_ERR_INVALID_CHAR_LITERAL_SPAN);
        return;
    }

    char c[2];
    c[0] = lexer->current_char;
    c[1] = '\0';

    push_lexer_token(lexer, (Token) {.kind = Char, .value = c});
    advance(lexer);
    advance(lexer);
}



static void handle_number(Lexer* lexer) {
    size_t len = 0;
    size_t start = lexer->pos;


    bool is_float = false; 

    while (isdigit((unsigned char) lexer->current_char) || lexer->current_char == '.') {
        if (lexer->current_char == '.') {
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
       push_lexer_token(lexer, (Token) {.kind = FloatNumber, .value = buffer});
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = IntNumber, .value = buffer});
    }
    free(buffer);
}

static void handle_string(Lexer* lexer) {
    advance(lexer);
    
    size_t len = 0;
    size_t start = lexer->pos;

    while(lexer->current_char && lexer->current_char != '"') {
        len+=1;
        advance(lexer);
    }

    if (!lexer->current_char) {
        lexer->err_status = gen_lexerror(lexer, LEXER_ERR_UNTERMINATED_STRING);
        return;
    }

    char* buffer = malloc(len + 1);
    if (buffer == NULL) {
        lexer->err_status = gen_lexerror(lexer, LEXER_ERR_INTERNAL_PUSH_ERROR);
        return;
    }

    memcpy(buffer, lexer->input+start, len);
    buffer[len] = '\0';

    push_lexer_token(lexer, (Token) {.kind = StringLiteral, .value = buffer});
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
        push_lexer_token(lexer, (Token) {.kind = If, .value = ident});
    } else if (strcmp(ident, "else") == 0) {
        push_lexer_token(lexer, (Token) {.kind = Else, .value = ident});
    } else if (strcmp(ident, "return") == 0) {
        push_lexer_token(lexer, (Token) {.kind = Return, .value = ident});
    } else if (strcmp(ident, "while") == 0) {
        push_lexer_token(lexer, (Token) {.kind = While, .value = ident});
    } else if (strcmp(ident, "true") == 0) {
        push_lexer_token(lexer, (Token) {.kind = True, .value = ident});
    } else if (strcmp(ident, "false") == 0) {
        push_lexer_token(lexer, (Token) {.kind = False, .value = ident});
    } else if (strcmp(ident, "let") == 0) {
        push_lexer_token(lexer, (Token) {.kind = Let, .value = ident});
    } else if (strcmp(ident, "fn") == 0) {
        push_lexer_token(lexer, (Token) {.kind = Fn, .value = ident});
    } else if (strcmp(ident, "in") == 0) {
        push_lexer_token(lexer, (Token) {.kind = In, .value = ident});
    } else if (strcmp(ident, "struct") == 0) {
        push_lexer_token(lexer, (Token) {.kind = Struct, .value = ident});
    } else if (strcmp(ident, "const") == 0) {
        push_lexer_token(lexer, (Token) {.kind = Const, .value = ident});
    } else if (strcmp(ident, "for") == 0) {
        push_lexer_token(lexer, (Token) {.kind = For, .value = ident});
    } else if (strcmp(ident, "continue") == 0) {
        push_lexer_token(lexer, (Token) {.kind = Continue, .value = ident});
    } else if (strcmp(ident, "break") == 0) {
        push_lexer_token(lexer, (Token) {.kind = Break, .value = ident});
    } else if (strcmp(ident, "enum") == 0) {
        push_lexer_token(lexer, (Token) {.kind = Enum, .value = ident});
    } else {
        push_lexer_token(lexer, (Token) {.kind = Ident, .value = ident});
    }
    free(ident);
}


static void handle_star(Lexer* lexer){
    if (peek(lexer) == '=') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = StarEq, .value = "*="});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = Star, .value = "*"});
        advance(lexer);
    }
}

static void handle_pipe(Lexer* lexer){
    if (peek(lexer) == '|') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = PipePipe, .value = "||"});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = Pipe, .value = "|"});
        advance(lexer);
    }
}

static void handle_amp(Lexer* lexer){
    if (peek(lexer) == '&') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = AmpAmp, .value = "&&"});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = Amp, .value = "&"});
        advance(lexer);
    }
}


static void handle_bang(Lexer* lexer){
    if (peek(lexer) == '=') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = BangEq, .value = "!="});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = Bang, .value = "!"});
        advance(lexer);
    }
}

static void skip_to_new_line(Lexer* lexer) {
    while(lexer->current_char && lexer->current_char != '\n') {
        advance(lexer);
    }
}

static void handle_slash(Lexer* lexer){
    if (peek(lexer) == '/') {
        advance(lexer);
        skip_to_new_line(lexer);
    }
    else if (peek(lexer) == '=') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = SlashEq, .value = "/="});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = Slash, .value = "/"});
        advance(lexer);
    }
}
static void handle_gt(Lexer* lexer){
    if (peek(lexer) == '=') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = GtEq, .value = ">="});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = Gt, .value = ">"});
        advance(lexer);
    }
}
static void handle_lt(Lexer* lexer){
    if (peek(lexer) == '=') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = LtEq, .value = "<="});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = Lt, .value = "<"});
        advance(lexer);
    }
}

static void handle_eq(Lexer* lexer){
    if (peek(lexer) == '=') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = EqEq, .value = "=="});
        advance(lexer);
    }
    else if (peek(lexer) == '>') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = FatArrow, .value = "=>"});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = Eq, .value = "="});
        advance(lexer);
    }
}

static void handle_plus(Lexer* lexer){
    if (peek(lexer) == '=') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = PlusEq, .value = "+="});
        advance(lexer);
    }
    else if (peek(lexer) == '+') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = PlusPlus, .value = "++"});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = Plus, .value = "+"});
        advance(lexer);
    }
}

static void handle_minus(Lexer* lexer){
    if (peek(lexer) == '=') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = MinusEq, .value = "-="});
        advance(lexer);
    }
    else if (peek(lexer) == '-') {
        advance(lexer);
        push_lexer_token(lexer, (Token) {.kind = MinusMinus, .value = "--"});
        advance(lexer);
    }
    else {
        push_lexer_token(lexer, (Token) {.kind = Minus, .value = "-"});
        advance(lexer);
    }
}

static void generate_next_token(Lexer* lexer) {
    skip_whitespaces(lexer);

    if (!lexer->current_char) {
        return;
    }

    switch (lexer->current_char)  {
        case ',':
            push_lexer_token(lexer, (Token) {.kind=Comma, .value = ","});
            advance(lexer);
            break;
        case '%':
            push_lexer_token(lexer, (Token) {.kind=Percent, .value = "%"});
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
            push_lexer_token(lexer, (Token) {.kind=Colon, .value = ":"});
            advance(lexer);
            break;
        case ';':
            push_lexer_token(lexer, (Token) {.kind=Semicolon, .value = ";"});
            advance(lexer);
            break;
        case '.':
            push_lexer_token(lexer, (Token) {.kind=Dot, .value = "."});
            advance(lexer);
            break;
        case '!':
            handle_bang(lexer);
            break;
        case '}':
            push_lexer_token(lexer, (Token) {.kind=RBrace, .value = "}"});
            advance(lexer);
            break;
        case '"':
            handle_string(lexer);
            break;
        case '\'':
            handle_char(lexer);
            break;
        case '{':
            push_lexer_token(lexer, (Token) {.kind=LBrace, .value = "{"});
            advance(lexer);
            break;
        case ']':
            push_lexer_token(lexer, (Token) {.kind=RBracket, .value = "]"});
            advance(lexer);
            break;
        case '[':
            push_lexer_token(lexer, (Token) {.kind=LBracket, .value = "["});
            advance(lexer);
            break;
        case ')':
            push_lexer_token(lexer, (Token) {.kind=RParen, .value = ")"});
            advance(lexer);
            break;
        case '(':
            push_lexer_token(lexer, (Token) {.kind=LParen, .value = "("});
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
    Lexer lexer = {.pos = 0, .current_char = *str_input, .input = str_input, .tokens = create_token_Vec()};
    lexer.err_status = gen_lexerror(&lexer, LEXER_OK);


    while (lexer.current_char) {
        generate_next_token(&lexer);

        if (lexer.err_status.status != LEXER_OK) {
            return (LexResult) {.tokens = lexer.tokens, .error = lexer.err_status};
        }
    }

    push_lexer_token(&lexer, (Token) {.kind=Eof, .value = "EOF"});

    return (LexResult) {.tokens = lexer.tokens, .error = lexer.err_status};
}
