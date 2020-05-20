#ifndef Backend_Real_h
#define Backend_Real_h

#include "../my_general/General.h"
#include "../my_asm/Assembler.h"
#include "../my_asm/CPU.h"

#define GLOBAL -1

struct ASM_VAR_t {
	bool is_global = false;
	size_t shift = 0; // Своеобразный сдвиг - будет ли она сверху или снизу от rbp
};

extern size_t NUM_IF;
extern size_t NUM_POW;
extern size_t locals_num;
const int REAL_ACCURACY = 1e2; // Для арифметики с фиксированной точностью, >= 10
const int SQRT_REAL_ACCURACY = sqrt (REAL_ACCURACY);
const int EXP_REAL_ACCURACY = 2; //log (REAL_ACCURACY);

extern ASM_VAR_t asm_vars [ARRAY_SIZE];

namespace ASM {
	void GetVarIdx ();
	void TreeToASM (Node *node);
	void NodeToASM (FILE *writefile, Node *node);
	void ExtraFuncs (FILE *writefile);
	void AddGlobals (FILE *writefile);
}

#endif