#if !defined(___CMM_PARSER_H)
#define ___CMM_PARSER_H

#include <assert.h>
#include "cmm_syntax_tree.h"
#include "../lib/sc/queue/sc_queue.h"

typedef struct
{
    int type; 
    int line; // FIXME: 现在是假的
    int column; // FIXME: 现在是假的
    char *text;
} cmm_token;

typedef struct
{
    struct sc_queue_int *types;
    struct sc_queue_ptr *texts;
} token_quene;

static cmm_syntax_node *parse_value(token_quene *tokens);
static cmm_syntax_node *parse_statement(token_quene *tokens);
static cmm_token_type peek_type(token_quene *tokens, int index);

#endif