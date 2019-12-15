#ifndef Backend_h
#define Backend_h

#include "../my_lang/General.h"

#include "../my_asm/Assembler.h"
#include "../my_asm/CPU.h"

namespace ASM {
    void TreeToASM (Node *node);
    void NodeToASM (FILE *writefile, Node *node);
}

#endif