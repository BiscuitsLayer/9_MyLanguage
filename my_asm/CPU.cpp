#include "CPU.h"

void CPUMain () {
    CPU_t CPU = {}; //Структура процессора
    CPU.stk = StackInit (); //Инициализация процессора
    CPU.func = StackInit (); //Инициализация стека вызовов функций
    CPU.RAM = (int *) calloc (RAM_SIZE, sizeof (int));

    FILE *program_code = fopen ("../my_ast/program.code", "rb"); //Машинный код

    fseek (program_code, 0, SEEK_END);
    size_t program_len = ftell (program_code); //Ввод длины машинного кода
    fseek (program_code, 0, SEEK_SET);
    program_len /= sizeof (int);

    LoadData (&CPU, program_code, program_len); //Загрузка машинного кода в процессор

    DataHandle (&CPU, program_len); //Исполнение машинного кода
    fclose (program_code);
}

void LoadData (CPU_t *CPU, FILE *program_code, size_t program_len) {
    CPU->data = (int *) calloc (program_len, sizeof(int)); //Выделение памяти под машинный код
    fread (CPU->data, sizeof(int), program_len, program_code); //Чтение машинного кода
}

void DataHandle (CPU_t *CPU, size_t program_len) {
    for (CPU->it = 0; CPU->it < program_len; ++CPU->it) {
        switch (CPU->data[CPU->it]) { //Обработка машинного кода
            #include "data/program.lang"
            #define DEF_CMD(name, num, code) case CMD_##name COLON { code; break; }
            #include "data/program.commands"

#undef DEF_CMD
        }
    }
}