#include "Bintrans.h"
#include <elf.h>
#include <cstdarg>

static size_t NUM_IF = 1;
static size_t NUM_POW = 1;
static size_t locals_num = 0;

ELF_VAR_t elf_vars [ARRAY_SIZE];
static int temp [ARRAY_SIZE];

const int CODE_SIZE = 10000;
static char code [CODE_SIZE]; //Сначала пишем программу сюда, потом меняем размеры и адреса
static size_t code_idx = 0; //Индекс в массиве, который содержит программу

static LABEL_t labels [ARRAY_SIZE];
static size_t labels_idx = 0;

static LABEL_REQUEST_t label_reqs [ARRAY_SIZE];
static size_t label_reqs_idx = 0;

int ELF::GetLabelAddress (char name[ARRAY_SIZE], LABEL_REQUEST_TYPE type) {
	for (int i = 0; i < labels_idx; ++i) {
		if (strcmp (labels[i].name, name) == 0) {
			return labels[i].code_ptr - code_idx;
		}
	}
	strcpy (labels[labels_idx].name, name); //Работвем с массивом меток
	labels[labels_idx++].code_ptr = 0;
	strcpy (label_reqs[label_reqs_idx].name, name); //Работаем с массивом запросов (сохраняем искомое имя)
	label_reqs[label_reqs_idx].code_idx_request = code_idx; //Сохраняем место в коде, где был запрос на метку
	label_reqs[label_reqs_idx].type = type;
	label_reqs_idx++; //Увеличиваем количество запроосов
	return (0x90 << 24) | (0x90 << 16) | (0x90 << 8) | (0x90);
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

void ELF::ELF_Header (FILE *writefile) {
	Insert (4, 0x7F, 0x45, 0x4C, 0x46) //EI_MAG = ELF
	Insert (1, 0x02) //EI_CLASS = 64 BIT
	Insert (1, 0x01) //EI_DATA = DATA2LSB (Little-Endian)
	Insert (1, 0x01) //EI_VERSION = EV_CURRENT
	Insert (1, 0x00) //EI_OS/ABI = UNIX SYSTEM V ABI
	Insert (1, 0x00) //EI_OS/ABI VER = 0
	MultiInsert (7, 0x00) //EmptyStuff
	Insert (2, 0x02, 0x00) //E_TYPE = EXEC
	Insert (2, 0x3E, 0x00) //E_MACHINE = EM_X86_64 (AMD x86- 64 architecture)
	Insert (4, 0x01, 0x00, 0x00, 0x00) //E_VERSION = EV_CURRENT
	//Entry offset
	Insert (4, 0x80, 0x00, 0x40, 0x00)
	MultiInsert (4, 0x00) //Empty Stuff
	//Program header offset
	Insert (4, 0x40, 0x00, 0x00, 0x00)
	MultiInsert (4, 0x00)
	//Section header offset
	MultiInsert (8, 0x00)
	//Flags
	MultiInsert (4, 0x00)
	//ELF header size
	Insert (2, 0x40, 0x00)
	//Program header size
	Insert (2, 0x38, 0x00)
	//Program headers quantity (number)
	Insert (2, 0x01, 0x00)
	//Section header size
	Insert (2, 0x40, 0x00)
	//Section headers quantity (number)
	MultiInsert (2, 0x00)
	//Section headers index table
	MultiInsert (2, 0x00)
}

void ELF::Program_Header (FILE *writefile) {
	Insert (4, 0x01, 0x00, 0x00, 0x00) //P_TYPE = Loadable program segment
	Insert (4, 0x05, 0x00, 0x00, 0x00) //P_FLAGS = Segment is readable and executable
	MultiInsert (8, 0x00) //P_OFFSET
	//Virtual address
	Insert (4, 0x00, 0x00, 0x40, 0x00)
	MultiInsert (4, 0x00)
	//Physical address
	Insert (4, 0x00, 0x00, 0x40, 0x00)
	MultiInsert (4, 0x00)
	//code[64d + 32d] = code [40h + 20h]
	//File size
	MultiInsert (8, 0x00) //Заполним, когда будеи известен размер программы
	//Memory size
	MultiInsert (8, 0x00) //Заполним, когда будеи известен размер программы
	//code[64d + 48d] = code [40h + 30h]
	//Align
	Insert (4, 0x00, 0x00, 0x20, 0x00)
	MultiInsert (4, 0x00)
	MultiInsert (8, 0x00) //Empty Stuff
}

void ELF::CreateELF (Node *node) {
	FILE *writefile = fopen ("../my_bintrans/test", "wb");
	if (!writefile) {
		printf ("Error opening ELF File\n");
		assert (writefile);
		return;
	}

	ELF::ELF_Header (writefile);
	ELF::Program_Header (writefile);
	ELF::TreeToELF (node, writefile);

	//Теперь заполним размер программы
	size_t sz = code_idx; //Размер программы
	//Теперь размер есть в sz, а значит в переменной code_idx он больше не нужен

	//File size
	code_idx = 0x40 + 0x20;
	code[code_idx++] = (sz & 0x000000FF);
	code[code_idx++] = (sz & 0x0000FF00) >> 8;
	code[code_idx++] = (sz & 0x00FF0000) >> 16;
	code[code_idx++] = (sz & 0xFF000000) >> 24;
	//Memory size
	code_idx = 0x40 + 0x28;
	code[code_idx++] = (sz & 0x000000FF);
	code[code_idx++] = (sz & 0x0000FF00) >> 8;
	code[code_idx++] = (sz & 0x00FF0000) >> 16;
	code[code_idx++] = (sz & 0xFF000000) >> 24;

	fwrite (code, 1, sz, writefile);
	fclose (writefile);
}

void ELF::GetVarIdx () {
	//Временно запомним число переменных в каждой функции
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
			elf_vars[i].shift = 8 * (++ funcs[(int) vars[i].val].val + 1); //ещё +1 из-за того, что сначала лежит адрес возврата
			if (funcs[(int) vars[i].val].val == temp[(int)vars[i].val])
				temp[(int)vars[i].val] = 0;
		}
		else {
			elf_vars[i].is_global = false;
			elf_vars[i].shift = 8 * (-- temp[(int)vars[i].val]);
		}
	}
	//После выполнения цикла все переменные получат свои сдвиги и количества аргументов у функций вернутся к начальному значению
	//А в массиве temp будут лежать количества локальных переменных (разве что, со знаком минус)
}

void ELF::TreeToELF (Node *node, FILE *writefile) {
	ELF::GetVarIdx ();
	Node *base = node;
	while (node->left && node->left->type != TYPE_FUNC) {
		node = node->right;
	}
	//Вход в программу
	code_idx += 21; //Выставляем адрес
	SetLabelAddress ("main"); //Функции main
	code_idx -= 21; //Возвращаемся назад
	Insert (5, _CALL (GetLabelAddress ("main", REQUEST_TYPE_CALL))) //Вызов main

	//Выход из программы
	Insert (7, _MOV_R_N (_RAX, 0x3C))
	Insert (7, _MOV_R_N (_RDI, 0))
	Insert (2, _SYSCALL)
	ELF::NodeToELF (writefile, node);
	//ELF::ExtraFuncs (writefile);
	//ELF::AddGlobals (writefile);
	ELF::HandleLabels ();
}

void ELF::NodeToELF (FILE *writefile, Node *node) {
	if (node->type == TYPE_NUM) {
		Insert (5, _PUSH_N ((int) (REAL_ACCURACY * node->data)))
	}
	else if (node->type == TYPE_VAR) {
		if (elf_vars[(int)node->data].is_global)
			fprintf (writefile, "push qword [g%zu] ; push global variable\n", elf_vars[(int)node->data].shift);
		else
			Insert (3, _PUSH_MR_N (_RBP, elf_vars[(int)node->data].shift)) //Кладём в стек глобальные переменные
	}
	else if (node->type == TYPE_OP) {
		ELF::NodeToELF (writefile, node->left);
		ELF::NodeToELF (writefile, node->right);
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
				Insert (1, _POP_R (_RBX)) //Начало умножения
				Insert (1, _POP_R (_RAX))
				Insert (3, _XOR_R_R (_RDX, _RDX)) //Иначе множитель будет DX_AX
				Insert (5, _CALL (GetLabelAddress ("mymul", REQUEST_TYPE_CALL)))
				//ACCURACY
				Insert (3, _XOR_R_R (_RDX, _RDX))
				Insert (7, _MOV_R_N (_RBX, REAL_ACCURACY))
				Insert (5, _CALL (GetLabelAddress ("mydiv", REQUEST_TYPE_CALL)))
				//ACCURACY
				Insert (1, _PUSH_R (_RAX)) //Конец умножения
				break;
			}
			case OP_DIV: {
				Insert (1, _POP_R (_RBX)) //Начало деления
				Insert (1, _POP_R (_RAX))
				//ACCURACY
				Insert (3, _XOR_R_R (_RDX, _RDX)) //Иначе делитель будет DX_AX
				Insert (3, _MOV_R_R (_RCX, _RBX))
				Insert (7, _MOV_R_N (_RBX, REAL_ACCURACY))
				Insert (5, _CALL (GetLabelAddress ("mymul", REQUEST_TYPE_CALL)))
				Insert (3, _MOV_R_R (_RBX, _RCX))
				//ACCURACY
				Insert (5, _CALL (GetLabelAddress ("mydiv", REQUEST_TYPE_CALL)))
				Insert (1, _PUSH_R (_RAX)) //Конец деления
				break;
			}
			case OP_POW: {
				char str_num1 [ARRAY_SIZE];
				char str_num2 [ARRAY_SIZE];
				sprintf (str_num1, "sqrt_%d", NUM_POW);
				sprintf (str_num1, "endpow_%d", NUM_POW);
				Insert (1, _POP_R (_RCX)) //Начало возведения в степень
				Insert (1, _POP_R (_RAX))
				Insert (7, _CMP_R_N (_RCX, (int) (0.5 * REAL_ACCURACY)))
				Insert (6, _JE (GetLabelAddress (str_num1, REQUEST_TYPE_J_COND)))
				Insert (5, _CALL (GetLabelAddress ("pow", REQUEST_TYPE_CALL)))
				Insert (5, _JMP (GetLabelAddress (str_num2, REQUEST_TYPE_JUMP)))
				SetLabelAddress (str_num1);
				Insert (5, _CALL (GetLabelAddress ("sqrt", REQUEST_TYPE_CALL)))
				SetLabelAddress (str_num2);
				Insert (1, _PUSH_R (_RAX)) //Конец возведения в степень
				++NUM_POW;
				break;
			}
			case OP_ABOVE: {
				char str_num [ARRAY_SIZE];
				sprintf (str_num, "elseif_%d", NUM_IF);
				Insert (1, _POP_R (_RAX)) //Начало прыжка
				Insert (1, _POP_R (_RBX))
				Insert (3, _CMP_R_R (_RAX, _RBX))
				Insert (6, _JGE (GetLabelAddress (str_num, REQUEST_TYPE_J_COND))) //Конец прыжка
				break;
			}
			case OP_ABOVE_EQUAL: {
				char str_num [ARRAY_SIZE];
				sprintf (str_num, "elseif_%d", NUM_IF);
				Insert (1, _POP_R (_RAX)) //Начало прыжка
				Insert (1, _POP_R (_RBX))
				Insert (3, _CMP_R_R (_RAX, _RBX))
				Insert (6, _JG (GetLabelAddress (str_num, REQUEST_TYPE_J_COND))) //Конец прыжка
				break;
			}
			case OP_BELOW: {
				char str_num [ARRAY_SIZE];
				sprintf (str_num, "elseif_%d", NUM_IF);
				Insert (1, _POP_R (_RAX)) //Начало прыжка
				Insert (1, _POP_R (_RBX))
				Insert (3, _CMP_R_R (_RAX, _RBX))
				Insert (6, _JLE (GetLabelAddress (str_num, REQUEST_TYPE_J_COND))) //Конец прыжка //Конец прыжка
				break;
			}
			case OP_BELOW_EQUAL: {
				char str_num [ARRAY_SIZE];
				sprintf (str_num, "elseif_%d", NUM_IF);
				Insert (1, _POP_R (_RAX)) //Начало прыжка
				Insert (1, _POP_R (_RBX))
				Insert (3, _CMP_R_R (_RAX, _RBX))
				Insert (6, _JL (GetLabelAddress (str_num, REQUEST_TYPE_J_COND))) //Конец прыжка
				break;
			}
			case OP_EQUAL: {
				char str_num [ARRAY_SIZE];
				sprintf (str_num, "elseif_%d", NUM_IF);
				Insert (1, _POP_R (_RAX)) //Начало прыжка
				Insert (1, _POP_R (_RBX))
				Insert (3, _CMP_R_R (_RAX, _RBX))
				Insert (6, _JNE (GetLabelAddress (str_num, REQUEST_TYPE_J_COND))) //Конец прыжка
				break;
			}
			case OP_UNEQUAL: {
				char str_num [ARRAY_SIZE];
				sprintf (str_num, "elseif_%d", NUM_IF);
				Insert (1, _POP_R (_RAX)) //Начало прыжка
				Insert (1, _POP_R (_RBX))
				Insert (3, _CMP_R_R (_RAX, _RBX))
				Insert (6, _JE (GetLabelAddress (str_num, REQUEST_TYPE_J_COND))) //Конец прыжка
				break;
			}
		}
	}
	else if (node->type == TYPE_FUNC) {
		if (node->parent->type == TYPE_SYS && node->parent->data == SEMICOLON) { //Текст функции
			SetLabelAddress (funcs[(int) node->data].name);
			Insert (1, _PUSH_R (_RBP)) //Стандартный вход в функцию
			Insert (3, _MOV_R_R (_RBP, _RSP))
			locals_num =  -1 * temp[(int) node->data]; //Количество локальных переменных * 8
			Insert (7, _SUB_R_N (_RSP, 8 * locals_num)) //Выделяем место под локальные переменные
			ELF::NodeToELF (writefile, node->right);
		}
		else { //Вызов функции
			Node *base = node;
			while (node->left && node->left->type == TYPE_SYS && node->left->data == COMMA)
				node = node->left;
			while (node != base) { //Передача параметров в обратном порядке
				ELF::NodeToELF (writefile, node->right);
				node = node->parent;
			}
			Insert (5, _CALL (GetLabelAddress (funcs[(int) node->data].name, REQUEST_TYPE_CALL)))
			Insert (7, _ADD_R_N (_RSP, (int) (8 * funcs[(int) node->data].val))) //Подчищаем стек на количество переданных аргументов
			Insert (1, _PUSH_R (_RAX)) //Return value
		}
	}
	else if (node->type == TYPE_SYS) {
		switch ((int)node->data) {
			case EQUAL: {
				ELF::NodeToELF (writefile, node->right);
				if (elf_vars[(int)node->left->data].is_global)
					fprintf (writefile, "pop qword [g%zu] ; equal\n", elf_vars[(int)node->left->data].shift);
				else
					Insert (3, _POP_MR_N (_RBP, elf_vars[(int)node->left->data].shift))
				break;
			}
			case RET: {
				if (node->left->type == TYPE_NUM)
				Insert (7, _MOV_R_N (_RAX, (int) (REAL_ACCURACY * node->left->data))) //Return value в RAX
				else if (node->left->type == TYPE_VAR) {
					if (elf_vars[(int)node->left->data].is_global)
						fprintf (writefile, "mov rax, qword [g%zu] ; return value in rax\n", elf_vars[(int)node->left->data].shift);
					else
						Insert (4, _MOV_R_MR_N (_RAX, _RBP, elf_vars[(int)node->left->data].shift))
				}
				else {
					ELF::NodeToELF (writefile, node->left);
					if (node->left) {
						Insert (1, _POP_R (_RAX)) //Return value в RAX
					}
				}
				Insert (7, _ADD_R_N (_RSP, 8 * locals_num)) //Подчищаем место, выделенное под локальные переменные
				Insert (1, _POP_R (_RBP))
				Insert (1, _RET) //Стандартный выход из функции
				break;
			}
			case PUT: {
				if (node->left->type == TYPE_NUM) {
					Insert (5, _PUSH_N ((int) (REAL_ACCURACY * node->left->data))) //Начало Put
					Insert (1, _POP_R (_RAX))
				}
				else if (node->left->type == TYPE_VAR) {
					if (elf_vars[(int)node->left->data].is_global)
						fprintf (writefile, "mov rax, [g%zu]\n", elf_vars[(int)node->left->data].shift);
					else {
						Insert (4, _MOV_R_MR_N (_RAX, _RBP, elf_vars[(int) node->left->data].shift))
					}
				}
				else {
					ELF::NodeToELF (writefile, node->left);
					Insert (1, _POP_R (_RAX)) //Выражение закончилось
				}
				Insert (5, _CALL (GetLabelAddress ("put", REQUEST_TYPE_CALL))) //Конец Put
				break;
			}
			case GET: {
				if (elf_vars[(int)node->left->data].is_global) {
					fprintf (writefile, "call get ; get_start\nmov [g%zu], rax ; get_end\n",
							elf_vars[(int) node->left->data].shift);
				}
				else {
					Insert (5, _CALL (GetLabelAddress ("get", REQUEST_TYPE_CALL))) //Начало Get
					Insert (4, _MOV_MR_N_R (_RBP, elf_vars[(int) node->left->data].shift, _RAX))
				}
				break;
			}
			case IF: {
				char str_num1 [ARRAY_SIZE];
				char str_num2 [ARRAY_SIZE];
				sprintf (str_num1, "elseif_%d", NUM_IF);
				sprintf (str_num2, "endif_%d", NUM_IF);
				ELF::NodeToELF (writefile, node->left);
				ELF::NodeToELF (writefile, node->right->left);
				Insert (5, _JMP (GetLabelAddress (str_num2, REQUEST_TYPE_JUMP)))
				SetLabelAddress (str_num1);
				ELF::NodeToELF (writefile, node->right->right);
				SetLabelAddress (str_num2);
				++NUM_IF;
				break;
			}
			case SEMICOLON: {
				ELF::NodeToELF (writefile, node->left);
				if (node->right)
					ELF::NodeToELF (writefile, node->right);
				break;
			}
			case OP: {
				ELF::NodeToELF (writefile, node->left);
				if (node->right)
					ELF::NodeToELF (writefile, node->right);
				break;
			}
		}
	}
}

void ELF::ExtraFuncs (FILE *writefile) {
	fprintf (writefile, "\n"
	                    "mymul:\n"
	                    "push r8 ; save r8\n"
	                    "xor r8, r8 ; number digit\n"
	                    "add rax, 0\n"
	                    "jns secondmul\n"
	                    "neg rax\n"
	                    "inc r8\n"
	                    "secondmul:\n"
	                    "add rbx, 0\n"
	                    "jns checkmul\n"
	                    "neg rbx\n"
	                    "dec r8\n"
	                    "checkmul:\n"
	                    "imul rbx\n"
	                    "cmp r8, 0d\n"
	                    "je endmul\n"
	                    "neg rax\n"
	                    "endmul:\n"
	                    "pop r8 ; return saved r8\n"
	                    "ret\n"
	                    "\n"
	                    "mydiv:\n"
	                    "push r8 ; save r8\n"
	                    "xor r8, r8 ; number digit\n"
	                    "add rax, 0\n"
	                    "jns seconddiv\n"
	                    "neg rax\n"
	                    "inc r8\n"
	                    "seconddiv:\n"
	                    "add rbx, 0\n"
	                    "jns checkdiv\n"
	                    "neg rbx\n"
	                    "dec r8\n"
	                    "checkdiv:\n"
	                    "idiv rbx\n"
	                    "cmp r8, 0d\n"
	                    "je enddiv\n"
	                    "neg rax\n"
	                    "enddiv:\n"
	                    "pop r8 ; return saved r8\n"
	                    "ret\n");
	fprintf (writefile, "\n"
	                    "put:\n"
	                    "xor r8, r8 ; r8 <- extra char '-' if number is negative\n"
	                    "xor r9, r9 ; flag if pointput was called\n"
	                    "add rax, 0d ; check if rax is negative\n"
	                    "jns startput ; not negative -> startput\n"
	                    "neg rax ; rax *= -1\n"
	                    "mov rcx, output ; string offset\n"
	                    "mov byte [rcx], 02dh ; '-' char\n"
	                    "inc r8 ; because the number is negative\n"
	                    "startput:\n"
	                    "mov rdi, 10d ; rdi = 10 <- decimal\n"
	                    "xor rsi, rsi ; rsi <- digits counter\n"
	                    "repput:\n"
	                    "xor rdx, rdx ; or divident will be dx_ax\n"
	                    "xchg rbx, rdi ; to call mydiv\n"
	                    "call mydiv ; rax /= 10, rdx = rax mod 10\n"
	                    "xchg rbx, rdi ; to call mydiv\n"
	                    "add rdx, '0' ; rdx -> ascii\n"
	                    "push rdx\n"
	                    "inc rsi ; ++digits counter\n"
	                    "cmp rax, 0 ; while rax != 0\n"
	                    "jne repput ; continue cycle\n"
	                    "mov rcx, output ; string offset\n"
	                    "mov rdx, rsi ; output length\n"
	                    "add rdx, 2 ; point char and \\n are printed too!\n"
	                    "add rdx, r8 ; +1 if the number is negative\n"
	                    "add rcx, r8 ; +1 if the number is negative\n"
	                    "repput2:\n"
	                    "pop rbx ; pop digit ascii\n"
	                    "mov byte [rcx], bl\n"
	                    "inc rcx ; next char\n"
	                    "dec rsi ; --digits counter\n"
	                    "cmp rsi, %dd ; it's point time\n"
	                    "je pointput\n"
	                    "cmp rsi, 0d\n"
	                    "jne repput2\n"
	                    "mov byte [rcx], 0ah ; \\n symbol \n"
	                    "jmp endput\n"
	                    "pointput:\n"
	                    "inc r9 ; pointput flag\n"
	                    "mov bl, 02ch ; point char\n"
	                    "mov byte [rcx], bl\n"
	                    "inc rcx ; next char\n"
	                    "jmp repput2\n"
	                    "endput:\n"
	                    "mov rcx, output ; string offset\n"
	                    "test r9, r9 ; test pointput flag\n"
	                    "jnz endput2\n"
	                    "push rdx ; save rdx\n"
	                    "push rcx ; save rcx\n"
	                    "xor rdx, rdx ; just clean rdx (no mul or div)\n"
	                    "mov rcx, service_pos; print '0,'\n"
	                    "mov rdi, service_pos_len\n"
	                    "mov dl, byte [rdi]\n"
	                    "test r8, r8 ; check if we need '-' char\n"
	                    "jz nextput ; if we don't\n"
	                    "pop rcx ; slightly change the value of rcx saved\n"
	                    "pop rdx ; slightly change the value of rdx saved\n"
	                    "dec rdx ;  in the main put we don't print \'-\'\n"
	                    "inc rcx ; starting from the number\n"
	                    "push rdx ; slightly change the value of rdx saved\n"
	                    "push rcx ; slightly change the value of rcx saved\n"
	                    "mov rcx, service_neg; print '-0,'\n"
	                    "mov rdi, service_neg_len\n"
	                    "mov dl, byte [rdi]\n"
	                    "nextput:\n"
	                    "mov rax, 4 ; sys_write\n"
	                    "mov rbx, 1 ; file descriptor = stdout\n"
	                    "int 0x80\n"
	                    "pop rcx ; return saved rcx\n"
	                    "pop rdx ; return saved rdx\n"
	                    "dec rdx ; in the main put we don't print point\n"
	                    "endput2:\n"
	                    "; PUT FUNCTION\n"
	                    "mov rax, 4 ; sys_write\n"
	                    "mov rbx, 1 ; file descriptor = stdout\n"
	                    "int 0x80\n"
	                    "; PUT FUNCTION\n"
	                    "ret\n", EXP_REAL_ACCURACY);
	fprintf (writefile, "\n"
	                    "get:\n"
	                    "; GET FUNCTION\n"
	                    "mov rax, 3 ; sys_read\n"
	                    "mov rbx, 1 ; file descriptor = stdin\n"
	                    "mov rcx, input ; string offset\n"
	                    "mov rdx, 16d ; input length\n"
	                    "int 0x80\n"
	                    "; GET FUNCTION\n"
	                    "xor r8, r8 ; r8 <- extra char '-' if number is negative\n"
	                    "xor rax, rax ; rax = 0 <- result\n"
	                    "mov rdi, 10d ; rdi = 10 <- decimal\n"
	                    "mov rcx, input ; string offset\n"
	                    "repget:\n"
	                    "mov bl, byte [rcx] ; bl = next char\n"
	                    "; sign\n"
	                    "cmp bl, 02dh ; 02dh <- '-'\n"
	                    "je signget\n"
	                    "; sign\n"
	                    "; end string\n"
	                    "cmp bl, 0ah ; 0ah <- end string char \n"
	                    "je endget\n"
	                    "; end string\n"
	                    "; point\n"
	                    "cmp bl, 02ch ; if (new_char != ',')\n"
	                    "je pointget\n"
	                    "cmp bl, 02eh ; if (new_char != ',')\n"
	                    "je pointget\n"
	                    "; point\n"
	                    "sub bl, '0' ; bl (ascii) -> bl (digit)\n"
	                    "xchg rbx, rdi ; to call mymul\n"
	                    "call mymul ; result *= 10\n"
	                    "xchg rbx, rdi ; to call mymul\n"
	                    "add rax, rbx ; result += new_char\n"
	                    "inc rcx ; next char\n"
	                    "jmp repget ; continue cycle\n"
	                    "signget:\n"
	                    "inc r8 ; number is negative\n"
	                    "inc rcx ; next char\n"
	                    "jmp repget\n"
	                    "pointget:\n"
	                    "mov rsi, %dd ; rsi = number of digits after the point\n"
	                    "inc rcx ; next char\n"
	                    "repget2:\n"
	                    "mov bl, byte [rcx] ; bl = next char\n"
	                    "; end string\n"
	                    "cmp bl, 0ah ; 0ah <- end string char \n"
	                    "je endget2\n"
	                    "; end string\n"
	                    "sub bl, '0' ; bl (ascii) -> bl (digit)\n"
	                    "xchg rbx, rdi ; to call mymul\n"
	                    "call mymul ; result *= 10\n"
	                    "xchg rbx, rdi ; to call mymul\n"
	                    "add rax, rbx ; result += new_char\n"
	                    "inc rcx ; next char\n"
	                    "dec rsi ; --counter\n"
	                    "jnz repget2 ; continue cycle\n"
	                    "jmp retget\n"
	                    "endget:\n"
	                    "; ACCURACY\n"
	                    "mov rcx, %dd\n"
	                    "xchg rbx, rcx ; to call mymul\n"
	                    "call mymul\n"
	                    "xchg rbx, rcx ; to call mymul\n"
	                    "; ACCURACY\n"
	                    "jmp retget\n"
	                    "endget2:\n"
	                    "repget3:\n"
	                    "xchg rbx, rdi ; to call mymul\n"
	                    "call mymul\n"
	                    "xchg rbx, rdi ; to call mymul\n"
	                    "dec rsi\n"
	                    "jnz repget3\n"
	                    "retget:\n"
	                    "cmp r8, 1d ; if the number is negative\n"
	                    "jne retget2\n"
	                    "neg rax ; rax *= -1\n"
	                    "retget2:\n"
	                    "ret\n", EXP_REAL_ACCURACY, REAL_ACCURACY);
	fprintf (writefile, "\n"
	                    "pow:\n"
	                    "push rbp\n"
	                    "mov rbp, rsp\n"
	                    "test rcx, 1d ; check if rcx is odd (mod 2 != 0) or even (mod 2 = 0)\n"
	                    "jnz powstart\n"
	                    "add rax, 0\n"
	                    "jns powstart\n"
	                    "neg rax ; if number is even (mod 2 = 0) rax *= -1\n"
	                    "powstart:\n"
	                    "mov rbx, rax ; rbx = factor\n"
	                    "; ACCURACY\n"
	                    "mov rdi, %dd ; rdi = ACCURACY\n"
	                    "xor rdx, rdx ; or divident will be dx_ax\n"
	                    "xchg rbx, rdi ; to call mydiv\n"
	                    "call mydiv\n"
	                    "xchg rbx, rdi ; to call mydiv\n"
	                    "xchg rax, rbx ; rbx = rbx / ACCURACY\n"
	                    "; ACCURACY\n"
	                    "sub rcx, rdi\n"
	                    "reppow:\n"
	                    "call mymul ; multiply by a factor\n"
	                    "sub rcx, rdi ; --rcx (because rcx is multiplied by REAL_ACCURACY)\n"
	                    "jnz reppow ; continue cycle\n"
	                    "pop rbp\n"
	                    "ret\n"
	                    "\n"
	                    "sqrt:\n"
	                    "push rbp\n"
	                    "mov rbp, rsp\n"
	                    "mov rdx, num ; rdx = memory address for sqrt (num)\n"
	                    "mov qword [rdx], rax ; num = rax\n"
	                    "finit\n"
	                    "fild qword [rdx] ; fpu_reg = num\n"
	                    "fsqrt ; fpu_reg = sqrt (fpu_reg)\n"
	                    "fistp qword [rdx] ; num = fpu_reg\n"
	                    "mov rax, qword [rdx] ; rax = num\n"
	                    "; ACCURACY\n"
	                    "mov rdi, %dd\n"
	                    "xchg rbx, rdi ; to call mymul\n"
	                    "call mymul\n"
	                    "xchg rbx, rdi ; to call mymul\n"
	                    "; ACCURACY\n"
	                    "pop rbp\n"
	                    "ret\n", REAL_ACCURACY, SQRT_REAL_ACCURACY);
	fprintf (writefile, "\n"
	                    "section .data\n"
	                    "; FOR PUT\n"
	                    "service_pos db \"0,\"\n"
	                    "service_pos_len db $ - service_pos\n"
	                    "service_neg db \"-0,\"\n"
	                    "service_neg_len db $ - service_neg\n"
	                    "output dq 1\n"
	                    "; FOR PUT\n"
	                    "; FOR GET\n"
	                    "input dq 1\n"
	                    "; FOR GET\n"
	                    "; FOR SQRT OPERATION\n"
	                    "num dq 1\n"
	                    "; FOR SQRT OPERATION\n");
}

void ELF::AddGlobals (FILE *writefile) {
	for (size_t i = 0; i < var_idx; ++i) {
		if ((int) vars[i].val == GLOBAL) {
			fprintf (writefile, "g%d dq 1\n", elf_vars[i].shift);
		}
	}
}

void ELF::HandleLabels () {
	for (int i = 0; i < label_reqs_idx; ++i) {
		for (int j = 0; j < labels_idx; ++j) {
			if (strcmp (label_reqs[i].name, labels[j].name) == 0) {
				printf ("label %s found on the idx %x and pasted into %x - %x bytes\n", label_reqs[i].name, labels[j].code_ptr, label_reqs[i].code_idx_request, label_reqs[i].code_idx_request + 3);
				code [label_reqs[i].code_idx_request + label_reqs[i].type] = \
				((labels[j].code_ptr - label_reqs[i].code_idx_request - \
				(label_reqs[i].type == REQUEST_TYPE_CALL ? _CALL_LEN : \
				label_reqs[i].type == REQUEST_TYPE_JUMP ? _JUMP_LEN : _J_COND_LEN)) & 0x000000FF);
				code [label_reqs[i].code_idx_request + label_reqs[i].type + 1] = \
				((labels[j].code_ptr - label_reqs[i].code_idx_request - \
				(label_reqs[i].type == REQUEST_TYPE_CALL ? _CALL_LEN : \
				label_reqs[i].type == REQUEST_TYPE_JUMP ? _JUMP_LEN : _J_COND_LEN)) & 0x0000FF00) >> 8;
				code [label_reqs[i].code_idx_request + label_reqs[i].type + 2] = \
				((labels[j].code_ptr - label_reqs[i].code_idx_request - \
				(label_reqs[i].type == REQUEST_TYPE_CALL ? _CALL_LEN : \
				label_reqs[i].type == REQUEST_TYPE_JUMP ? _JUMP_LEN : _J_COND_LEN)) & 0x00FF0000) >> 16;
				code [label_reqs[i].code_idx_request + label_reqs[i].type + 3] = \
				((labels[j].code_ptr - label_reqs[i].code_idx_request - \
				(label_reqs[i].type == REQUEST_TYPE_CALL ? _CALL_LEN : \
				label_reqs[i].type == REQUEST_TYPE_JUMP ? _JUMP_LEN : _J_COND_LEN)) & 0xFF000000) >> 24;
				break;
			}
			if (j == labels_idx - 1)
				printf ("not found label %s\n", label_reqs[i].name);
		}
		for (int k = label_reqs[i].code_idx_request; k <= label_reqs[i].code_idx_request + 4; ++k)
			printf ("%x ", code[k]);
		printf ("\n");
	}
}