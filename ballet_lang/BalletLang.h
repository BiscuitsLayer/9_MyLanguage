#ifndef BalletLang_h
#define BalletLang_h

#include "../my_general/General.h"

struct BL_VAR_t {
	bool appeared = false;
};

extern BL_VAR_t bl_vars[ARRAY_SIZE];
extern bool bl_just_skipped_line;

namespace BalletLang {
	void PrintShift (FILE *writefile);
	void NodeToLang (FILE *writefile, Node *node);
}

#endif