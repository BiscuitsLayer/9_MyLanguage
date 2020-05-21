#ifndef Bintrans_h
#define Bintrans_h

#include "Bintrans_commands.h"
#include "../my_backend_real/Backend_Real.h"
//#include "../my_general/General.h"
//#include "../my_asm/Assembler.h"
//#include "../my_asm/CPU.h"

#define GLOBAL -1

struct ELF_VAR_t {
	bool is_global = false;
	size_t shift = 0; // Своеобразный сдвиг - будет ли она сверху или снизу от rbp
};

enum Hexadecimal { A = 10, B, C, D, E, F};
#define h(a, b) a * 16 + b //TODO Надо бы убрать

#define Insert(num, ...) { unsigned char bytes[] = {__VA_ARGS__};  \
		for (size_t i = 0; i < num; ++i) code[code_idx++] = bytes[i]; }
#define MultiInsert(num, byte) { for (size_t i = 0; i < num; ++i) \
		code[code_idx++] = byte; }

extern ELF_VAR_t elf_vars [ARRAY_SIZE];

namespace ELF {
	void ELF_Header (FILE *writefile);
	void Program_Header (FILE *writefile);
	void CreateELF (Node *node);
	void GetVarIdx ();
	void TreeToELF (Node *node, FILE *writefile);
	void NodeToELF (FILE *writefile, Node *node);
	void ExtraFuncs (FILE *writefile);
	void AddGlobals (FILE *writefile);
}

#endif