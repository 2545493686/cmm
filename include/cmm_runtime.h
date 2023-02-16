#ifndef __CMM_RUNTIME_H__

#define __CMM_RUNTIME_H__ 1

#include "cmm_syntax_tree.h"
#include "../lib/sc/map/sc_map.h"

typedef enum {
    CmmBaseType_Void = 0,
    CmmBaseType_Return,  // value_ptr:value_ptr
    CmmBaseType_Int,    // value_ptr:int32_t
    CmmBaseType_Func,   // value_ptr:cmm_func
    CmmBaseType_Bool, //TODO:
    CmmBaseType_Char, //TODO:
    CmmBaseType_String, //TODO:
    CmmBaseType_Array, //TODO:
    CmmBaseType_Struct, //TODO:
    CmmBaseType_Union, //TODO:
    CmmBaseType_Enum, //TODO:
    CmmBaseType_Pointer, //TODO:
} cmm_base_type;

typedef struct CmmValue
{
    cmm_base_type type;
    void *value_ptr;
} cmm_value;

typedef struct ManagedValue
{
    cmm_value *value;
    struct ManagedValue *next;
} managed_value;


typedef struct RuntimeContext
{
    struct sc_map_sv values; // id-cmm_value pairs, value中不能有新增的指针
    struct RuntimeContext *last;
    managed_value *managed; //托管的内存，在块内新开辟的内存应存放在values或managed
} runtime_context;

typedef struct
{
    cmm_syntax_node *func_def;   
} cmm_func;

typedef void (*executable_handler)(cmm_syntax_node *node, runtime_context *context); //废弃
typedef cmm_value *(*value_handler)(cmm_syntax_node *node, runtime_context *context);

static cmm_value * get_value(cmm_syntax_node *node, runtime_context *context);

#endif