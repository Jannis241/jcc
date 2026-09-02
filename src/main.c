#include "../include/parser.h"
#include "../include/semantic.h"

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

static void print_out_token(Token *token) {
    printf("[TOKEN] kind=%s value=%s\n", token_kind_name(token->kind),
           token->value);
}

static void print_spaces(size_t count) {
    for (size_t i = 0; i < count; i++) {
        putchar(' ');
    }
}

static size_t digit_count(size_t value) {
    size_t digits = 1;

    while (value >= 10) {
        value /= 10;
        digits += 1;
    }

    return digits;
}



static void print_source_line(const char *path, const char *source,
                              SourceSpan span) {
    if (span.line == 0 || span.column == 0) {
        return;
    }

    size_t line_start = span.start;
    while (line_start > 0 && source[line_start - 1] != '\n') {
        line_start -= 1;
    }

    size_t line_end = span.start;
    while (source[line_end] && source[line_end] != '\n') {
        line_end += 1;
    }

    size_t line_digits = digit_count(span.line);

    printf(" --> %s:%zu:%zu\n", path, span.line, span.column);
    print_spaces(line_digits);
    printf(" |\n");
    printf("%zu | ", span.line);
    fwrite(source + line_start, 1, line_end - line_start, stdout);
    putchar('\n');
    print_spaces(line_digits);
    printf(" | ");
    print_spaces(span.column - 1);
    printf("^\n");
}

static void print_lexer_error(const char *path, const char *source,
                              LexError error) {
    switch (error.status) {
    case LEXER_ERR_INTERNAL_PUSH_ERROR:
        printf("error: internal lexer error\n");
        break;
    case LEXER_ERR_INVALID_CHARACTER:
        printf("error: invalid character '%c'\n", error.character);
        break;
    case LEXER_ERR_UNEXPECTED_EOF:
        printf("error: unexpected end of file\n");
        break;
    case LEXER_ERR_INVALID_CHAR_LITERAL_SPAN:
        printf("error: invalid char literal\n");
        break;
    case LEXER_ERR_UNTERMINATED_STRING:
        printf("error: unterminated string literal\n");
        break;
    case LEXER_ERR_UNTERMINATED_CHAR:
        printf("error: unterminated char literal\n");
        break;
    case LEXER_ERR_INVALID_FLOAT:
        printf("error: invalid float literal\n");
        break;
    case LEXER_ERR_INVALID_NUMBER:
        printf("error: invalid number literal\n");
        break;
    case LEXER_ERR_UNTERMINATED_BLOCKCOMMENT:
        printf("error: unterminated block comment\n");
        break;
    case LEXER_OK:
        return;
    }

    print_source_line(path, source, error.span);
}

static void print_parser_error(const char *path, const char *source,
                               ParserError error) {
    switch (error.status) {
    case PARSER_ERR_UNEXPECTED_TOP_LEVEL:
        printf("error: unexpected top-level token %s\n",
               token_kind_name(error.got));
        break;
    case PARSER_ERR_UNEXPECTED_EOF:
        printf("error: unexpected end of file\n");
        break;
    case PARSER_ERR_UNEXPECTED_TOKEN:
        printf("error: expected %s, got %s\n", token_kind_name(error.expected),
               token_kind_name(error.got));
        break;
    case PARSER_ERR_UNEXPECTED_EXPR_START:
        printf("error: expected expression, got %s\n",
               token_kind_name(error.got));
        break;
    case PARSER_ERR_INVALID_CHAR_LITERAL:
        printf("error: invalid char literal\n");
        break;
    case PARSER_ERR_UNEXPECTED_STMT_START:
        printf("error: expected statement, got %s\n",
               token_kind_name(error.got));
        break;
    case PARSER_ERR_INVALID_ASSIGNMENT_TARGET:
        printf("error: invalid assignment target\n");
        break;
    case PARSER_OK:
        return;
    }

    print_source_line(path, source, error.span);
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

    if (token_res.error.status != LEXER_OK) {
        print_lexer_error(argv[1], source, token_res.error);
        return 1;
    }

    if (token_res.tokens.num_of_tokens == 0) {
        return 0;
    }

    ParserResult parser_result = parse_tokens(&token_res.tokens);

    if (parser_result.error.status != PARSER_OK) {
        print_parser_error(argv[1], source, parser_result.error);
        return 1;
    }
    print_ast(&parser_result.ast);
}
