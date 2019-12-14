#include "../my_stack/Label.h"
#include <cmath>
#include <cstdio>
#include <cstring>
#include <stdlib.h>

void DisassemblerMain ();
void LoadData (int **code, FILE *program_code, size_t program_len);
void DataHandle (int *code, FILE *disasm_decode, size_t program_len, Label_t *labels);