#if !defined(___CMM_SYNTAX_TREE_H)
#define ___CMM_SYNTAX_TREE_H

#include "cmm_token_type.h"

typedef enum 
{
    BlockGeneral,  // text是空，info1是一个语句*链表*
    StatementIf,   // text是空，info1是条件，第一个info2是条件成立时的块，TODO: 第二个info2是条件不成立时的块
    StatementCall,   // text是空，info1是一个 ValueCall
    ValueInteger,  // text是整数
    ValueIdentifier,  // text是标识符
    ValueCall,     // text是函数名，info1是 Args*链表* 
    ValueArgs,     // text是空，next是下一个参数，info1是一个值结点
    ValueNegate,   // text是空，info1是目标值
    ValueAdd,      // text是空，info1是左值，info2是右值
    ValueSub,      // text是空，info1是左值，info2是右值
    ValueMul,      // text是空，info1是左值，info2是右值
    ValueDiv,      // text是空，info1是左值，info2是右值
    ValueLess,      // text是空，info1是左值，info2是右值
    ValueGreater,      // text是空，info1是左值，info2是右值
} cmm_syntax_node_type;

const char* cmm_syntax_node_type_alias[] = 
{
    [BlockGeneral] = "block",
    [StatementIf] = "if",
    [StatementCall] = "icall",
    [ValueInteger] = "int",
    [ValueIdentifier] = "id",
    [ValueCall] = "call",
    [ValueArgs] = "args",
    [ValueNegate] = "negate",
    [ValueAdd] = "add",
    [ValueSub] = "sub",
    [ValueMul] = "mul",
    [ValueDiv] = "div",
    [ValueLess] = "<",
    [ValueGreater] = ">",
}; 

#define GET_SYNTAX_NODE_ALIAS(type) (cmm_syntax_node_type_alias[type])

typedef enum 
{
    Value = 1,
    Executable = 2,
    ProgramBlock = 4, 
} cmm_syntax_node_tag;

typedef struct CmmSyntaxNode
{
    cmm_syntax_node_type type;
    cmm_syntax_node_tag tags;
    const char *value;
    struct CmmSyntaxNode *info1;
    struct CmmSyntaxNode *info2;
    struct CmmSyntaxNode *next; // 为链表预留
} cmm_syntax_node;

#endif