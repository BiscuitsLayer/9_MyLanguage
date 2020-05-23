#include "Bintrans.h"
#include <elf.h>
#include <cstdarg>

static size_t NUM_IF = 1;
static size_t NUM_POW = 1;
static size_t locals_num = 0;

ELF_VAR_t elf_vars [ARRAY_SIZE];
static int temp [ARRAY_SIZE];

const int START_ADDRESS = 0x400000; // Адрес начала программы
const int CODE_SIZE = 10000;
static char code [CODE_SIZE]; // Сначала пишем программу сюда, потом меняем размеры и адреса
static size_t code_idx = 0; // Индекс в массиве, который содержит программу

const int EMPTY = -1;
static LABEL_t labels [ARRAY_SIZE];
static size_t labels_idx = 0;

static LABEL_REQUEST_t label_reqs [ARRAY_SIZE];
static size_t label_reqs_idx = 0;

int ELF::GetLabelAddress (char name[ARRAY_SIZE], LABEL_REQUEST_TYPE type) {
	int flag = 0;
	for (int i = 0; i < labels_idx; ++i) {
		if ((int)(labels[i].code_ptr - code_idx) < 0)
			flag = 1;
		if (strcmp (labels[i].name, name) == 0) {
			if ((int)labels[i].code_ptr != EMPTY) {
				switch (type) {
					case REQUEST_TYPE_CALL:  // REQUEST_TYPE_JUMP <- тоже самое
						return (labels[i].code_ptr - code_idx + 0xFFFFFFFF - _CALL_LEN + flag) % (0xFFFFFFFF);
					case REQUEST_TYPE_J_COND:
						return (labels[i].code_ptr - code_idx + 0xFFFFFFFF - _J_COND_LEN + flag) % (0xFFFFFFFF);
					case REQUEST_TYPE_MOV:
						return labels[i].code_ptr + START_ADDRESS;
					case REQUEST_TYPE_GLOBAL_MOV:
						return labels[i].code_ptr + START_ADDRESS;
				}
			}
			else {
				strcpy (label_reqs[label_reqs_idx].name, name); // Работаем с массивом запросов (сохраняем искомое имя)
				label_reqs[label_reqs_idx].code_idx_request = code_idx; // Сохраняем место в коде, где был запрос на метку
				label_reqs[label_reqs_idx].type = type;
				label_reqs_idx++; // Увеличиваем количество запроосов
				return 0x90909090;
			}
		}
	}
	strcpy (labels[labels_idx].name, name); // Работаем с массивом меток
	labels[labels_idx++].code_ptr = EMPTY;
	strcpy (label_reqs[label_reqs_idx].name, name); // Работаем с массивом запросов (сохраняем искомое имя)
	label_reqs[label_reqs_idx].code_idx_request = code_idx; // Сохраняем место в коде, где был запрос на метку
	label_reqs[label_reqs_idx].type = type;
	label_reqs_idx++; // Увеличиваем количество запроосов
	return 0x90909090;
}

void ELF::SetLabelAddress (char name[ARRAY_SIZE]) {
	for (int i = 0; i < labels_idx; ++i) {
		if (strcmp (labels[i].name, name) == 0) {
			labels[i].code_ptr = code_idx;
			return;
		}
	}
	strcpy(labels[labels_idx].name, name);
	labels[labels_idx++].code_ptr = code_idx;
}

void ELF::ELF_Header () {
	Insert (4, 0x7F, 0x45, 0x4C, 0x46) // EI_MAG = ELF
	Insert (1, 0x02) // EI_CLASS = 64 BIT
	Insert (1, 0x01) // EI_DATA = DATA2LSB (Little-Endian)
	Insert (1, 0x01) // EI_VERSION = EV_CURRENT
	Insert (1, 0x00) // EI_OS/ABI = UNIX SYSTEM V ABI
	Insert (1, 0x00) // EI_OS/ABI VER = 0
	MultiInsert (7, 0x00) // EmptyStuff
	Insert (2, 0x02, 0x00) // E_TYPE = EXEC
	Insert (2, 0x3E, 0x00) // E_MACHINE = EM_X86_64 (AMD x86- 64 architecture)
	Insert (4, 0x01, 0x00, 0x00, 0x00) // E_VERSION = EV_CURRENT
	// Entry offset
	Insert (4, 0x80, 0x00, 0x40, 0x00)
	MultiInsert (4, 0x00) // Empty Stuff
	// Program header offset
	Insert (4, 0x40, 0x00, 0x00, 0x00)
	MultiInsert (4, 0x00)
	// Section header offset
	MultiInsert (8, 0x00)
	// Flags
	MultiInsert (4, 0x00)
	// ELF header size
	Insert (2, 0x40, 0x00)
	// Program header size
	Insert (2, 0x38, 0x00)
	// Program headers quantity (number)
	Insert (2, 0x01, 0x00)
	// Section header size
	Insert (2, 0x40, 0x00)
	// Section headers quantity (number)
	MultiInsert (2, 0x00)
	// Section headers index table
	MultiInsert (2, 0x00)
}

void ELF::Program_Header () {
	Insert (4, 0x01, 0x00, 0x00, 0x00) // P_TYPE = Loadable program segment
	Insert (4, 0x07, 0x00, 0x00, 0x00) // P_FLAGS = Segment is readable, writable and executable
	MultiInsert (8, 0x00) // P_OFFSET
	// Virtual address
	Insert (4, 0x00, 0x00, 0x40, 0x00)
	MultiInsert (4, 0x00)
	// Physical address
	Insert (4, 0x00, 0x00, 0x40, 0x00)
	MultiInsert (4, 0x00)
	// code[64d + 32d] = code [40h + 20h]
	// File size
	MultiInsert (8, 0x00) // Заполним, когда будет известен размер программы
	// Memory size
	MultiInsert (8, 0x00) // Заполним, когда будет известен размер программы
	// code[64d + 48d] = code [40h + 30h]
	// Align
	Insert (4, 0x00, 0x00, 0x20, 0x00)
	MultiInsert (4, 0x00)
	MultiInsert (8, 0x00) // Empty Stuff
}

void ELF::CreateELF (Node *node) {
	FILE *writefile = fopen ("../my_programs/result/my_main", "wb");
	if (!writefile) {
		printf ("Error opening ELF File\n");
		assert (writefile);
		return;
	}

	ELF::ELF_Header ();
	ELF::Program_Header ();
	ELF::TreeToELF (node);

	// Теперь заполним размер программы
	size_t sz = code_idx; // Размер программы
	// Теперь размер есть в sz, а значит в переменной code_idx он больше не нужен

	// File size
	code_idx = 0x40 + 0x20;
	InsertSize (sz)
	// Memory size
	code_idx = 0x40 + 0x28;
	InsertSize (sz)

	fwrite (code, 1, sz, writefile);
	fclose (writefile);
}

void ELF::GetVarIdx () {
	// Временно запомним число переменных в каждой функции
	for (size_t i = 0; i < func_idx; ++i) {
		temp[i] = funcs[i].val;
		funcs[i].val = 0;
	}
	size_t glob_idx = 1;
	for (size_t i = 0; i < var_idx; ++i) {
		if ((int) vars[i].val == GLOBAL) {
			elf_vars[i].is_global = true;
			elf_vars[i].shift = glob_idx++;
		}
		else if (funcs[(int)vars[i].val].val < temp[(int)vars[i].val]) {
			elf_vars[i].is_global = false;
			elf_vars[i].shift = 8 * (++ funcs[(int) vars[i].val].val + 1); // ещё +1 из-за того, что сначала лежит адрес возврата
			if (funcs[(int) vars[i].val].val == temp[(int)vars[i].val])
				temp[(int)vars[i].val] = 0;
		}
		else {
			elf_vars[i].is_global = false;
			elf_vars[i].shift = 8 * (-- temp[(int)vars[i].val]);
		}
	}
	// После выполнения цикла все переменные получат свои сдвиги и количества аргументов у функций вернутся к начальному значению
	// А в массиве temp будут лежать количества локальных переменных (разве что, со знаком минус)
}

void ELF::TreeToELF (Node *node) {
	ELF::GetVarIdx ();
	Node *base = node;
	while (node->left && node->left->type != TYPE_FUNC) {
		node = node->right;
	}
	// Вход в программу
	code_idx += 22; // Выставляем адрес
	SetLabelAddress ("main"); // Функции main
	code_idx -= 22; // Возвращаемся назад
	Insert (5, _CALL (GetLabelAddress ("main", REQUEST_TYPE_CALL))) // Вызов main

	// Выход из программы
	Insert (7, _MOV_R_N (_RAX, 0x3C))
	Insert (7, _MOV_R_N (_RDI, 0))
	Insert (2, _SYSCALL)
	Insert (1, _RET)
	ELF::NodeToELF (node);
	ELF::ExtraFuncs ();
	ELF::AddGlobals ();
	ELF::HandleLabels ();
}

void ELF::NodeToELF (Node *node, size_t IF_NUMBER) {
	if (node->type == TYPE_NUM) {
		Insert (5, _PUSH_N ((int) (REAL_ACCURACY * node->data)))
	}
	else if (node->type == TYPE_VAR) {
		if (elf_vars[(int)node->data].is_global) {
			char g_num [ARRAY_SIZE];
			sprintf (g_num, "g%zu", elf_vars[(int) node->data].shift);
			Insert (7, _PUSH_M (GetLabelAddress (g_num, REQUEST_TYPE_MOV)))
		}
		else
			Insert (3, _PUSH_MR_N (_RBP, elf_vars[(int)node->data].shift)) // Кладём в стек глобальные переменные
	}
	else if (node->type == TYPE_OP) {
		ELF::NodeToELF (node->left);
		ELF::NodeToELF (node->right);
		switch ((int)node->data) {
			case OP_SUM: {
				Insert (1, _POP_R (_RCX))
				Insert (1, _POP_R (_RBX))
				Insert (3, _ADD_R_R (_RBX, _RCX))
				Insert (1, _PUSH_R (_RBX))
				break;
			}
			case OP_SUB: {
				Insert (1, _POP_R (_RCX))
				Insert (1, _POP_R (_RBX))
				Insert (3, _SUB_R_R (_RBX, _RCX))
				Insert (1, _PUSH_R (_RBX))
				break;
			}
			case OP_MUL: {
				Insert (1, _POP_R (_RBX)) // Начало умножения
				Insert (1, _POP_R (_RAX))
				Insert (3, _XOR_R_R (_RDX, _RDX)) // Иначе множитель будет DX_AX
				Insert (5, _CALL (GetLabelAddress ("mymul", REQUEST_TYPE_CALL)))
				// ACCURACY
				Insert (3, _XOR_R_R (_RDX, _RDX))
				Insert (7, _MOV_R_N (_RBX, REAL_ACCURACY))
				Insert (5, _CALL (GetLabelAddress ("mydiv", REQUEST_TYPE_CALL)))
				// ACCURACY
				Insert (1, _PUSH_R (_RAX)) // Конец умножения
				break;
			}
			case OP_DIV: {
				Insert (1, _POP_R (_RBX)) // Начало деления
				Insert (1, _POP_R (_RAX))
				// ACCURACY
				Insert (3, _XOR_R_R (_RDX, _RDX)) // Иначе делимое будет DX_AX
				Insert (3, _MOV_R_R (_RCX, _RBX))
				Insert (7, _MOV_R_N (_RBX, REAL_ACCURACY))
				Insert (5, _CALL (GetLabelAddress ("mymul", REQUEST_TYPE_CALL)))
				Insert (3, _MOV_R_R (_RBX, _RCX))
				// ACCURACY
				Insert (5, _CALL (GetLabelAddress ("mydiv", REQUEST_TYPE_CALL)))
				Insert (1, _PUSH_R (_RAX)) // Конец деления
				break;
			}
			case OP_POW: {
				size_t num = NUM_POW++;
				char str_num1 [ARRAY_SIZE];
				char str_num2 [ARRAY_SIZE];
				sprintf (str_num1, "sqrt_%d", num);
				sprintf (str_num2, "endpow_%d", num);
				Insert (1, _POP_R (_RCX)) // Начало возведения в степень
				Insert (1, _POP_R (_RAX))
				Insert (7, _CMP_R_N (_RCX, (int) (0.5 * REAL_ACCURACY)))
				Insert (6, _JE (GetLabelAddress (str_num1, REQUEST_TYPE_J_COND)))
				Insert (5, _CALL (GetLabelAddress ("pow", REQUEST_TYPE_CALL)))
				Insert (5, _JMP (GetLabelAddress (str_num2, REQUEST_TYPE_JUMP)))
				SetLabelAddress (str_num1);
				Insert (5, _CALL (GetLabelAddress ("sqrt", REQUEST_TYPE_CALL)))
				SetLabelAddress (str_num2);
				Insert (1, _PUSH_R (_RAX)) // Конец возведения в степень
				break;
			}
			case OP_ABOVE: {
				char str_num [ARRAY_SIZE];
				sprintf (str_num, "elseif_%d", IF_NUMBER);
				Insert (1, _POP_R (_RAX)) // Начало прыжка
				Insert (1, _POP_R (_RBX))
				Insert (3, _CMP_R_R (_RAX, _RBX))
				Insert (6, _JGE (GetLabelAddress (str_num, REQUEST_TYPE_J_COND))) // Конец прыжка
				break;
			}
			case OP_ABOVE_EQUAL: {
				char str_num [ARRAY_SIZE];
				sprintf (str_num, "elseif_%d", IF_NUMBER);
				Insert (1, _POP_R (_RAX)) // Начало прыжка
				Insert (1, _POP_R (_RBX))
				Insert (3, _CMP_R_R (_RAX, _RBX))
				Insert (6, _JG (GetLabelAddress (str_num, REQUEST_TYPE_J_COND))) // Конец прыжка
				break;
			}
			case OP_BELOW: {
				char str_num [ARRAY_SIZE];
				sprintf (str_num, "elseif_%d", IF_NUMBER);
				Insert (1, _POP_R (_RAX)) // Начало прыжка
				Insert (1, _POP_R (_RBX))
				Insert (3, _CMP_R_R (_RAX, _RBX))
				Insert (6, _JLE (GetLabelAddress (str_num, REQUEST_TYPE_J_COND))) // Конец прыжка // Конец прыжка
				break;
			}
			case OP_BELOW_EQUAL: {
				char str_num [ARRAY_SIZE];
				sprintf (str_num, "elseif_%d", IF_NUMBER);
				Insert (1, _POP_R (_RAX)) // Начало прыжка
				Insert (1, _POP_R (_RBX))
				Insert (3, _CMP_R_R (_RAX, _RBX))
				Insert (6, _JL (GetLabelAddress (str_num, REQUEST_TYPE_J_COND))) // Конец прыжка
				break;
			}
			case OP_EQUAL: {
				char str_num [ARRAY_SIZE];
				sprintf (str_num, "elseif_%d", IF_NUMBER);
				Insert (1, _POP_R (_RAX)) // Начало прыжка
				Insert (1, _POP_R (_RBX))
				Insert (3, _CMP_R_R (_RAX, _RBX))
				Insert (6, _JNE (GetLabelAddress (str_num, REQUEST_TYPE_J_COND))) // Конец прыжка
				break;
			}
			case OP_UNEQUAL: {
				char str_num [ARRAY_SIZE];
				sprintf (str_num, "elseif_%d", IF_NUMBER);
				Insert (1, _POP_R (_RAX)) // Начало прыжка
				Insert (1, _POP_R (_RBX))
				Insert (3, _CMP_R_R (_RAX, _RBX))
				Insert (6, _JE (GetLabelAddress (str_num, REQUEST_TYPE_J_COND))) // Конец прыжка
				break;
			}
		}
	}
	else if (node->type == TYPE_FUNC) {
		if (node->parent->type == TYPE_SYS && node->parent->data == SEMICOLON) { // Текст функции
			SetLabelAddress (funcs[(int) node->data].name);
			Insert (1, _PUSH_R (_RBP)) // Стандартный вход в функцию
			Insert (3, _MOV_R_R (_RBP, _RSP))
			locals_num =  -1 * temp[(int) node->data]; // Количество локальных переменных * 8
			Insert (7, _SUB_R_N (_RSP, 8 * locals_num)) // Выделяем место под локальные переменные
			ELF::NodeToELF (node->right);
		}
		else { // Вызов функции
			Node *base = node;
			while (node->left && node->left->type == TYPE_SYS && node->left->data == COMMA)
				node = node->left;
			while (node != base) { // Передача параметров в обратном порядке
				ELF::NodeToELF (node->right);
				node = node->parent;
			}
			Insert (5, _CALL (GetLabelAddress (funcs[(int) node->data].name, REQUEST_TYPE_CALL)))
			Insert (7, _ADD_R_N (_RSP, (int) (8 * funcs[(int) node->data].val))) // Подчищаем стек на количество переданных аргументов
			Insert (1, _PUSH_R (_RAX)) // Return value
		}
	}
	else if (node->type == TYPE_SYS) {
		switch ((int)node->data) {
			case EQUAL: {
				ELF::NodeToELF (node->right);
				if (elf_vars[(int)node->left->data].is_global) {
					char g_num [ARRAY_SIZE];
					sprintf (g_num, "g%zu", elf_vars[(int) node->left->data].shift);
					Insert (7, _POP_M (GetLabelAddress (g_num, REQUEST_TYPE_MOV)))
				}
				else {
					Insert (3, _POP_MR_N (_RBP, elf_vars[(int) node->left->data].shift))
				}
				break;
			}
			case RET: {
				if (node->left->type == TYPE_NUM)
				Insert (7, _MOV_R_N (_RAX, (int) (REAL_ACCURACY * node->left->data))) // Return value в RAX
				else if (node->left->type == TYPE_VAR) {
					if (elf_vars[(int)node->left->data].is_global) {
						char g_num [ARRAY_SIZE];
						sprintf (g_num, "g%zu", elf_vars[(int) node->left->data].shift);
						Insert (7, _MOV_R_M (_RAX, GetLabelAddress (g_num, REQUEST_TYPE_MOV)))
					}
					else
						Insert (4, _MOV_R_MR_N (_RAX, _RBP, elf_vars[(int)node->left->data].shift))
				}
				else {
					ELF::NodeToELF (node->left);
					if (node->left) {
						Insert (1, _POP_R (_RAX)) // Return value в RAX
					}
				}
				Insert (7, _ADD_R_N (_RSP, 8 * locals_num)) // Подчищаем место, выделенное под локальные переменные
				Insert (1, _POP_R (_RBP))
				Insert (1, _RET) // Стандартный выход из функции
				break;
			}
			case PUT: {
				if (node->left->type == TYPE_NUM) {
					Insert (5, _PUSH_N ((int) (REAL_ACCURACY * node->left->data))) // Начало Put
					Insert (1, _POP_R (_RAX))
				}
				else if (node->left->type == TYPE_VAR) {
					if (elf_vars[(int)node->left->data].is_global) {
						char g_num [ARRAY_SIZE];
						sprintf (g_num, "g%zu", elf_vars[(int) node->left->data].shift);
						Insert (7, _MOV_R_M (_RAX, GetLabelAddress (g_num, REQUEST_TYPE_MOV)))
					}
					else {
						Insert (4, _MOV_R_MR_N (_RAX, _RBP, elf_vars[(int) node->left->data].shift))
					}
				}
				else {
					ELF::NodeToELF (node->left);
					Insert (1, _POP_R (_RAX)) // Выражение закончилось
				}
				Insert (5, _CALL (GetLabelAddress ("put", REQUEST_TYPE_CALL))) // Конец Put
				break;
			}
			case GET: {
				if (elf_vars[(int)node->left->data].is_global) {
					char g_num [ARRAY_SIZE];
					sprintf (g_num, "g%zu", elf_vars[(int) node->left->data].shift);
					Insert (5, _CALL (GetLabelAddress ("get", REQUEST_TYPE_CALL))) // Начало Get
					Insert (8, _MOV_M_R (_RAX, GetLabelAddress (g_num, REQUEST_TYPE_GLOBAL_MOV))) //Конец Get
				}
				else {
					Insert (5, _CALL (GetLabelAddress ("get", REQUEST_TYPE_CALL))) // Начало Get
					Insert (4, _MOV_MR_N_R (_RBP, elf_vars[(int) node->left->data].shift, _RAX))
				}
				break;
			}
			case IF: {
				size_t num = NUM_IF++; // Потому что внутри конструкции IF может быть другой IF с уже большим номером
				char str_num1 [ARRAY_SIZE];
				char str_num2 [ARRAY_SIZE];
				sprintf (str_num1, "elseif_%d", num);
				sprintf (str_num2, "endif_%d", num);
				ELF::NodeToELF (node->left, num);
				ELF::NodeToELF (node->right->left);
				Insert (5, _JMP (GetLabelAddress (str_num2, REQUEST_TYPE_JUMP)))
				SetLabelAddress (str_num1);
				ELF::NodeToELF (node->right->right);
				SetLabelAddress (str_num2);
				break;
			}
			case SEMICOLON: {
				ELF::NodeToELF (node->left);
				if (node->right)
					ELF::NodeToELF (node->right);
				break;
			}
			case OP: {
				ELF::NodeToELF (node->left);
				if (node->right)
					ELF::NodeToELF (node->right);
				break;
			}
		}
	}
}

void ELF::ExtraFuncs () {
	/*  MYMUL   */
	SetLabelAddress ("mymul");
	Insert (2, _PUSH_PROR (_R8)) // Сохраняем значение
	Insert (3, _XOR_PROR_PROR (_R8, _R8))
	Insert (7, _ADD_R_N (_RAX, 0))
	Insert (6, _JNS (GetLabelAddress ("secondmul", REQUEST_TYPE_J_COND)))
	Insert (3, _NEG_R (_RAX))
	Insert (3, _INC_PROR (_R8))
	SetLabelAddress ("secondmul");
	Insert (7, _ADD_R_N (_RBX, 0))
	Insert (6, _JNS (GetLabelAddress ("checkmul", REQUEST_TYPE_J_COND)))
	Insert (3, _NEG_R (_RBX))
	Insert (3, _DEC_PROR (_R8))
	SetLabelAddress ("checkmul");
	Insert (3, _IMUL (_RBX))
	Insert (3, _TEST_PROR_PROR (_R8, _R8)) // Проверяем R8 == 0 (только выставляем флаги)
	Insert (6, _JZ (GetLabelAddress ("endmul", REQUEST_TYPE_J_COND)))
	Insert (3, _NEG_R (_RAX))
	SetLabelAddress ("endmul");
	Insert (2, _POP_PROR (_R8)) // Возвращаем сохранённое значение
	Insert (1, _RET)

	/*  MYDIV   */
	SetLabelAddress ("mydiv");
	Insert (2, _PUSH_PROR (_R8)) // Сохраняем значение
	Insert (3, _XOR_PROR_PROR (_R8, _R8))
	Insert (7, _ADD_R_N (_RAX, 0))
	Insert (6, _JNS (GetLabelAddress ("seconddiv", REQUEST_TYPE_J_COND)))
	Insert (3, _NEG_R (_RAX))
	Insert (3, _INC_PROR (_R8))
	SetLabelAddress ("seconddiv");
	Insert (7, _ADD_R_N (_RBX, 0))
	Insert (6, _JNS (GetLabelAddress ("checkdiv", REQUEST_TYPE_J_COND)))
	Insert (3, _NEG_R (_RBX))
	Insert (3, _DEC_PROR (_R8))
	SetLabelAddress ("checkdiv");
	Insert (3, _IDIV (_RBX))
	Insert (3, _TEST_PROR_PROR (_R8, _R8)) // Проверяем R8 == 0 (только выставляем флаги)
	Insert (6, _JZ (GetLabelAddress ("enddiv", REQUEST_TYPE_J_COND)))
	Insert (3, _NEG_R (_RAX))
	SetLabelAddress ("enddiv");
	Insert (2, _POP_PROR (_R8)) // Возвращаем сохранённое значение
	Insert (1, _RET)

	/*   PUT    */
	SetLabelAddress ("put");
	Insert (3, _XOR_PROR_PROR (_R8, _R8)) // Дополнительный символ '-' если число отрицательное
	Insert (3, _XOR_PROR_PROR (_R9, _R9)) // Флаг вызова pointput
	Insert (7, _ADD_R_N (_RAX, 0)) // Проверка RAX на отрицательность
	Insert (6, _JNS (GetLabelAddress ("startput", REQUEST_TYPE_J_COND))) // Если неотрицательное, начинаем Put
	Insert (3, _NEG_R (_RAX)) // RAX *= -1
	Insert (7, _MOV_R_M (_RCX, GetLabelAddress ("output", REQUEST_TYPE_MOV)))
	Insert (3, _MOV_BYTEMR_N (_RCX, 0x2d)) // Символ '-'
	Insert (3, _INC_PROR (_R8)) // Потому что число отрицательное
	SetLabelAddress ("startput");
	Insert (7, _MOV_R_N (_RDI, 10)) // Десятичная система счисления
	Insert (3, _XOR_R_R (_RSI, _RSI)) // Счётчик цифр
	SetLabelAddress ("repput");
	Insert (3, _XOR_R_R (_RDX, _RDX)) // Иначе делимое будет DX_AX
	Insert (3, _XCHG_R_R (_RBX, _RDI)) // Чтобы вызвать mydiv
	Insert (5, _CALL (GetLabelAddress ("mydiv", REQUEST_TYPE_CALL))) // RAX /= 10, RDX = RAX % 10
	Insert (3, _XCHG_R_R (_RBX, _RDI)) // Чтобы вызвать mydiv
	Insert (7, _ADD_R_N (_RDX, '0'))
	Insert (1, _PUSH_R (_RDX))
	Insert (3, _INC_R (_RSI))
	Insert (7, _CMP_R_N (_RAX, 0))
	Insert (6, _JNE (GetLabelAddress ("repput", REQUEST_TYPE_J_COND)))
	Insert (7, _MOV_R_M (_RCX, GetLabelAddress ("output", REQUEST_TYPE_MOV)))
	Insert (3, _MOV_R_R (_RDX, _RSI)) // Длина вывода
	Insert (7, _ADD_R_N (_RDX, 2)) // ',' и '\n' тоже печатаются
	Insert (3, _ADD_R_PROR (_RDX, _R8)) // +1 если число отрицательное
	Insert (3, _ADD_R_PROR (_RCX, _R8)) // +1 если число отрицательное
	SetLabelAddress ("repput2");
	Insert (1, _POP_R (_RBX))
	Insert (2, _MOV_MR_MINIR (_RCX, _BL))
	Insert (3, _INC_R (_RCX)) // Следующий символ
	Insert (3, _DEC_R (_RSI)) // Уменьшаем счётчик символов
	Insert (7, _CMP_R_N (_RSI, EXP_REAL_ACCURACY)) // Пора ли вставлять ','
	Insert (6, _JE (GetLabelAddress ("pointput", REQUEST_TYPE_J_COND)))
	Insert (7, _CMP_R_N (_RSI, 0))
	Insert (6, _JNE (GetLabelAddress ("repput2", REQUEST_TYPE_J_COND)))
	Insert (3, _MOV_BYTEMR_N (_RCX, 0x0a))
	Insert (5, _JMP (GetLabelAddress ("endput", REQUEST_TYPE_JUMP)))
	SetLabelAddress ("pointput");
	Insert (3, _INC_PROR (_R9)) // Флаг pointput
	Insert (2, _MOV_MINIR_N (_BL, 0x2c)) // Символ ','
	Insert (2, _MOV_MR_MINIR (_RCX, _BL))
	Insert (3, _INC_R (_RCX))
	Insert (5, _JMP (GetLabelAddress ("repput2", REQUEST_TYPE_JUMP)))
	SetLabelAddress ("endput");
	Insert (7, _MOV_R_M (_RCX, GetLabelAddress ("output", REQUEST_TYPE_MOV)))
	Insert (3, _TEST_PROR_PROR (_R9, _R9)) // Проверим флаг pointput
	Insert (6, _JNZ (GetLabelAddress ("endput2", REQUEST_TYPE_J_COND)))
	Insert (1, _PUSH_R (_RDX)) // Сохраним RDX
	Insert (1, _PUSH_R (_RCX)) // Сохраним RCX
	Insert (3, _XOR_R_R (_RDX, _RDX)) // Просто чистим RDX (без деления и умножения)
	Insert (7, _MOV_R_M (_RCX, GetLabelAddress ("service_pos", REQUEST_TYPE_MOV))) // Печатаем '0,'
	Insert (7, _MOV_R_M (_RDI, GetLabelAddress ("service_pos_len", REQUEST_TYPE_MOV))) // Указатель на длину
	Insert (3, _MOV_MINIR_BYTEMR (_DL, _RDI)) // Длина
	Insert (3, _TEST_PROR_PROR (_R8, _R8)) // Проверим символ '-'
	Insert (6, _JZ (GetLabelAddress ("nextput", REQUEST_TYPE_J_COND)))
	Insert (1, _POP_R (_RCX))
	Insert (1, _POP_R (_RDX))
	Insert (3, _DEC_R (_RDX)) // В основной печати мы не пишем '-'
	Insert (3, _INC_R (_RCX)) // Начинаем печать с цифры
	Insert (1, _PUSH_R (_RDX))
	Insert (1, _PUSH_R (_RCX))
	Insert (7, _MOV_R_M (_RCX, GetLabelAddress ("service_neg", REQUEST_TYPE_MOV))) // Печатаем '-0,'
	Insert (7, _MOV_R_M (_RDI, GetLabelAddress ("service_neg_len", REQUEST_TYPE_MOV))) // Указатель на длину
	Insert (3, _MOV_MINIR_BYTEMR (_DL, _RDI)) // Длина
	SetLabelAddress ("nextput");
	Insert (7, _MOV_R_N (_RAX, 4)) // sys_write
	Insert (7, _MOV_R_N (_RBX, 1)) // Файловый дескриптор = stdout
	Insert (2, _INT_0x80)
	Insert (1, _POP_R (_RCX))
	Insert (1, _POP_R (_RDX))
	Insert (3, _DEC_R (_RDX)) // В основной печати мы не пишем ','
	SetLabelAddress ("endput2");
	Insert (7, _MOV_R_N (_RAX, 4)) // sys_write
	Insert (7, _MOV_R_N (_RBX, 1)) // Файловый дескриптор = stdout
	Insert (2, _INT_0x80)
	Insert (1, _RET)

	/*   GET    */
	SetLabelAddress ("get");
	Insert (7, _MOV_R_N (_RAX, 3)) // sys_read
	Insert (7, _MOV_R_N (_RBX, 1)) // Файловый дескриптор = stdin
	Insert (7, _MOV_R_M (_RCX, GetLabelAddress ("input", REQUEST_TYPE_MOV)))
	Insert (7, _MOV_R_N (_RDX, 16)) // Длина
	Insert (2, _INT_0x80)
	Insert (3, _XOR_PROR_PROR (_R8, _R8)) // Будет доп. символ '-' в случае отриц. числа
	Insert (3, _XOR_R_R (_RAX, _RAX)) // В RAX будет результат
	Insert (7, _MOV_R_N (_RDI, 10)) // Десятичная система счисления
	Insert (7, _MOV_R_M (_RCX, GetLabelAddress ("input", REQUEST_TYPE_MOV)))
	SetLabelAddress ("repget");
	Insert (3, _MOV_MINIR_BYTEMR (_BL, _RCX)) // BL - следующий символ
	// Проверяем знак
	Insert (3, _CMP_MINIR_N (_BL, 0x2d))
	Insert (6, _JE (GetLabelAddress ("signget", REQUEST_TYPE_J_COND)))
	// Проверяем знак
	// Проверяем конец строки
	Insert (3, _CMP_MINIR_N (_BL, 0xa))
	Insert (6, _JE (GetLabelAddress ("endget", REQUEST_TYPE_J_COND)))
	// Проверяем конец строки
	// Проверка ',' или '.'
	Insert (3, _CMP_MINIR_N (_BL, 0x2c))
	Insert (6, _JE (GetLabelAddress ("pointget", REQUEST_TYPE_J_COND)))
	Insert (3, _CMP_MINIR_N (_BL, 0x2e))
	Insert (6, _JE (GetLabelAddress ("pointget", REQUEST_TYPE_J_COND)))
	// Проверка ',' или '.'
	Insert (3, _SUB_MINIR_N (_BL, '0')) // BL (ascii) -> BL (digit)
	Insert (3, _XCHG_R_R (_RBX, _RDI)) // Для вызова mymul
	Insert (5, _CALL (GetLabelAddress ("mymul", REQUEST_TYPE_CALL))) // Результат *= 10
	Insert (3, _XCHG_R_R (_RBX, _RDI)) // Для вызова mymul
	Insert (3, _ADD_R_R (_RAX, _RBX)) // Результат += новый символ
	Insert (3, _INC_R (_RCX)) // Следующий символ
	Insert (5, _JMP (GetLabelAddress ("repget", REQUEST_TYPE_JUMP)))
	SetLabelAddress ("signget");
	Insert (3, _INC_PROR (_R8)) // Число отрицательное
	Insert (3, _INC_R (_RCX)) // Следующий символ
	Insert (5, _JMP (GetLabelAddress ("repget", REQUEST_TYPE_JUMP)))
	SetLabelAddress ("pointget");
	Insert (7, _MOV_R_N (_RSI, EXP_REAL_ACCURACY)) // RSI <- количество цифр после запятой
	Insert (3, _INC_R (_RCX)) // Следующий символ
	SetLabelAddress ("repget2");
	Insert (3, _MOV_MINIR_BYTEMR (_BL, _RCX)) // BL - следующий символ
	// Конец строки
	Insert (3, _CMP_MINIR_N (_BL, 0xa))
	Insert (6, _JE (GetLabelAddress ("endget2", REQUEST_TYPE_J_COND)))
	// Конец строки
	Insert (3, _SUB_MINIR_N (_BL, '0')) // BL (ascii) -> BL (digit)
	Insert (3, _XCHG_R_R (_RBX, _RDI)) // Для вызова mymul
	Insert (5, _CALL (GetLabelAddress ("mymul", REQUEST_TYPE_CALL))) // Результат *= 10
	Insert (3, _XCHG_R_R (_RBX, _RDI)) // Для вызова mymul
	Insert (3, _ADD_R_R (_RAX, _RBX)) // Результат += новый символ
	Insert (3, _INC_R (_RCX)) // Следующий символ
	Insert (3, _DEC_R (_RSI)) // Уменьшаем счётчик
	Insert (6, _JNZ (GetLabelAddress ("repget2", REQUEST_TYPE_J_COND)))
	Insert (5, _JMP (GetLabelAddress ("retget", REQUEST_TYPE_JUMP)))
	SetLabelAddress ("endget");
	// ACCURACY
	Insert (7, _MOV_R_N (_RCX, REAL_ACCURACY))
	Insert (3, _XCHG_R_R (_RBX, _RCX)) // Для вызова mymul
	Insert (5, _CALL (GetLabelAddress ("mymul", REQUEST_TYPE_CALL)))
	Insert (3, _XCHG_R_R (_RBX, _RCX)) // Для вызова mymul
	// ACCURACY
	Insert (5, _JMP (GetLabelAddress ("retget", REQUEST_TYPE_JUMP)))
	SetLabelAddress ("endget2");
	SetLabelAddress ("repget3");
	Insert (3, _XCHG_R_R (_RBX, _RDI)) // Для вызова mymul
	Insert (5, _CALL (GetLabelAddress ("mymul", REQUEST_TYPE_CALL))) // Результат *= 10
	Insert (3, _XCHG_R_R (_RBX, _RDI)) // Для вызова mymul
	Insert (3, _DEC_R (_RSI))
	Insert (6, _JNZ (GetLabelAddress ("repget3", REQUEST_TYPE_J_COND)))
	SetLabelAddress ("retget");
	Insert (4, _CMP_PROR_N (_R8, 1)) // Если число отрицательное
	Insert (6, _JNE (GetLabelAddress ("retget2", REQUEST_TYPE_J_COND)))
	Insert (3, _NEG_R (_RAX))
	SetLabelAddress ("retget2");
	Insert (1, _RET)

	/*   POW    */
	SetLabelAddress ("pow");
	Insert (1, _PUSH_R (_RBP))
	Insert (3, _MOV_R_R (_RBP, _RSP))
	Insert (7, _TEST_R_N (_RCX, 1)) // Смотрим на чётность RCX
	Insert (6, _JNZ (GetLabelAddress ("powstart", REQUEST_TYPE_J_COND)))
	Insert (7, _ADD_R_N (_RAX, 0))
	Insert (6, _JNS (GetLabelAddress ("powstart", REQUEST_TYPE_J_COND)))
	Insert (3, _NEG_R (_RAX)) // Если степень чётная, то RAX *= -1
	SetLabelAddress ("powstart");
	Insert (3, _MOV_R_R (_RBX, _RAX)) // RBX <- множитель
	// ACCURACY
	Insert (7, _MOV_R_N (_RDI, REAL_ACCURACY))
	Insert (3, _XOR_R_R (_RDX, _RDX)) // Иначе делимое будет DX_AX
	Insert (3, _XCHG_R_R (_RBX, _RDI)) // Для вызова mymul
	Insert (5, _CALL (GetLabelAddress ("mydiv", REQUEST_TYPE_CALL)))
	Insert (3, _XCHG_R_R (_RBX, _RDI)) // Для вызова mymul
	Insert (3, _XCHG_R_R (_RAX, _RBX)) // Теперь RBX = RBX / ACCURACY
	// ACCURACY
	Insert (3, _SUB_R_R (_RCX, _RDI))
	SetLabelAddress ("reppow");
	Insert (5, _CALL (GetLabelAddress ("mymul", REQUEST_TYPE_CALL))) // Умножаем на множитель
	Insert (3, _SUB_R_R (_RCX, _RDI)) // По сути тут декремент RCX, поскольку RCX умножена на REAL_ACCURACY
	Insert (6, _JNZ (GetLabelAddress ("reppow", REQUEST_TYPE_J_COND)))
	Insert (1, _POP_R (_RBP))
	Insert (1, _RET)

	/*  SQRT    */
	SetLabelAddress ("sqrt");
	Insert (1, _PUSH_R (_RBP))
	Insert (3, _MOV_R_R (_RBP, _RSP))
	Insert (7, _MOV_R_M (_RDX, GetLabelAddress ("num", REQUEST_TYPE_MOV))) // Адрес результата в памяти
	Insert (4, _MOV_MR_N_R (_RDX, 0, _RAX)) // num = RAX
	Insert (3, _FINIT)
	Insert (2, _FILD_MR (_RDX)) // Регистр FPU = num
	Insert (2, _FSQRT)
	Insert (2, _FISTP_MR (_RDX)) // num = регистр FPU
	Insert (4, _MOV_R_MR_N (_RAX, _RDX, 0)) // RAX = num
	// ACCURACY
	Insert (7, _MOV_R_N (_RDI, SQRT_REAL_ACCURACY))
	Insert (3, _XCHG_R_R (_RBX, _RDI)) // Для вызова mymul
	Insert (5, _CALL (GetLabelAddress ("mymul", REQUEST_TYPE_CALL)))
	Insert (3, _XCHG_R_R (_RBX, _RDI)) // Для вызова mymul
	// ACCURACY
	Insert (1, _POP_R (_RBP))
	Insert (1, _RET)
	/*  СЛУЖЕБНЫЕ ДАННЫЕ */
	SetLabelAddress ("service_pos");
	code [code_idx++] = '0';
	code [code_idx++] = ',';
	SetLabelAddress ("service_pos_len");
	code [code_idx++] = 2;
	SetLabelAddress ("service_neg");
	code [code_idx++] = '-';
	code [code_idx++] = '0';
	code [code_idx++] = ',';
	SetLabelAddress ("service_neg_len");
	code [code_idx++] = 3;
	SetLabelAddress ("output");
	code_idx += 16;
	SetLabelAddress ("input");
	code_idx += 16;
	SetLabelAddress ("num");
	code_idx += 16;
}

void ELF::AddGlobals () {
	for (size_t i = 0; i < var_idx; ++i) {
		if ((int) vars[i].val == GLOBAL) {
			char g_num [ARRAY_SIZE];
			sprintf (g_num, "g%zu", elf_vars[i].shift);
			SetLabelAddress (g_num);
			code_idx += 16;
		}
	}
}

void ELF::HandleLabels () {
	for (int i = 0; i < label_reqs_idx; ++i) {
		for (int j = 0; j < labels_idx; ++j) {
			if (strcmp (label_reqs[i].name, labels[j].name) == 0) {
				if (label_reqs[i].type == REQUEST_TYPE_CALL) {
					code [label_reqs[i].code_idx_request + REQUEST_TYPE_CALL] = \
					(((labels[j].code_ptr - label_reqs[i].code_idx_request + 0xFFFFFFFF - \
					_CALL_LEN) % 0xFFFFFFFF) & 0x000000FF);
					code [label_reqs[i].code_idx_request + REQUEST_TYPE_CALL + 1] = \
					(((labels[j].code_ptr - label_reqs[i].code_idx_request + 0xFFFFFFFF - \
					_CALL_LEN) % 0xFFFFFFFF) & 0x0000FF00) >> 8;
					code [label_reqs[i].code_idx_request + REQUEST_TYPE_CALL + 2] = \
					(((labels[j].code_ptr - label_reqs[i].code_idx_request + 0xFFFFFFFF - \
					_CALL_LEN) % 0xFFFFFFFF) & 0x00FF0000) >> 16;
					code [label_reqs[i].code_idx_request + REQUEST_TYPE_CALL + 3] = \
					(((labels[j].code_ptr - label_reqs[i].code_idx_request + 0xFFFFFFFF - \
					_CALL_LEN) % 0xFFFFFFFF) & 0xFF000000) >> 24;
					break;
				}
				else if (label_reqs[i].type == REQUEST_TYPE_JUMP) {
					code [label_reqs[i].code_idx_request + REQUEST_TYPE_JUMP] = \
					(((labels[j].code_ptr - label_reqs[i].code_idx_request + 0xFFFFFFFF - \
					_JUMP_LEN) % 0xFFFFFFFF) & 0x000000FF);
					code [label_reqs[i].code_idx_request + REQUEST_TYPE_JUMP + 1] = \
					(((labels[j].code_ptr - label_reqs[i].code_idx_request + 0xFFFFFFFF - \
					_JUMP_LEN) % 0xFFFFFFFF) & 0x0000FF00) >> 8;
					code [label_reqs[i].code_idx_request + REQUEST_TYPE_JUMP + 2] = \
					(((labels[j].code_ptr - label_reqs[i].code_idx_request + 0xFFFFFFFF - \
					_JUMP_LEN) % 0xFFFFFFFF) & 0x00FF0000) >> 16;
					code [label_reqs[i].code_idx_request + REQUEST_TYPE_JUMP + 3] = \
					(((labels[j].code_ptr - label_reqs[i].code_idx_request + 0xFFFFFFFF - \
					_JUMP_LEN) % 0xFFFFFFFF) & 0xFF000000) >> 24;
					break;
				}
				else if (label_reqs[i].type == REQUEST_TYPE_J_COND) {
					code [label_reqs[i].code_idx_request + REQUEST_TYPE_J_COND] = \
					(((labels[j].code_ptr - label_reqs[i].code_idx_request + 0xFFFFFFFF - \
					_J_COND_LEN) % 0xFFFFFFFF) & 0x000000FF);
					code [label_reqs[i].code_idx_request + REQUEST_TYPE_J_COND + 1] = \
					(((labels[j].code_ptr - label_reqs[i].code_idx_request + 0xFFFFFFFF - \
					_J_COND_LEN) % 0xFFFFFFFF) & 0x0000FF00) >> 8;
					code [label_reqs[i].code_idx_request + REQUEST_TYPE_J_COND + 2] = \
					(((labels[j].code_ptr - label_reqs[i].code_idx_request + 0xFFFFFFFF - \
					_J_COND_LEN) % 0xFFFFFFFF) & 0x00FF0000) >> 16;
					code [label_reqs[i].code_idx_request + REQUEST_TYPE_J_COND + 3] = \
					(((labels[j].code_ptr - label_reqs[i].code_idx_request + 0xFFFFFFFF - \
					_J_COND_LEN) % 0xFFFFFFFF) & 0xFF000000) >> 24;
					break;
				}
				else if (label_reqs[i].type == REQUEST_TYPE_MOV) {
					code [label_reqs[i].code_idx_request + REQUEST_TYPE_MOV] = \
					((labels[j].code_ptr + START_ADDRESS) & 0x000000FF);
					code [label_reqs[i].code_idx_request + REQUEST_TYPE_MOV + 1] = \
					((labels[j].code_ptr + START_ADDRESS) & 0x0000FF00) >> 8;
					code [label_reqs[i].code_idx_request + REQUEST_TYPE_MOV + 2] = \
					((labels[j].code_ptr + START_ADDRESS) & 0x00FF0000) >> 16;
					code [label_reqs[i].code_idx_request + REQUEST_TYPE_MOV + 3] = \
					((labels[j].code_ptr + START_ADDRESS) & 0xFF000000) >> 24;
					break;
				}
				else if (label_reqs[i].type == REQUEST_TYPE_GLOBAL_MOV) {
					code [label_reqs[i].code_idx_request + REQUEST_TYPE_GLOBAL_MOV] = \
					((labels[j].code_ptr + START_ADDRESS) & 0x000000FF);
					code [label_reqs[i].code_idx_request + REQUEST_TYPE_GLOBAL_MOV + 1] = \
					((labels[j].code_ptr + START_ADDRESS) & 0x0000FF00) >> 8;
					code [label_reqs[i].code_idx_request + REQUEST_TYPE_GLOBAL_MOV + 2] = \
					((labels[j].code_ptr + START_ADDRESS) & 0x00FF0000) >> 16;
					code [label_reqs[i].code_idx_request + REQUEST_TYPE_GLOBAL_MOV + 3] = \
					((labels[j].code_ptr + START_ADDRESS) & 0xFF000000) >> 24;
					break;
				}
				else {
					printf ("Label \"%s\ntype undefined", label_reqs[i].name);
					break;
				}
			}
			if (j == labels_idx - 1)
				printf ("not found label %s\n", label_reqs[i].name);
		}
	}
}