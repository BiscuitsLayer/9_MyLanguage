#include "Backend.h"

void ASM::TreeToASM (Node *node) {
    FILE *writefile = fopen ("../my_ast/temp.asm", "rb");
    ASM::NodeToASM (writefile, node);
    fclose (writefile);
}

void ASM::NodeToASM (FILE *writefile, Node *node) {
    if (node->left)
        ASM::NodeToASM (writefile, node->left);
    if (node->right)
        ASM::NodeToASM (writefile, node->right);
}