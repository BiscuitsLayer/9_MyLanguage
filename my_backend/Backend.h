#ifndef Backend_h
#define Backend_h

#include "../my_lang/General.h"

#include "../my_asm/Assembler.h"
#include "../my_asm/CPU.h"

#define GLOBAL -1

struct ASM_VAR_t {
    size_t RAM_idx = 0;
    bool is_global = false;
};

extern size_t NUM_IF;
extern size_t global_vars;
extern size_t main_vars;

extern ASM_VAR_t asm_vars [ARRAY_SIZE];

namespace ASM {
    void GetRAMIdx ();
    void TreeToASM (Node *node);
    void NodeToASM (FILE *writefile, Node *node);
}

#endif