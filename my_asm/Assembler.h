#include "../my_stack/Label.h"
#include "GeneralAsm.h"

void AssemblerMain ();
void ReadUserInput (FILE *user_input, char **input_start);
size_t GetProgramLen (FILE *user_input, struct stat file_info, char **input_start);
void UserInputHandle (int **code, FILE *user_lst);