#include <cctype>
#include <cstring>
#include <cmath>
#include "Assembler.h"
#include "data/program.commands"

//TODO Обработать случаи с двумя совпадающими метками

void AssemblerMain () {
    char **commands_list = nullptr; //Массив с указателями на команды
    size_t program_len = 0; //Количество строк в программе
    size_t special = 0; //Количество специальных символов в программе

    Label_t *labels = nullptr;
    labels = (Label_t *) calloc (LABEL_ARRAY_SIZE, sizeof(Label_t)); //Массив меток
    size_t labels_idx = 0; //Индекс в массиве меток
    FILE *user_input = fopen ("user_data/user.input", "rb"); //Ввод пользователя
    char *input_start = nullptr; //Начало ввода пользователя
    ReadUserInput (user_input, &commands_list, &program_len, &special, &labels, &labels_idx, &input_start);
    fclose (user_input);

    int *code = nullptr; //Массив машинного кода процессора
    FILE *user_lst = fopen ("user_data/user.lst", "w"); //Листинг компиляции
    UserInputHandle (&code, commands_list, user_lst, program_len, special, labels, labels_idx);
    fclose (user_lst);
    FILE *program_code = fopen ("program_data/program.code", "w"); //Машинный код процессора
    fwrite (code, sizeof (int), program_len + special - labels_idx, program_code);

    //Освобождение памяти
    free (code);
    free (labels);
    free (input_start);
    fclose (program_code);
}

void ReadUserInput (FILE *user_input, char ***commands_list, size_t *program_len, size_t *special, Label_t **labels,
        size_t *labels_idx, char **input_start) {
    struct stat file_info = {}; //Информация о файле с вводом пользователя
    stat ("user_data/user.input", &file_info);
    *program_len = GetCommandsNum (user_input, file_info, input_start);

    //Выделение памяти для массива с указателями на команды
    *commands_list = (char **) calloc (*program_len, sizeof (char *));
    size_t idx = 0; //Индекс в массиве с указателями на команды
    (*commands_list)[idx++] = *input_start;

    //Расстановка указателей в массив commands_list
    //ptr - указатель в буфере
    while (isspace (**input_start)) ++input_start;
    for (char *ptr = *input_start; ( (ptr < *input_start + file_info.st_size) && (*ptr != EOF) && (idx != *program_len) ); ++ptr) {
        while ( (*ptr != '\0') && (*ptr != EOF) ) ++ptr;
        ++ptr;
        while ( (isspace (*ptr)) && (*ptr != EOF) ) ++ptr;
        if (*ptr != EOF) (*commands_list)[idx++] = ptr;
    }

    char *command = (char *) calloc (STRLEN, sizeof (char)); //Введенная команда

    //Поиск количества особых команд, содержащих аргументы
    for (size_t i = 0; i < *program_len; ++i) {
        sscanf ((*commands_list)[i], "%s", command);

        //Проверка на команду PUSH
        if STR_CMP(PUSH) {
            char reg_name = ' ';
            sscanf ((*commands_list)[i], "%*s %cX", &reg_name);
            if (isdigit (reg_name) || (reg_name == '-'))
                ++*special;
            else
                *((*commands_list)[i] + PUSH_LEN) = '_';
        }

        //Проверка на команду POP
        else if STR_CMP(POP) {
            char reg_name = ' ';
            sscanf ((*commands_list)[i], "%*s %cX", &reg_name);
            *((*commands_list)[i] + POP_LEN) = '_';
        }

        //Проверка на команду JMP или CALL
        else if ( STR_CMP(JMP) || STR_CMP(JA) || STR_CMP(JAE) || STR_CMP(JB) || STR_CMP(JBE) || STR_CMP(JE) || STR_CMP(JNE) || STR_CMP(CALL) ) {
            ++*special;
        }

        //Проверка на наличие комментария
        else if (command[strlen (command) - 1] == ':') {
            (*labels)[*labels_idx].name = (*commands_list)[i];
            (*labels)[*labels_idx].name[strlen ((*labels)[*labels_idx].name) - 1] = '\0';
            (*labels)[(*labels_idx)++].num = -1;
        }
    }

    //Освобождение памяти
    free (command);
}

size_t GetCommandsNum (FILE *user_input, struct stat file_info, char **input_start) {
    char *buf = (char *) calloc (file_info.st_size + BUF_EXTRA_SIZE, sizeof (char)); //Буфер из файла ввода пользователя
    fread (buf, sizeof (char), file_info.st_size, user_input); //Загрузка ввода пользователя в буфер

    size_t program_len = 0; //Колиество строк в программе
    size_t i = 0; //Индекс в буфере

    //Пропуск пробелов до первой команды
    while (isspace (buf[i])) ++i;

    //Сохранение указателя на начало буфера
    *input_start = buf;

    //Обработка ввода пользователя
    for ( ; ( (i < file_info.st_size) && (buf[i] != EOF) ); ++i) {

        //Новая строка в программе
        if (buf[i] == '\n')
            ++program_len, buf[i] = '\0';

        //Пропуск пробелов в начале новой строки
        while ((isspace (buf[i]) || buf[i] == '\0') && (i < file_info.st_size - 1))
            ++i;

        //Если достигли конца ввода
        if (buf[i] == EOF) {
            ++program_len;
            break;
        }
        if ((i == file_info.st_size - 1) && (buf[i] != '\n')) {
            ++program_len;
        }
    }

    //Возврат количества строк в программе
    return program_len;
}

void UserInputHandle (int **code, char **commands_list, FILE *user_lst, size_t program_len, size_t special,
        Label_t *labels, size_t labels_idx) {
    *code = (int *) calloc (program_len + special - labels_idx,
                           sizeof(int)); //Выделение памяти для массива с машинным кодом
    size_t idx = 0; //Индекс в массиве с машинным кодом
    size_t shift = 0; //Сдвиг в листинге компиляции
    char *command = (char *) calloc(STRLEN, sizeof(char)); //Введенная команда
    char comment_tag = ';'; //Введенный символ комментария
    long long read_words = 0; //Количество слов в строке

    //Обработка команд из массива с указателями на команды
    for (int pass = 1; pass <= 2; ++pass) {
        idx = 0;
        shift = 0;
        for (size_t i = 0; i < program_len; ++i) {
            sscanf (commands_list[i], "%s", command);
            //Обработка команд, содержащих аргументы
            if STR_CMP(PUSH) {
                double operand_double = 0; //Введенный операнд (если имеется)
                int operand_int = 0; //Введенный операнд (если имеется) с заданной точоностью
                read_words = sscanf (commands_list[i], "%*s %lg %c", &operand_double, &comment_tag);
                if ((read_words != 1) && (comment_tag != ';')) { //Проверка на наличие в строке комментариев
                    printf("ERROR in line '%s': Wrong syntax\n", commands_list[i]);
                    exit(2);
                }
                (*code)[idx++] = CMD_PUSH;
                operand_int = (int) (operand_double * pow(10, ACCURACY));
                (*code)[idx++] = operand_int;

                // (Вывод в листинг компиляции если 2 проход)
                if (pass == 2) fprintf (user_lst, "%08zu %08x %08x PUSH %lg\n", shift, CMD_PUSH, operand_int, operand_double);
                shift += 4 * sizeof (char) + sizeof (int); //Увеличение сдвига в листинге компиляции
                continue;

            //Если в строке содержится метка или CALL
            } else if ( STR_CMP(JMP) || STR_CMP(JA) || STR_CMP(JAE) || STR_CMP(JB) || STR_CMP(JBE) || STR_CMP(JE) || STR_CMP(JNE) || STR_CMP(CALL) ) {
                size_t location_idx = 0; //Номер команды, куда направляет JMP
                char *location = (char *) calloc(STRLEN, sizeof(char)); //Название метки, куда направляет JMP
                read_words = sscanf(commands_list[i], "%*s %s %c", location, &comment_tag);

                for (size_t label_idx = 0; label_idx < labels_idx; ++label_idx) {
                    if (strcmp(location, labels[label_idx].name) == 0)
                        location_idx = labels[label_idx].num;
                }

                if ((read_words != 1) && (comment_tag != ';')) { //Проверка на наличие в строке комментариев
                    printf("ERROR in line '%s': Wrong syntax\n", commands_list[i]);
                    exit(2);
                }

                //Проверка на наличие в строке JMP
                // (Вывод в листинг компиляции если 2 проход)
                #define DEF_JMP(name) else if STR_CMP(name) { (*code)[idx++] = CMD_##name; \
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
            else if STR_CMP(name) { (*code)[idx++] = num; \
            if (pass == 2) fprintf (user_lst, "%08zu %08x %08x "#name"\n", shift, CMD_##name, 0); shift += strlen (#name) * sizeof (char);}
            #include "data/program.operations"
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

    //Освобождение памяти
    free (commands_list);
    free (command);
}