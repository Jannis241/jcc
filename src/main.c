#include "../include/ast.h"
#include "../include/lexer.h"
#include "../include/parser.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static void print_out_token(Token *token) {
    printf("[TOKEN] kind=%s value=%s\n", token_kind_name(token->kind),
           token->value);
}

static char *read_file(const char *path) {
    FILE *file = fopen(path, "rb");
    if (file == NULL) {
        perror("fopen");
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    if (size < 0) {
        fclose(file);
        return NULL;
    }

    char *buffer = malloc((size_t)size + 1);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    size_t read = fread(buffer, 1, (size_t)size, file);
    buffer[read] = '\0';

    fclose(file);
    return buffer;
}

int main(int argc, char *argv[]) {

    if (argc != 2) {
        printf("Usage: jcc <filename> \n");
        return 0;
    }

    char *source = read_file(argv[1]);
    if (source == NULL)
        return 1;

    LexResult token_res = generate_tokens(source);


    switch (token_res.error.status) {
    case LEXER_OK:
        printf("[LEXER SUCCESS]\n");
        break;
    case LEXER_ERR_INTERNAL_PUSH_ERROR:
        printf("ERR_INTERNAL_PUSH_ERROR \n");
        return 1;
    case LEXER_ERR_INVALID_CHARACTER:
        printf("INVALID_CHARACTER: '%c' at pos: '%zu' \n",
               token_res.error.character, token_res.error.pos);
        return 1;
    case LEXER_ERR_INVALID_FLOAT:
        printf("LEXER_ERR_INVALID_FLOAT \n");
        return 1;
    case LEXER_ERR_UNEXPECTED_EOF:
        printf("UNEXPECTED_EOF \n");
        return 1;
    case LEXER_ERR_UNTERMINATED_CHAR:
        printf("UNTERMINATED_CHAR \n");
        return 1;
    case LEXER_ERR_INVALID_CHAR_LITERAL_SPAN: {
        printf("INVALID_CHAR_LITERAL_SPAN");
        return 1;
    }
    case LEXER_ERR_UNTERMINATED_STRING:
        printf("UNTERMINATED_STRING \n");
        return 1;
    }

    if (token_res.tokens.num_of_tokens == 0) {
        return 0;
    }

    for (size_t i = 0; i < token_res.tokens.num_of_tokens; i++) {
        print_out_token(&token_res.tokens.data[i]);
    }

    ParserResult parser_result = parse_tokens(&token_res.tokens);

    switch (parser_result.error.status) {
    case PARSER_OK:
        printf("[PARSER SUCCESS]\n");
        break;
    case PARSER_ERR_UNEXPECTED_EXPR_START:
        printf("PARSER_ERR_UNEXPECTED_EXPR_START\n");
        break;
    case PARSER_ERR_INVALID_CHAR_LITERAL:
        printf("PARSER_ERR_INVALID_CHAR_LITERAL \n");
        break;
    case PARSER_ERR_UNEXPECTED_TOKEN:
        printf("PARSER_ERR_UNEXPECTED_TOKEN: expected: %s, got: %s, pos: %zu \n", token_kind_name(parser_result.error.expected),token_kind_name(parser_result.error.got), parser_result.error.token_pos);
        return 1;
    case PARSER_ERR_UNEXPECTED_TOP_LEVEL:
        printf("PARSER_ERR_UNEXPECTED_TOP_LEVEL\n");
        return 1;
    case PARSER_ERR_UNEXPECTED_EOF:
        printf("PARSER_ERR_UNEXPECTED_EOF\n");
        return 1;
    }

    for (size_t i = 0; i < parser_result.ast.constants.len; i++) {
        printf("%s: %s = '%s' \n", parser_result.ast.constants.data[i]->name, parser_result.ast.constants.data[i]->type, parser_result.ast.constants.data[i]->value);
    }

}
