#include <stdarg.h>
#include <stdio.h>
#include "cmm_parser.h"

#ifndef EOF
#define EOF (-1)
#endif

#define printf __wrap_printf
#define TOKEN_VALUE_LEN 32

#define throw_error(...)\
    __real_printf(__VA_ARGS__);\
    assert(0);

#define throw_error_token(tokens, target_token_type)\
({\
    if (target_token_type == -1)\
    {\
        printf("unexpect token %d %s\n", sc_queue_peek_first(tokens->types), \
            (char *)sc_queue_peek_first(tokens->texts));\
    }\
    else\
    {\
        printf("expect token type %d, but got %d %s\n", target_token_type,  \
            sc_queue_peek_first(tokens->types), (char *)sc_queue_peek_first(tokens->texts));\
    }\
    for (int i = tokens->types->first; i < sc_queue_size(tokens->types); i++)\
    {\
        printf("[%d - %s] ", tokens->types->elems[i], (char *)tokens->texts->elems[i]);\
    }\
    printf("\n");\
    assert(0);\
})

#define test_and_rm_token(tokens, target_token_type) \
({\
    if (peek_type(tokens, 0) != target_token_type)\
    {\
        throw_error_token(tokens, target_token_type);\
    }\
    (void)sc_queue_del_first(tokens->types);\
    (void)sc_queue_del_first(tokens->texts);\
})

static char *___last_pop_token_text;

#define test_and_pop_token(tokens, target_token_type) \
(({\
    if (peek_type(tokens, 0) != target_token_type)\
    {\
        throw_error_token(tokens, target_token_type);\
        assert(0);\
    }\
    ___last_pop_token_text = sc_queue_peek_first(tokens->texts);\
    (void)sc_queue_del_first(tokens->types);\
    (void)sc_queue_del_first(tokens->texts);\
}), ___last_pop_token_text)

int verbose_log = 0;

int __real_printf(const char *__fmt, ...)
{
    // 调用vprintf
    va_list args;
    va_start(args, __fmt);
    int ret = vprintf(__fmt, args);
    va_end(args);
    return ret;
}

int __wrap_printf(const char *__fmt, ...)
{
    if (verbose_log)
    {
        // 调用vprintf
        va_list args;
        va_start(args, __fmt);
        int ret = vprintf(__fmt, args);
        va_end(args);
        return ret;
    }
    return 0;
}

static cmm_syntax_node *new_node(cmm_syntax_node_type type, cmm_syntax_node_tag tags)
{
    cmm_syntax_node *node = (cmm_syntax_node *)malloc(sizeof(cmm_syntax_node));
    node->type = type;
    node->tags = tags;
    node->info1 = NULL;
    node->info2 = NULL;
    node->next = NULL;
    return node;
}

static cmm_syntax_node *pop_literal(token_quene *tokens, cmm_token_type test_type, cmm_syntax_node_type node_type)
{
    cmm_syntax_node *node = new_node(node_type, Value);
    node->value = test_and_pop_token(tokens, test_type);
    return node;
}

static cmm_token_type peek_type(token_quene *tokens, int index)
{
    if (sc_queue_size(tokens->types) < index)
    {
        throw_error("error EOF\n");
    }

    return (cmm_token_type)tokens->types->elems[tokens->types->first + index];
}

static cmm_syntax_node *parse_args(token_quene *tokens)
{
    cmm_syntax_node *node = new_node(ValueArgs, Value);
    node->info1 = parse_value(tokens);
    if (peek_type(tokens, 0) == Comma)
    {
        test_and_rm_token(tokens, Comma);
        node->next = parse_args(tokens);
    }
    return node;
}

static cmm_syntax_node *parse_call(token_quene *tokens)
{
    cmm_syntax_node *node = new_node(ValueCall, Value | Executable);

    node->value = test_and_pop_token(tokens, Identifier); // 函数名
    test_and_rm_token(tokens, LeftBracket); // (
    
    if (peek_type(tokens, 0) == RightBracket)
    {
        test_and_rm_token(tokens, RightBracket); // )
        return node;
    }

    node->info1 = parse_args(tokens); // 参数列表
    test_and_rm_token(tokens, RightBracket); // )
    return node;
}

static cmm_syntax_node *parse_direct_expr(token_quene *tokens)
{
    if (peek_type(tokens, 0) == Integer)
    {
        return pop_literal(tokens, Integer, ValueInteger);
    }
    
    if (peek_type(tokens, 0) == LeftBracket) // ( value )
    {
        test_and_rm_token(tokens, LeftBracket);
        cmm_syntax_node *node = parse_value(tokens);
        test_and_rm_token(tokens, RightBracket);
        return node;
    }

    if (peek_type(tokens, 0) == Minus) // - value
    {
        test_and_rm_token(tokens, Minus);
        cmm_syntax_node *node = new_node(ValueNegate, Value);
        node->info1 = parse_value(tokens);
        return node;
    }

    if (peek_type(tokens, 0) == Identifier) 
    {
        // 函数调用
        if (peek_type(tokens, 1) == LeftBracket)
        {
            return parse_call(tokens);
        }

        // 变量
        return pop_literal(tokens, Identifier, ValueIdentifier);
    }
    
    throw_error("unexpected token type: %d\n", peek_type(tokens, 0));
}

static cmm_syntax_node *parse_expr_mul_div(token_quene *tokens)
{
    cmm_syntax_node *info1 = parse_direct_expr(tokens);

    if (peek_type(tokens, 0) == Star)
    {
        test_and_rm_token(tokens, Star);
        cmm_syntax_node *node = new_node(ValueMul, Value);
        node->info1 = info1;
        node->info2 = parse_expr_mul_div(tokens);
        return node;
    }

    if (peek_type(tokens, 0) == Slash)
    {
        test_and_rm_token(tokens, Slash);
        cmm_syntax_node *node = new_node(ValueDiv, Value);
        node->info1 = info1;
        node->info2 = parse_expr_mul_div(tokens);
        return node;
    }
    
    return info1;
}

static cmm_syntax_node *parse_expr_add_sub(token_quene *tokens)
{
    cmm_syntax_node *info1 = parse_expr_mul_div(tokens);

    if (peek_type(tokens, 0) == Plus)
    {
        test_and_rm_token(tokens, Plus);
        cmm_syntax_node *node = new_node(ValueAdd, Value);
        node->info1 = info1;
        node->info2 = parse_expr_add_sub(tokens);
        return node;
    }

    if (peek_type(tokens, 0) == Minus)
    {
        test_and_rm_token(tokens, Minus);
        cmm_syntax_node *node = new_node(ValueSub, Value);
        node->info1 = info1;
        node->info2 = parse_expr_add_sub(tokens);
        return node;
    }
    
    return info1;
}

static cmm_syntax_node *parse_expr_compare(token_quene *tokens)
{
    cmm_syntax_node *info1 = parse_expr_add_sub(tokens);

    if (peek_type(tokens, 0) == LessThan)
    {
        test_and_rm_token(tokens, LessThan);
        cmm_syntax_node *node = new_node(ValueLess, Value);
        node->info1 = info1;
        node->info2 = parse_expr_add_sub(tokens);
        return node;
    }

    if (peek_type(tokens, 0) == GreaterThan)
    {
        test_and_rm_token(tokens, GreaterThan);
        cmm_syntax_node *node = new_node(ValueGreater, Value);
        node->info1 = info1;
        node->info2 = parse_expr_add_sub(tokens);
        return node;
    }

    // TODO: <= >= ==
    
    return info1;
}

static cmm_syntax_node *parse_assign(token_quene *tokens)
{
    cmm_syntax_node *info1 = parse_expr_compare(tokens);
    
    if (peek_type(tokens, 0) == Equal)
    {
        cmm_syntax_node *node = new_node(StatementAssign, Executable | Value);
        node->info1 = info1;
        test_and_rm_token(tokens, Equal);
        node->info2 = parse_assign(tokens);
        return node;
    }
    
    return info1;
}

static cmm_syntax_node *parse_indirect_expr(token_quene *tokens)
{
    return parse_assign(tokens);
}

static cmm_syntax_node *parse_value(token_quene *tokens)
{
    return parse_indirect_expr(tokens);
}

static cmm_syntax_node *parse_block(token_quene *tokens)
{
    cmm_syntax_node *node = new_node(BlockGeneral, ProgramBlock);
    test_and_rm_token(tokens, LeftBrace);

    cmm_syntax_node *last = NULL;
    while (peek_type(tokens, 0) != RightBrace)
    {
        cmm_syntax_node *stmt = parse_statement(tokens);
        if (last == NULL)
        {
            node->info1 = stmt;
        }
        else
        {
            last->next = stmt;
        }
        last = stmt;
    }

    test_and_rm_token(tokens, RightBrace);
    return node;
}

static cmm_syntax_node *parse_if(token_quene *tokens)
{
    cmm_syntax_node *node = new_node(StatementIf, Executable);

    test_and_rm_token(tokens, If);
    test_and_rm_token(tokens, LeftBracket);

    node->info1 = parse_value(tokens);

    test_and_rm_token(tokens, RightBracket);

    node->info2 = parse_block(tokens);

    return node;
}

static cmm_syntax_node *parse_return(token_quene *tokens)
{
    cmm_syntax_node *node = new_node(StatementReturn, Executable);
    test_and_rm_token(tokens, Return);
    node->info1 = parse_value(tokens);
    test_and_rm_token(tokens, EOL);
    return node;
}

static cmm_syntax_node *parse_value_statement(token_quene *tokens)
{
    cmm_syntax_node *node = parse_value(tokens);

    if ((node->tags & Executable) == 0)
    {
        cmm_syntax_tree_output_debug_style(node);
        printf("\n");
        throw_error("error: expr is not executable\n")
    }

    test_and_rm_token(tokens, EOL);
    return node;
}

static cmm_syntax_node *parse_variable_def(token_quene *tokens)
{
    cmm_syntax_node *node = new_node(StatementVarDef, Executable);
    node->info1 = pop_literal(tokens, Identifier, ValueIdentifier);
    node->value = test_and_pop_token(tokens, Identifier);

    if (peek_type(tokens, 0) == Equal)
    {
        test_and_rm_token(tokens, Equal);
        node->info2 = parse_value(tokens);
    }

    test_and_rm_token(tokens, EOL);
    return node;
}

static cmm_syntax_node *parse_args_def(token_quene *tokens)
{
    cmm_syntax_node *node = new_node(ValueArgs, Value);
    node->info1 = pop_literal(tokens, Identifier, ValueIdentifier);
    node->value = test_and_pop_token(tokens, Identifier);

    if (peek_type(tokens, 0) == Comma)
    {
        test_and_rm_token(tokens, Comma);
        node->next = parse_args(tokens);
    }

    return node;
}

static cmm_syntax_node *parse_func_def(token_quene *tokens)
{
    cmm_syntax_node *node = new_node(StatementFuncDef, Executable);
    node->info1 = pop_literal(tokens, Identifier, ValueIdentifier); // 返回值
    node->value = test_and_pop_token(tokens, Identifier); // 函数名
    test_and_rm_token(tokens, LeftBracket);
    node->info1->next = parse_args_def(tokens);
    test_and_rm_token(tokens, RightBracket);
    node->info2 = parse_block(tokens);
    return node;
}

// 分析一个语句
static cmm_syntax_node *parse_statement(token_quene *tokens)
{
    if (peek_type(tokens, 0) == EOF)
    {
        throw_error("none input\n");
    }
    
    if (peek_type(tokens, 0) == If)
    {
        return parse_if(tokens);
    }

    if (peek_type(tokens, 0) == Return)
    {
        return parse_return(tokens);
    }

    if (peek_type(tokens, 0) == Identifier)
    {
        if (peek_type(tokens, 1) == Identifier)
        {
            if (peek_type(tokens, 2) == EOL) // 变量声明
            {
                return parse_variable_def(tokens);
            }

            if (peek_type(tokens, 2) == Equal) // 变量赋值声明
            {
                return parse_variable_def(tokens);
            }

            if (peek_type(tokens, 2) == LeftBracket) //( 函数声明
            {
                return parse_func_def(tokens);
            }
        }
    }

    return parse_value_statement(tokens);
}

int main(int argc, char const *argv[])
{
    enum {
        NormalStyle = 0,
        DebugStyle,
    } style = NormalStyle;   // 0: normal, 1: debug

    for (size_t i = 0; i < argc; i++)
    {
        if (strcmp(argv[i], "-s=debug") == 0)
        {
            style = DebugStyle;
        }
        else if (strcmp(argv[i], "-v") == 0)
        {
            verbose_log = 1;
        }
    }

    // 读入输入，获取 Tokens
    struct sc_queue_int token_types;
    struct sc_queue_ptr token_texts;

	sc_queue_init(&token_types);
    sc_queue_init(&token_texts); 

    int token_type, read_count;
    char *token_value = (char *)malloc(TOKEN_VALUE_LEN);
    memset(token_value, 0, TOKEN_VALUE_LEN);

    while ((read_count = scanf("%d %s ", &token_type, token_value)) != -1)
    {
        // printf("read %d: %d %s\n", read_count, token_type, token_value);
        
        if (read_count <= 1)
        {
            throw_error("Error: invalid value");
        }
        if (token_value[32] != '\0')
        {
            throw_error("Error: value too long");
        }

        sc_queue_add_last(&token_types, token_type);
        sc_queue_add_last(&token_texts, token_value);

        token_value = (char *)malloc(TOKEN_VALUE_LEN);
        memset(token_value, 0, TOKEN_VALUE_LEN);
    }

    for (size_t i = 0; i < sc_queue_size(&token_types); i++) {
		printf("[%d - %s] ", token_types.elems[i], (char *)token_texts.elems[i]);
	}

    printf("count: %ld\n", sc_queue_size(&token_types));    

    sc_queue_add_last(&token_types, EOF);
    sc_queue_add_last(&token_texts, NULL);

    // 转为抽象语法树
    token_quene token_quene;
    token_quene.types = &token_types;
    token_quene.texts = &token_texts;

    while (peek_type(&token_quene, 0) != EOF)
    {
        cmm_syntax_node *node = parse_statement(&token_quene);
        switch (style)
        {
            case NormalStyle:
                cmm_syntax_tree_output(node);
                break;
            case DebugStyle:
                cmm_syntax_tree_output_debug_style(node);
                break;
            default:
                throw_error("Error: unknown style %d", style);
                break;
        }
    }    
}
