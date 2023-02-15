#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include "cmm_syntax_tree.h"

static void info_node_output(cmm_syntax_node *info_node)
{
    if (info_node != NULL)
    {
        printf("$");
        cmm_syntax_tree_output(info_node);
        printf("%%");
    }
    else
    {
        printf(".");
    }
}

void cmm_syntax_tree_output(cmm_syntax_node *node)
{
    if (node == NULL)
    {
        printf("null syntax tree!");
        return;       
    }

    printf("(");
    printf("%u ", node->type);
    printf("%u ", node->tags);
    
    if (node->value)
    {
        printf("$");
        printf("%s ", node->value);
    }
    else
    {
        printf(".");
    }
    
    info_node_output(node->info1);
    info_node_output(node->info2);

    printf(")");

    if (node->next)
    {
        printf("$");
        cmm_syntax_tree_output(node->next);
    }
}

static cmm_syntax_node * info_node_input()
{
    char c;
    scanf("%c", &c);
    if (c == '.')
    {
        return NULL;
    }
    
    assert(c == '$');
    cmm_syntax_node *begin = cmm_syntax_tree_input(); 
    cmm_syntax_node *end = begin;
    scanf("%c", &c);

    while (c == '$')
    {
        end->next = cmm_syntax_tree_input();
        end = end->next;
        scanf("%c", &c);
    }
    
    assert(c == '%');

    return begin;
}

cmm_syntax_node * cmm_syntax_tree_input()
{
    char c;
    
    cmm_syntax_node *node = (cmm_syntax_node *)malloc(sizeof(cmm_syntax_node));
    char *value = NULL;
    node->info1 = NULL;
    node->info2 = NULL;
    node->next = NULL;

    if (scanf("%c", &c) == -1)
    {
        return NULL;
    }

    if (c != '(')
    {
        printf("syntax error: %c.", c);
        return NULL;
    }
    
    scanf("%u %u ", &node->type, &node->tags);

    scanf("%c", &c);
    if (c == '$')
    {
        char *buf = (char *)malloc(32);
        scanf("%s ", buf);
        node->value = buf;
    }
    else
    {
        assert(c =='.');
    }

    node->info1 = info_node_input();
    node->info2 = info_node_input();

    scanf("%c", &c);
    assert(c == ')');
    return node;
}


static void syntax_tree_output_debug_style(cmm_syntax_node *node, int tab_count)
{
    if ((node->tags & ProgramBlock) != 0)
    {
        printf("{\n");
        cmm_syntax_node *stmt = node->info1;
        while (stmt != NULL)
        {
            for (size_t i = 0; i < tab_count + 1; i++)
                printf("    ");

            syntax_tree_output_debug_style(stmt, tab_count + 1);
            printf("\n");
            stmt = stmt->next;
        }
        for (size_t i = 0; i < tab_count; i++)
            printf("    ");

        printf("}");
        return;
    }

    printf("(%s ", GET_SYNTAX_NODE_ALIAS(node->type));
    
    if (node->value)
    {
        printf("%s ", node->value);
    }
    
    cmm_syntax_node *info1 = node->info1;
    if (!info1)
    {
        printf(". ");
    }
    while (info1 != NULL)
    {
        syntax_tree_output_debug_style(info1, tab_count);
        printf(" ");
        info1 = info1->next;
    }

    cmm_syntax_node *info2 = node->info2;
    if (!info2)
    {
        printf(". ");
    }
    while (info2 != NULL)
    {
        syntax_tree_output_debug_style(info2, tab_count);
        printf(" ");
        info2 = info2->next;
    }
    
    printf(")");
}

void cmm_syntax_tree_output_debug_style(cmm_syntax_node *node)
{
    syntax_tree_output_debug_style(node, 0);
    printf("\n");
}