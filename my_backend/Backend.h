#ifndef Backend_h
#define Backend_h

#include "../my_lang/General.h"

#include "../my_asm/Assembler.h"
#include "../my_asm/CPU.h"

struct ASM_VAR_t {
    size_t vars_array_idx = 0;
    size_t RAM_idx = 0;
};

struct ASM_FUNC_t {
    size_t funcs_array_idx = 0;
};

extern size_t RAM_base;
extern size_t RAM_it;

extern ASM_VAR_t asm_vars [ARRAY_SIZE];
extern ASM_FUNC_t asm_funcs [ARRAY_SIZE];

namespace ASM {
    void GetRAMIdx ();
    void TreeToASM (Node *node);
    void NodeToASM (FILE *writefile, Node *node);
}

#endif