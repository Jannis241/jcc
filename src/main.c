#include "../include/lexer.h"
#include "../include/parser.h"
#include "../include/ast.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static void print_out_token(Token* token) {
    printf("[TOKEN] kind=%s value=%s\n", token_kind_name(token->kind), token->value);
}


static char* read_file(const char* path) {
    FILE* file = fopen(path, "rb");
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

    char* buffer = malloc((size_t)size + 1);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }

    size_t read = fread(buffer, 1, (size_t)size, file);
    buffer[read] = '\0';

    fclose(file);
    return buffer;
}



int main(int argc, char* argv[]) {

    if (argc != 2) {
        printf("Usage: jcc <filename> \n");
        return 0;
    }

    char* source = read_file(argv[1]);
    if (source == NULL) return 1;

    LexResult token_res = generate_tokens(source);

    switch (token_res.error.status) {
        case LEXER_ERR_INTERNAL_PUSH_ERROR: 
            printf("ERR_INTERNAL_PUSH_ERROR \n");
            return 1;
        case LEXER_ERR_INVALID_CHARACTER: 
            printf("INVALID_CHARACTER: '%c' at pos: '%zu' \n", token_res.error.character, token_res.error.pos);
            return 1;
        case LEXER_ERR_UNEXPECTED_EOF: 
            printf("UNEXPECTED_EOF \n");
            return 1;
        case LEXER_ERR_UNTERMINATED_BLOCK: 
            printf("UNTERMINATED_BLOCK \n");
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

    ParserResult parser_result = parse_tokens(&token_res.tokens);

    if (parser_result.error.status != PARSER_OK) {
        switch (parser_result.error.status) {
            case PARSER_ERR_UNEXPECTED_TOP_LEVEL: 
                return 1;
            case PARSER_ERR_UNEXPECTED_EOF:
                return 1;
        }
    }


    free_token_vec(&token_res.tokens);
    free(source);
}
