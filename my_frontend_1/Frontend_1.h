#ifndef Frontend_1_h
#define Frontend_1_h

#include "../my_lang/General.h"

struct FE_1_VAR_t {
	bool appeared = false;
};

extern FE_1_VAR_t fe_vars[ARRAY_SIZE];
extern bool just_skipped_line;

namespace Lang {
    void TreeToLang (Node *node);
    void PrintShift (FILE *writefile);
    void NodeToLang (FILE *writefile, Node *node);
}

#endif