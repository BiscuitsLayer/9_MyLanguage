#include "Assembler.h"

char **commands_list = nullptr; //Массив с указателями на команды
size_t program_len = 0; //Количество строк в программе
size_t special = 0; //Количество специальных символов в программе

Label_t *labels = nullptr; //Массив меток
size_t labels_idx = 0; //Индекс в массиве меток

void AssemblerMain () {
    labels = (Label_t *) calloc (LABEL_ARRAY_SIZE, sizeof(Label_t)); //Массив меток

    FILE *user_input = fopen ("../my_ast/temp.asm", "rb"); //Ввод пользователя
    char *input_start = nullptr; //Начало ввода пользователя
    ReadUserInput (user_input, &input_start);
    fclose (user_input);

    int *code = nullptr; //Массив машинного кода процессора
    FILE *user_lst = fopen ("../my_ast/temp.lst", "w"); //Листинг компиляции
    UserInputHandle (&code, user_lst);
    fclose (user_lst);

    FILE *program_code = fopen ("../my_ast/program.code", "w"); //Машинный код процессора
    fwrite (code, sizeof (int), program_len + special - labels_idx, program_code);

    free (code);
    free (labels);
    free (input_start);
    fclose (program_code);
}

void ReadUserInput (FILE *user_input, char **input_start) {
    struct stat file_info = {}; //Информация о файле с вводом пользователя
    stat ("../my_ast/temp.asm", &file_info);
    program_len = GetProgramLen (user_input, file_info, input_start);

    commands_list = (char **) calloc (program_len, sizeof (char *));
    idx = 0; //Индекс в массиве с указателями на команды
    commands_list[idx++] = *input_start;

    //Расстановка указателей в массив commands_list
    //ptr - указатель в буфере
    while (isspace (**input_start)) ++input_start;
    for (char *ptr = *input_start; ( (ptr < *input_start + file_info.st_size) && (*ptr != EOF) && (idx != program_len) ); ++ptr) {
        while ( (*ptr != '\0') && (*ptr != EOF) ) ++ptr;
        ++ptr;
        while ( (isspace (*ptr)) && (*ptr != EOF) ) ++ptr;
        if (*ptr != EOF) commands_list[idx++] = ptr;
    }

    char command[STR_LEN];

    //Поиск количества особых команд, содержащих аргументы
    for (size_t i = 0; i < program_len; ++i) {
        //strcpy (command, commands_list[i]);
        sscanf (commands_list[i], "%s", command);

        //Проверка на команду PUSH
        if (strcmp (command, "PUSH") == 0) {
            if (commands_list[i][PUSH_LEN + 2] == 'X' || commands_list[i][PUSH_LEN + 2] == 'P') {
                commands_list[i][PUSH_LEN] = '_';
            }
            else if (commands_list[i][PUSH_LEN + 1] == '[' && commands_list[i][PUSH_LEN + 3] == 'X') {
                char reg_name = commands_list[i][PUSH_LEN + 2];
                sprintf (commands_list[i], "PUSHRM_%cX", reg_name);
            }
            else if (commands_list[i][PUSH_LEN + 1] == '[' && commands_list[i][PUSH_LEN + 3] == 'P') {
                char reg_name = commands_list[i][PUSH_LEN + 2];
                sprintf (commands_list[i], "PUSHRM_%cP", reg_name);
            }
            else if (commands_list[i][PUSH_LEN + 1] == '[') {
                int temp = 0;
                sscanf (commands_list[i], "PUSH [%d]", &temp);
                sprintf (commands_list[i], "PUSHRM %d", temp);
                ++special;
            }
            else if (commands_list[i][PUSH_LEN + 1] == '(') {
                int temp = 0;
                sscanf (commands_list[i], "PUSH (%d)", &temp);
                sprintf (commands_list[i], "PUSHDR %d", temp);
                ++special;
            }
            else {
                ++special;
            }
        }

        //Проверка на команду POP
        if (strcmp (command, "POP") == 0) {
            if (commands_list[i][POP_LEN + 2] == 'X' || commands_list[i][POP_LEN + 2] == 'P') {
                commands_list[i][POP_LEN] = '_';
            }
            else if (commands_list[i][POP_LEN + 1] == '[' && commands_list[i][POP_LEN + 3] == 'X') {
                char reg_name = commands_list[i][POP_LEN + 2];
                sprintf (commands_list[i], "POPRM_%cX", reg_name);
            }
            else if (commands_list[i][POP_LEN + 1] == '[' && commands_list[i][POP_LEN + 3] == 'P') {
                char reg_name = commands_list[i][POP_LEN + 2];
                sprintf (commands_list[i], "POPRM_%cP", reg_name);
            }
            else if (commands_list[i][POP_LEN + 1] == '[') {
                int temp = 0;
                sscanf (commands_list[i], "POP [%d]", &temp);
                sprintf (commands_list[i], "POPRM %d", temp);
                ++special;
            }
            else if (commands_list[i][POP_LEN + 1] == '(') {
                int temp = 0;
                sscanf (commands_list[i], "POP (%d)", &temp);
                sprintf (commands_list[i], "POPDR %d", temp);
                ++special;
            }
        }

        //Проверка на команду JMP или CALL
        else if ( (strcmp (command, "JMP") == 0) || (strcmp (command, "JA") == 0) ||
        (strcmp (command, "JAE") == 0) || (strcmp (command, "JB") == 0) || (strcmp (command, "JBE") == 0) ||
        (strcmp (command, "JE") == 0) || (strcmp (command, "JNE") == 0) || (strcmp (command, "CALL") == 0) ) {
            ++special;
        }

        //Проверка на наличие комментария
        else if (command[strlen (command) - 1] == ':') {
            labels[labels_idx].name = commands_list[i];
            for (int j = 0; j < labels_idx; ++j) {
            	if (strcmp (labels[j].name, labels[labels_idx].name) == 0) {
					printf ("ERROR! Same labels names\n");
					exit (2);
	            }
            }
            labels[labels_idx].name[strlen (labels[labels_idx].name) - 1] = '\0';
            labels[(labels_idx)++].num = -1;
        }
    }
}

size_t GetProgramLen (FILE *user_input, struct stat file_info, char **input_start) {
    char *buf = (char *) calloc (file_info.st_size + BUF_EXTRA_SIZE, sizeof (char)); //Буфер из файла ввода пользователя
    fread (buf, sizeof (char), file_info.st_size, user_input); //Загрузка ввода пользователя в буфер

    size_t program_len = 0; //Колиество строк в программе
    size_t i = 0; //Индекс в буфере

    while (isspace (buf[i])) ++i;
    *input_start = buf;
    for ( ; ( (i < file_info.st_size) && (buf[i] != EOF) ); ++i) {
        if (buf[i] == '\n')
            ++program_len, buf[i] = '\0';
        while ((isspace (buf[i]) || buf[i] == '\0') && (i < file_info.st_size - 1))
            ++i;
        if (buf[i] == EOF) {
            ++program_len;
            break;
        }
        if ((i == file_info.st_size - 1) && (buf[i] != '\n')) {
            ++program_len;
        }
    }
    return program_len;
}

void UserInputHandle (int **code, FILE *user_lst) {
    *code = (int *) calloc (program_len + special - labels_idx, sizeof(int)); //Выделение памяти для массива с машинным кодом
    idx = 0; //Индекс в массиве с машинным кодом
    size_t shift = 0; //Сдвиг в листинге компиляции
    char *command = (char *) calloc (STR_LEN, sizeof(char)); //Введенная команда
    char comment_tag = ';'; //Введенный символ комментария
    long long read_words = 0; //Количество слов в строке

    //Обработка команд из массива с указателями на команды
    for (int pass = 1; pass <= 2; ++pass) {
        idx = 0;
        shift = 0;
        for (size_t i = 0; i < program_len; ++i) {
            sscanf (commands_list[i], "%s", command);
            //Обработка команд, содержащих аргументы
            if (strcmp (command, "PUSH") == 0) {
                double operand_double = 0; //Введенный операнд (если имеется)
                int operand_int = 0; //Введенный операнд (если имеется) с заданной точоностью
                read_words = sscanf (commands_list[i], "%*s %lg %c", &operand_double, &comment_tag);
                if ((read_words != 1) && (comment_tag != ';')) { //Проверка на наличие в строке комментариев
                    printf("ERROR in line '%s': Wrong syntax\n", commands_list[i]);
                    exit(2);
                }
                (*code)[idx++] = CMD_PUSH;
                operand_int = (int) (operand_double * pow (10, ACCURACY));
                (*code)[idx++] = operand_int;

                // (Вывод в листинг компиляции если 2 проход)
                if (pass == 2)
                    fprintf (user_lst, "%08zu %08x %08x PUSH %lg\n", shift, CMD_PUSH, operand_int, operand_double);
                shift += 4 * sizeof(char) + sizeof(int); //Увеличение сдвига в листинге компиляции
                continue;
            }
            else if (strcmp (command, "PUSHRM") == 0) {
                double operand_double = 0; //Введенный операнд (если имеется)
                int operand_int = 0; //Введенный операнд (если имеется) с заданной точоностью
                read_words = sscanf (commands_list[i], "%*s %lg %c", &operand_double, &comment_tag);
                if ((read_words != 1) && (comment_tag != ';')) { //Проверка на наличие в строке комментариев
                    printf ("ERROR in line '%s': Wrong syntax\n", commands_list[i]);
                    exit (2);
                }
                (*code)[idx++] = CMD_PUSHRM;
                operand_int = (int) (operand_double * pow (10, ACCURACY));
                (*code)[idx++] = operand_int;

                // (Вывод в листинг компиляции если 2 проход)
                if (pass == 2)
                    fprintf(user_lst, "%08zu %08x %08x PUSHRM %lg\n", shift, CMD_PUSHRM, operand_int, operand_double);
                shift += 6 * sizeof(char) + sizeof(int); //Увеличение сдвига в листинге компиляции
                continue;
            }
            else if (strcmp (command, "POPRM") == 0) {
                double operand_double = 0; //Введенный операнд (если имеется)
                int operand_int = 0; //Введенный операнд (если имеется) с заданной точоностью
                read_words = sscanf (commands_list[i], "%*s %lg %c", &operand_double, &comment_tag);
                if ((read_words != 1) && (comment_tag != ';')) { //Проверка на наличие в строке комментариев
                    printf ("ERROR in line '%s': Wrong syntax\n", commands_list[i]);
                    exit (2);
                }
                (*code)[idx++] = CMD_POPRM;
                operand_int = (int) (operand_double * pow (10, ACCURACY));
                (*code)[idx++] = operand_int;

                // (Вывод в листинг компиляции если 2 проход)
                if (pass == 2)
                    fprintf(user_lst, "%08zu %08x %08x POPRM %lg\n", shift, CMD_POPRM, operand_int, operand_double);
                shift += 5 * sizeof(char) + sizeof(int); //Увеличение сдвига в листинге компиляции
                continue;
            }
            else if (strcmp (command, "PUSHDR") == 0) {
                double operand_double = 0; //Введенный операнд (если имеется)
                int operand_int = 0; //Введенный операнд (если имеется) с заданной точоностью
                read_words = sscanf (commands_list[i], "%*s %lg %c", &operand_double, &comment_tag);
                if ((read_words != 1) && (comment_tag != ';')) { //Проверка на наличие в строке комментариев
                    printf ("ERROR in line '%s': Wrong syntax\n", commands_list[i]);
                    exit (2);
                }
                (*code)[idx++] = CMD_PUSHDR;
                operand_int = (int) (operand_double * pow (10, ACCURACY));
                (*code)[idx++] = operand_int;

                // (Вывод в листинг компиляции если 2 проход)
                if (pass == 2)
                    fprintf(user_lst, "%08zu %08x %08x PUSHDR %lg\n", shift, CMD_PUSHRM, operand_int, operand_double);
                shift += 6 * sizeof(char) + sizeof(int); //Увеличение сдвига в листинге компиляции
                continue;
            }
            else if (strcmp (command, "POPDR") == 0) {
                double operand_double = 0; //Введенный операнд (если имеется)
                int operand_int = 0; //Введенный операнд (если имеется) с заданной точоностью
                read_words = sscanf (commands_list[i], "%*s %lg %c", &operand_double, &comment_tag);
                if ((read_words != 1) && (comment_tag != ';')) { //Проверка на наличие в строке комментариев
                    printf ("ERROR in line '%s': Wrong syntax\n", commands_list[i]);
                    exit (2);
                }
                (*code)[idx++] = CMD_POPDR;
                operand_int = (int) (operand_double * pow (10, ACCURACY));
                (*code)[idx++] = operand_int;

                // (Вывод в листинг компиляции если 2 проход)
                if (pass == 2)
                    fprintf(user_lst, "%08zu %08x %08x POPDR %lg\n", shift, CMD_POPRM, operand_int, operand_double);
                shift += 5 * sizeof(char) + sizeof(int); //Увеличение сдвига в листинге компиляции
                continue;
            }
            //Если в строке содержится метка или CALL
            else if ( (strcmp (command, "JMP") == 0) || (strcmp (command, "JA") == 0) ||
                        (strcmp (command, "JAE") == 0) || (strcmp (command, "JB") == 0) || (strcmp (command, "JBE") == 0) ||
                        (strcmp (command, "JE") == 0) || (strcmp (command, "JNE") == 0) || (strcmp (command, "CALL") == 0) ) {
                size_t location_idx = 0; //Номер команды, куда направляет JMP
                char *location = (char *) calloc (STR_LEN, sizeof(char)); //Название метки, куда направляет JMP
                read_words = sscanf(commands_list[i], "%*s %s %c", location, &comment_tag);

                for (size_t label_idx = 0; label_idx < labels_idx; ++label_idx) {
                    if (strcmp (location, labels[label_idx].name) == 0)
                        location_idx = labels[label_idx].num;
                }
                if ((read_words != 1) && (comment_tag != ';')) { //Проверка на наличие в строке комментариев
                    printf("ERROR in line '%s': Wrong syntax\n", commands_list[i]);
                    exit(2);
                }

                //Проверка на наличие в строке JMP
                // (Вывод в листинг компиляции если 2 проход)
                #define DEF_JMP(name) else if (strcmp (command, #name) == 0) { (*code)[idx++] = CMD_##name; \
                if (pass == 2) fprintf (user_lst, "%08zu %08x %08x "#name" %zu\n", shift, CMD_##name, location_idx, location_idx); }

                DEF_JMP(JMP)
                DEF_JMP(JA)
                DEF_JMP(JAE)
                DEF_JMP(JB)
                DEF_JMP(JBE)
                DEF_JMP(JE)
                DEF_JMP(JNE)
                DEF_JMP(CALL)

                #undef DEF_JMP
                (*code)[idx++] = location_idx;
                //Увеличение сдвига в листинге компиляции
                shift += (STR_CMP(JA) || STR_CMP(JB) || STR_CMP(JE) ? 2 : STR_CMP(CALL) ? 4 : 3) * sizeof (char) + sizeof (int);

                free(location);
                continue;
            }

            //Обработка остальных команд
            // (Вывод в листинг компиляции если 2 проход)
            // + Увеличение сдвига в листинге компиляции
            #define DEF_CMD(name, num, _code) \
            else if (strcmp (command, #name) == 0) { (*code)[idx++] = num; \
            if (pass == 2) fprintf (user_lst, "%08zu %08x %08x "#name"\n", shift, CMD_##name, 0); shift += strlen (#name) * sizeof (char);}
            #include "data/program.commands"

#undef DEF_CMD

            //Обработка меток
            else {
                bool is_label = false;
                for (size_t label_idx = 0; label_idx < labels_idx; ++label_idx) {
                    if (strcmp(command, labels[label_idx].name) == 0) {
                        labels[label_idx].num = idx;
                        is_label = true;
                    }
                }

            //Обработка некорректных названий команд
            if (!is_label) {
                    printf("ERROR in line '%s': No such command\n", commands_list[i]);
                    exit(2);
                }
            }

            read_words = sscanf(commands_list[i], "%*s %c", &comment_tag);
            if ((read_words == 1) &&
                ((comment_tag != ';') && (comment_tag != ' '))) { //Проверка на наличие в строке комментариев
                printf("ERROR in line '%s': Wrong syntax\n", commands_list[i]);
                exit(2);
            }
        }
    }
}