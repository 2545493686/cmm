#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include "cmm_runtime.h"
#include "cmm_syntax_tree.h"

//#region throw
#define throw_on(test, node)\
    if (test) \
    {\
        cmm_syntax_tree_output_debug_style(node); \    
        __real_printf(#test); \
        __real_printf(" is occur.");\
        assert(0);\
    }

#define throw(node, ...)\
    cmm_syntax_tree_output_debug_style(node); \
    __real_printf(__VA_ARGS__); \
    assert(0); 
//#endregion

#define printf __wrap_printf

static const cmm_value Void = { .type = CmmBaseType_Void, .value_ptr = NULL };
static struct sc_map_sv *builtin_func; //name-value_handler pairs
static int verbose_log = 1;

static int __real_printf(const char *__fmt, ...)
{
    // 调用vprintf
    va_list args;
    va_start(args, __fmt);
    int ret = vprintf(__fmt, args);
    va_end(args);
    return ret;
}

static int __wrap_printf(const char *__fmt, ...)
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

static runtime_context * new_context()
{
    runtime_context *context = (runtime_context *)malloc(sizeof(runtime_context));
    sc_map_init_sv(&context->values, 0, 0);
    return context;
}

static cmm_value * get_context_value(runtime_context *context, char *key)
{
    cmm_value *value = (cmm_value *)sc_map_get_sv(&context->values, key);
    if (sc_map_found(&context->values))
    {
        return value;
    }

    if (context->last != NULL)
    {
        return get_context_value(context->last, key);
    }

    return NULL;
}

static cmm_value * get_int(cmm_syntax_node *node, runtime_context *context)
{
    throw_on(node->type != ValueInteger, node);

    cmm_value *value = (cmm_value *)malloc(sizeof(cmm_value));
    value->type = CmmBaseType_Int;
    
    int32_t *i32 = (int32_t *)malloc(sizeof(int32_t));
    *i32 = atoi(node->value);
    value->value_ptr = (void *)i32;

    return value;
}

static cmm_value * get_identifier(cmm_syntax_node *node, runtime_context *context)
{
    cmm_value *value = get_context_value(context, node->value);
	
    if (value != NULL) 
        return value;
	else
        throw(node, "syntax error: %s is not defined.", node->value);
}

static cmm_value * get_from_block(cmm_syntax_node *node, runtime_context *context)
{
    // TODO: 执行直到遇到 return
    
}

static cmm_value * get_from_builtin_call(cmm_syntax_node *node, runtime_context *context)
{
    value_handler handler = (value_handler)sc_map_get_sv(builtin_func, node->value);
    if (!sc_map_found(builtin_func))
    {
        throw(node, "syntax error: func %s is not defined.", node->value);
    }

    return handler(node, context);
}

static cmm_value * get_from_call(cmm_syntax_node *node, runtime_context *context)
{
    throw_on(node->type != ValueCall, node);

    // 获取函数
    cmm_value *func_value = get_context_value(context, node->value);

    if (func_value == NULL)
    {
        return get_from_builtin_call(node, context);
    }

    cmm_func *func = (cmm_func *)func_value->value_ptr;
    cmm_syntax_node *func_def = func->func_def;
    cmm_syntax_node *args_def = func_def->info1->next;

    runtime_context *sub_context = new_context();
    sub_context->last = context;

    // 获取参数并加入 sub_context
    cmm_syntax_node *param_node = node->info1;
    while (param_node != NULL)
    {
        cmm_value *param_value = get_value(param_node->info1, context);
        sc_map_put_sv(&sub_context->values, args_def->info1->value, param_value);
        param_node = param_node->next;
        args_def = args_def->next;
    }

    cmm_value *ret = get_from_block(func_def->info2, sub_context);

    char *key;
	void *value;
    sc_map_foreach (&sub_context->values, key, value) {
		free(value);
	}

    sc_map_term_sv(&sub_context->values);

    free(sub_context);

    return ret;
}

static value_handler value_handlers[] = {
    [ValueInteger] = get_int,
    [ValueIdentifier] = get_identifier,
};

static cmm_value * get_value(cmm_syntax_node *node, runtime_context *context)
{
    if (node == NULL)
    {
        return &Void;
    }

    throw_on((node->tags % Value) == 0, node);    

    value_handler handler = value_handlers[node->type];
    if (handler == NULL)
    {
        throw(node, "syntax error: %s is not implemented.", GET_SYNTAX_NODE_ALIAS(node->type));
    }

    return handler(node, context);
}

static void run_func_def(cmm_syntax_node *node, runtime_context *context)
{
    throw_on(node->type != StatementFuncDef, node);

    cmm_func *func = (cmm_func *)malloc(sizeof(cmm_func));
    func->func_def = node;

    cmm_value *value = (cmm_value *)malloc(sizeof(cmm_value));
    value->type = CmmBaseType_Func;
    value->value_ptr = func;

    sc_map_put_sv(&context->values, node->value, value);
}

static void run_var_def(cmm_syntax_node *node, runtime_context *context)
{
    throw_on(node->type != StatementVarDef, node);

    // TODO: 设置初值
    sc_map_put_sv(&context->values, node->value, get_value(node->info2, context));
}

static executable_handler executable_handlers[] = {
    [StatementFuncDef] = run_func_def,
    [StatementVarDef] = run_var_def,
};

static int run_executable(cmm_syntax_node *node, runtime_context *context)
{
    if ((node->tags & Executable) == 0)
    {
        throw(node, "syntax error: %s is not executable.", GET_SYNTAX_NODE_ALIAS(node->type));
    }

    executable_handler handler = executable_handlers[node->type];
    if (handler == NULL)
    {
        throw(node, "syntax error: %s is not implemented.", GET_SYNTAX_NODE_ALIAS(node->type));
    }
    
    handler(node, context);
}

cmm_value * builtin_print(cmm_syntax_node *node, runtime_context *context)
{
    throw_on(node->type != ValueCall, node);

    cmm_syntax_node *arg = node->info1;

    if (arg->next != NULL)
    {
        throw(node, "syntax error: %s can only print one value.", GET_SYNTAX_NODE_ALIAS(node->type));
    }

    cmm_value *value = get_value(arg->info1, context);

    if (value->type == CmmBaseType_Int)
    {
        printf("%d\n", *(int32_t *)value->value_ptr);
    }
    else if (value->type == CmmBaseType_String)
    {
        printf("%s\n", (char *)value->value_ptr);
    }
    else
    {
        throw(node, "syntax error: %s can not print.", GET_SYNTAX_NODE_ALIAS(node->type));
    }

    return &Void;
}

static void init_builtin_func()
{
    builtin_func = (struct sc_map_sv *)malloc(sizeof(struct sc_map_sv));
    sc_map_init_sv(builtin_func, 0, 0);

    sc_map_put_sv(builtin_func, "print", (void *)builtin_print);
}

int main(int argc, char const *argv[])
{
    init_builtin_func();

    runtime_context *context = new_context();

    cmm_syntax_node *node = NULL;
    while ((node = cmm_syntax_tree_input()) != NULL)
    {
        parse_statement(node, context);
    }

    return 0;
}
