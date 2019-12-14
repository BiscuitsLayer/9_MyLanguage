#include "../my_stack/StackLib.h"
#include <cmath>
//#include <TXLin.h>

struct CPU_t {
    Stack_t stk = {};
    Stack_t func = {};
    int *RAM = nullptr;
    int ax = 0, bx = 0, cx = 0, dx = 0;
    int *data = nullptr;
    size_t it = 0;
};

void CPUMain ();
void LoadData (CPU_t *CPU, FILE *program_code, size_t program_len);
void DataHandle (CPU_t *CPU, size_t program_len);