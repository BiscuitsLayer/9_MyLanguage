#ifndef Bintrans_h
#define Bintrans_h

#include "../my_general/General.h"
//#include "../my_asm/Assembler.h"
//#include "../my_asm/CPU.h"

#define GLOBAL -1

enum Hexadecimal { A = 10, B, C, D, E, F};
#define h(a, b) a * 16 + b

#define Insert(num, ...) { int bytes[] = {__VA_ARGS__};  \
		for (size_t i = 0; i < num; ++i) fprintf (writefile, "%c", bytes[i]);}
#define MultiInsert(num, byte) { for (size_t i = 0; i < num; ++i) \
		fprintf (writefile, "%c", byte);}

const int HEADERS_SIZE = h (8, 0);

void CreateELF ();

#endif