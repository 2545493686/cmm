#include <stdio.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdarg.h>
#include "cmm_runtime.h"
#include "cmm_syntax_tree.h"

//#region throw
#define throw_on(test, node)\
    if ((test)) \
    {\
        __real_printf("current node:\n");\
        cmm_syntax_tree_output_debug_style((node)); \
        __real_printf(#test); \
        __real_printf(" is occur.");\
        assert(0);\
    }

#define throw(node, ...)\
    ({cmm_syntax_tree_output_debug_style((node)); \
    __real_printf(__VA_ARGS__); \
    assert(0);}) 

//#endregion

static int verbose_log = 1;

//#region wrap printf
#define printf __wrap_printf

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
//#endregion

static cmm_value Void = { .type = CmmBaseType_Void, .value_ptr = NULL };
static struct sc_map_sv *builtin_func; //name-value_handler pairs

static runtime_context * new_context()
{
    runtime_context *context = (runtime_context *)malloc(sizeof(runtime_context));
    sc_map_init_sv(&context->values, 0, 0);
    context->last = NULL;
    context->managed = NULL;
    return context;
}

static void free_context(runtime_context *context, cmm_value *ret)
{
    const char *key;
	void *value;
    sc_map_foreach (&context->values, key, value) 
    {
        if (value != ret)
        {
            free(((cmm_value *)value)->value_ptr);
            free(value);
        }
	}

    sc_map_term_sv(&context->values);

    managed_value *managed = context->managed;
    while (managed != NULL)
    {
        managed_value *item = managed;
        managed = managed->next;
        if (item->value != ret)
        {
            free(item->value->value_ptr);
            free(item->value);
        }
        free(item);
    }

    free(context);
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

static void add_managed_value(runtime_context *context, cmm_value *value)
{
    managed_value *managed = (managed_value *)malloc(sizeof(managed_value));
    managed->value = value;
    managed->next = context->managed;
    context->managed = managed->next;
}

static cmm_value * get_int(cmm_syntax_node *node, runtime_context *context)
{
    throw_on(node->type != ValueInteger, node);

    cmm_value *value = (cmm_value *)malloc(sizeof(cmm_value));
    value->type = CmmBaseType_Int;
    
    int32_t *i32 = (int32_t *)malloc(sizeof(int32_t));
    *i32 = atoi(node->value);
    value->value_ptr = (void *)i32;

    add_managed_value(context, value);

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
    throw_on(node->type != BlockGeneral, node);
    
    cmm_syntax_node *statement = node->info1;

    while (statement != NULL)
    {
        cmm_value *value = get_value(statement, context);

        if (value->type == CmmBaseType_Return)
        {
            cmm_value *ret = (cmm_value *)value->value_ptr;
            free(value);
            return ret;
        }

        statement = statement->next;
    }
    
    return &Void;
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

    if (func_value->type != CmmBaseType_Func)
    {
        throw(node, "syntax error: %s is not a function.", node->value);
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
        sc_map_put_sv(&sub_context->values, args_def->value, param_value);
        param_node = param_node->next;
        args_def = args_def->next;
    }

    cmm_value *ret = get_from_block(func_def->info2, sub_context);

    add_managed_value(context, ret);
    free_context(sub_context, ret);

    return ret;
}

static cmm_value * get_negate(cmm_syntax_node *node, runtime_context *context)
{
    throw_on(node->type != ValueNegate, node);

    cmm_value *value = get_value(node->info1, context);

    if (value->type != CmmBaseType_Int)
    {
        throw(node, "syntax error: negate only support int.");
    }

    int32_t *i32 = (int32_t *)value->value_ptr;
    *i32 = -*i32;

    return value;
}

static cmm_value * int_binary_operator(cmm_syntax_node *node, cmm_value *value1, cmm_value *value2)
{
    int32_t *i32_1 = (int32_t *)value1->value_ptr;
    int32_t *i32_2 = (int32_t *)value2->value_ptr;

    int32_t *i32 = (int32_t *)malloc(sizeof(int32_t));

    switch(node->type)
    {
        case ValueAdd:
            *i32 = *i32_1 + *i32_2;
            break;
        case ValueSub:
            *i32 = *i32_1 - *i32_2;
            break;
        case ValueMul:
            *i32 = *i32_1 * *i32_2;
            break;
        case ValueDiv:
            *i32 = *i32_1 / *i32_2;
            break;
        case ValueLess:
            *i32 = *i32_1 < *i32_2;
            break;
        case ValueGreater:
            *i32 = *i32_1 > *i32_2;
            break;
        default:
            throw(node, "syntax error: int binary operator only support +, -, *, /, <, >.");
    }

    cmm_value *ret = (cmm_value *)malloc(sizeof(cmm_value));
    ret->type = CmmBaseType_Int;
    ret->value_ptr = (void *)i32;

    return ret;
}   

static cmm_value * get_binary_operator(cmm_syntax_node *node, runtime_context *context)
{
    cmm_value *value1 = get_value(node->info1, context);
    cmm_value *value2 = get_value(node->info2, context);

    if (value1->type != CmmBaseType_Int || value2->type != CmmBaseType_Int)
    {
        throw(node, "syntax error: add only support int.");
    }

    cmm_value *ret = int_binary_operator(node, value1, value2);
    add_managed_value(context, ret);

    return ret;
}

static cmm_value * get_assign(cmm_syntax_node *node, runtime_context *context)
{
    throw_on(node->type != StatementAssign, node);

    if (node->info1->type != ValueIdentifier)
    {
        throw(node, "syntax error: assign only support identifier.");
    }

    cmm_value *value = get_value(node->info2, context);

    sc_map_put_sv(&context->values, node->info1->value, value);

    return value;
}



static cmm_value * get_func_def(cmm_syntax_node *node, runtime_context *context)
{
    throw_on(node->type != StatementFuncDef, node);

    cmm_func *func = (cmm_func *)malloc(sizeof(cmm_func));
    func->func_def = node;

    cmm_value *value = (cmm_value *)malloc(sizeof(cmm_value));
    value->type = CmmBaseType_Func;
    value->value_ptr = func;

    sc_map_put_sv(&context->values, node->value, value);

    return &Void;
}

static cmm_value * get_var_def(cmm_syntax_node *node, runtime_context *context)
{
    throw_on(node->type != StatementVarDef, node);

    sc_map_put_sv(&context->values, node->value, get_value(node->info2, context));

    return &Void;
}

static cmm_value * get_if(cmm_syntax_node *node, runtime_context *context)
{
    throw_on(node->type != StatementIf, node);

    cmm_value *value = get_value(node->info1, context);
    if (value->type != CmmBaseType_Int)
    {
        throw(node, "syntax error: if condition only support int.");
    }

    cmm_value *ret_from_block = &Void;
    int32_t *i32 = (int32_t *)value->value_ptr;
    if (*i32 != 0)
    {
        // FIXME: sub_context
        ret_from_block = get_from_block(node->info2, context);
    }
    else
    {
        // TODO: 条件不成立
        // run_executable(node->info2->next, context);
    }

    if (ret_from_block == &Void)
    {
        return ret_from_block;
    }
    else
    {
        cmm_value *ret = (cmm_value *)malloc(sizeof(cmm_value));
        ret->type = CmmBaseType_Return;
        ret->value_ptr = ret_from_block;
        return ret;
    }
}

static cmm_value * get_return(cmm_syntax_node *node, runtime_context *context)
{
    throw_on(node->type != StatementReturn, node);

    cmm_value *value = get_value(node->info1, context);

    cmm_value *ret = (cmm_value *)malloc(sizeof(cmm_value));  // 不用托管，在块得到时free
    ret->type = CmmBaseType_Return;
    ret->value_ptr = value;
    return ret;
}

static cmm_value * get_value(cmm_syntax_node *node, runtime_context *context)
{
    if (node == NULL)
    {
        return &Void;
    }

    value_handler handler = NULL;

    switch(node->type)
    {
        case ValueInteger:
            handler = get_int;
            break;
        case ValueIdentifier:
            handler = get_identifier;
            break;
        case ValueNegate:
            handler = get_negate;
            break;  
        case ValueAdd:
            handler = get_binary_operator;
            break;
        case ValueSub:
            handler = get_binary_operator;
            break;
        case ValueMul:
            handler = get_binary_operator;
            break;
        case ValueDiv:
            handler = get_binary_operator;
            break;
        case ValueLess:
            handler = get_binary_operator;
            break;
        case ValueGreater:
            handler = get_binary_operator;
            break;
        case ValueCall:
            handler = get_from_call;
            break;
        case StatementAssign:
            handler = get_assign;
            break;
        case StatementFuncDef:
            handler = get_func_def;
            break;
        case StatementVarDef:
            handler = get_var_def;
            break;
        case StatementIf:
            handler = get_if;
            break;
        case StatementReturn:
            handler = get_return;
            break;
        default:
            throw(node, "syntax error: %s is not implemented.", GET_SYNTAX_NODE_ALIAS(node->type));
    }

    return handler(node, context);
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
        cmm_value *value = get_value(node, context);
        if (value->type == CmmBaseType_Return)
        {
            break;
        }
        if (value->type !=CmmBaseType_Void)
        {
            free(value->value_ptr);
            free(value);
            free_syntax_node(node);
        }
    }

    return 0;
}
