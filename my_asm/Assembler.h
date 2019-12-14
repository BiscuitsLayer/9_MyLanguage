#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include "../my_stack/LabelLib.h"

const int STRLEN = 5;
const int BUF_EXTRA_SIZE = 2;

void AssemblerMain ();
void ReadUserInput (FILE *user_input, char ***commands_list, size_t *program_len, size_t *special, Label_t **labels, size_t *labels_idx, char **input_start);
size_t GetCommandsNum (FILE *user_input, struct stat file_info, char **input_start);
void UserInputHandle (int **code, char **commands_list, FILE *user_lst, size_t program_len, size_t special, Label_t *labels, size_t labels_idx);