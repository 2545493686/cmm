#include "cmm_parser.h"

#define TOKEN_VALUE_LEN 32

#define throw_error_token(tokens, target_token_type)\
({\
    printf("expect token type %d, but got %d\n", target_token_type, sc_queue_peek_first(tokens->types));\
    for (int i = tokens->types->first; i < sc_queue_size(tokens->types); i++)\
    {\
        printf("[%d - %s] ", tokens->types->elems[i], tokens->texts->elems[i]);\
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

static const char *___last_pop_token_text;

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

static cmm_syntax_node *pop_literal(token_quene *tokens, cmm_token_type test_type, cmm_syntax_node_type node_type)
{
    cmm_syntax_node *node = (cmm_syntax_node *)malloc(sizeof(cmm_syntax_node));
    node->type = node_type;
    node->tags = Value;
    node->value = test_and_pop_token(tokens, test_type);
    return node;
}

static cmm_token_type peek_type(token_quene *tokens, int index)
{
    if (sc_queue_size(tokens->types) < index)
    {
        printf("error EOF\n");
        assert(0);
    }

    return (cmm_token_type)tokens->types->elems[tokens->types->first + index];
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
    cmm_syntax_node *node = new_node(ValueCall, Value);

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
    
    printf("unexpected token type: %d\n", peek_type(tokens, 0));
    assert(0);
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

static cmm_syntax_node *parse_indirect_expr(token_quene *tokens)
{
    return parse_expr_add_sub(tokens);
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
    cmm_syntax_node *node = (cmm_syntax_node *)malloc(sizeof(cmm_syntax_node));
    node->type = StatementIf;
    node->tags = Executable;

    test_and_rm_token(tokens, If);
    test_and_rm_token(tokens, LeftBracket);

    node->info1 = parse_value(tokens);

    test_and_rm_token(tokens, RightBracket);

    node->info1 = parse_block(tokens);

    return node;
}

// 分析一个语句
static cmm_syntax_node *parse_statement(token_quene *tokens)
{
    if (peek_type(tokens, 0) == EOF)
    {
        printf("none input\n");
        assert(0);
    }
    
    if (peek_type(tokens, 0) == If)
    {
        return parse_if(tokens);
    }

    printf("unexpected token type: %s", GET_SYNTAX_NODE_ALIAS(peek_type(tokens, 0)));
    assert(0);
}

// TODO: 风格切换
static void log_syntax_tree(cmm_syntax_node *node)
{
    if ((node->tags & ProgramBlock) != 0)
    {
        printf("{\n");
        cmm_syntax_node *stmt = node->info1;
        while (stmt != NULL)
        {
            log_syntax_tree(stmt);
            printf("\n");
            stmt = stmt->next;
        }
        printf("}\n");
    }

    printf("(%d %s ", node->type, GET_SYNTAX_NODE_ALIAS(node->type));
    
    if (node->value)
    {
        printf("%s ", node->value);
    }
    
    cmm_syntax_node *info1 = node->info1;
    while (info1 != NULL)
    {
        log_syntax_tree(info1);
        info1 = info1->next;
    }

    cmm_syntax_node *info2 = node->info2;
    while (info2 != NULL)
    {
        log_syntax_tree(info2);
        info2 = info2->next;
    }
    
    printf(")");
}

int main(int argc, char const *argv[])
{
    // 读入输入，获取 Tokens
    struct sc_queue_int token_types;
    struct sc_queue_str token_texts;

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
            printf("Error: invalid value");
            assert(0);
        }
        if (token_value[32] != '\0')
        {
            printf("Error: value too long");
            assert(0);
        }

        sc_queue_add_last(&token_types, token_type);
        sc_queue_add_last(&token_texts, token_value);

        token_value = (char *)malloc(TOKEN_VALUE_LEN);
        memset(token_value, 0, TOKEN_VALUE_LEN);
    }

    for (size_t i = 0; i < sc_queue_size(&token_types); i++) {
		printf("[%d - %s] ", token_types.elems[i], token_texts.elems[i]);
	}

    printf("count: %ld\n", sc_queue_size(&token_types));    

    sc_queue_add_last(&token_types, EOF);
    sc_queue_add_last(&token_texts, NULL);

    // 转为抽象语法树
    token_quene token_quene;
    token_quene.types = &token_types;
    token_quene.texts = &token_texts;

    cmm_syntax_node *node = parse_statement(&token_quene);
    log_syntax_tree(node);

    // while (peek_type(&token_quene, 0) != EOF)
    // {
    // }    
}
