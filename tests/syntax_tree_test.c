#include <stdio.h>
#include "cmm_syntax_tree.h"

int main(int argc, char const *argv[])
{
    cmm_syntax_node *node = NULL;
    while ((node = cmm_syntax_tree_input()) != NULL)
    {
        cmm_syntax_tree_output_debug_style(node);
    }
        
    return 0;
}
