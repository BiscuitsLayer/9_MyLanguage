#ifndef GeneralAsm_h
#define GeneralAsm_h

#include "../my_general/General.h"
#include <cstdio>
#include <cstdlib>
#include <cctype>
#include <cstring>
#include <cmath>
#include <sys/stat.h>

const size_t PUSH_LEN = 4;
const size_t POP_LEN = 3;
const size_t ACCURACY = 4;
const size_t RAM_SIZE = 1e6;
const size_t BUF_EXTRA_SIZE = 2;

#define STR_CMP(func) (strcmp(command, #func) == 0)
#define DEF_CMD(name, num, code) CMD_##name = num,

enum Commands {
#include "data/program.commands"
};

#undef DEF_CMD

#endif