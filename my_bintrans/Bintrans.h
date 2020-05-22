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

struct LABEL_t {
	size_t code_ptr = 0; //Место в массиве программы, где находится метка
	char name [ARRAY_SIZE] = {}; //Имя метки
};

enum LABEL_REQUEST_TYPE { REQUEST_TYPE_CALL = 1, REQUEST_TYPE_JUMP = 1, REQUEST_TYPE_J_COND = 2, REQUEST_TYPE_MOV = 3 };

struct LABEL_REQUEST_t {
	size_t code_idx_request = 0; //Место в программе, где произошёл запрос к метке
	char name [ARRAY_SIZE] = {}; //Имя метки
	LABEL_REQUEST_TYPE type = REQUEST_TYPE_CALL;
};

enum Hexadecimal { A = 10, B, C, D, E, F};
#define h(a, b) a * 16 + b //TODO Надо бы убрать

#define Insert(num, ...) { unsigned char bytes[] = {__VA_ARGS__};  \
		for (size_t i = 0; i < num; ++i) code[code_idx++] = bytes[i]; }
#define MultiInsert(num, byte) { for (size_t i = 0; i < num; ++i) \
		code[code_idx++] = byte; }

extern ELF_VAR_t elf_vars [ARRAY_SIZE];

namespace ELF {
	int GetLabelAddress (char name[ARRAY_SIZE], LABEL_REQUEST_TYPE type);
	void SetLabelAddress (char name[ARRAY_SIZE]);
	void ELF_Header (FILE *writefile);
	void Program_Header (FILE *writefile);
	void CreateELF (Node *node);
	void GetVarIdx ();
	void TreeToELF (Node *node, FILE *writefile);
	void NodeToELF (FILE *writefile, Node *node);
	void ExtraFuncs (FILE *writefile);
	void AddGlobals (FILE *writefile);
	void HandleLabels ();
}

#endif