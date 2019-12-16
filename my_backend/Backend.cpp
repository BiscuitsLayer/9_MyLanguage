#include "Backend.h"

void ASM::TreeToASM (Node *node) {
    FILE *writefile = fopen ("../my_ast/temp.asm", "rb");
    ASM::NodeToASM (writefile, node);
    fclose (writefile);
}

void ASM::NodeToASM (FILE *writefile, Node *node) {
    if (node->type == TYPE_SYS && node->data == IF) {
        ASM::NodeToASM (writefile, node->left);
        fprintf (writefile, "label1\n");
        ASM::NodeToASM (writefile, node->right);
    }
    if (node->type == TYPE_OP && node->data == OP_EQUAL) {
        ASM::NodeToASM (writefile, node->left);
        ASM::NodeToASM (writefile, node->right);
        fprintf (writefile, "JE ");
    }
    else {
        if (node->left)
            ASM::NodeToASM (writefile, node->left);
        if (node->right)
            ASM::NodeToASM (writefile, node->right);
    }
}