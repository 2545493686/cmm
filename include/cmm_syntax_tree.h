#if !defined(___CMM_SYNTAX_TREE_H)
#define ___CMM_SYNTAX_TREE_H

#include "cmm_token_type.h"

typedef enum 
{
    BlockGeneral,  // text是空，info1是一个语句*链表*
    StatementIf,   // text是空，info1是条件，第一个info2是条件成立时的块，TODO: 第二个info2是条件不成立时的块
    StatementReturn,   // text是空，info1是一个 Value
    StatementAssign,   // text是空，info1是一个 Value，info2是一个 Value
    StatementVarDef,    // text是变量名，info1是一个Id标志类型，info2是初值（Value）
    StatementFuncDef,    // text是函数名，第一个info1是一个Id标志返回值类型，剩下的info1是ArgsDef**链表**，info2是程序块
    ArgsDef,   // text是参数名，第一个info1是类型
    ValueInteger,  // text是整数
    ValueIdentifier,  // text是标识符
    ValueCall,     // text是函数名，info1是 ValueArgs*链表* 
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
    [StatementReturn] = "return",
    [StatementAssign] = "=",
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
    [StatementVarDef] = "var",
    [StatementFuncDef] = "func",
    [ArgsDef] = "args",
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

void cmm_syntax_tree_output(cmm_syntax_node *node);
cmm_syntax_node * cmm_syntax_tree_input();
void cmm_syntax_tree_output_debug_style(cmm_syntax_node *node);

#endif