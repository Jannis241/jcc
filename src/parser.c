#include"../include/lexer.h"
#include"../include/parser.h"
#include"../include/ast.h"

ParserResult parse_tokens(const TokenVec* tokens) {
    ASTConstVec const_vec;
    ASTConstVec_init(&const_vec);

    ASTFunctionVec func_vec;
    ASTFunctionVec_init(&func_vec);

    ASTStructDefVec structdef_vec;
    ASTStructDefVec_init(&structdef_vec);

    ASTEnumDefVec enumdef_vec;
    ASTEnumDefVec_init(&enumdef_vec);
    
    AST new_ast = {.constants = const_vec, .functions = func_vec, .struct_defs = structdef_vec,.enum_defs = enumdef_vec};

    Parser parser = {.tokens = tokens, .ast = new_ast, .parser_error = {0},.current_token = &tokens->data[0], .pos = 0};

    return (ParserResult) {.ast = parser.ast, .error = parser.parser_error};
}
