#include "Backend_Real.h"

//TODO Посмотреть что там с умножением двух отриц чисел в функции степени
//TODO Дробная степень не 0.5
//TODO IF без ELSE

size_t NUM_IF = 1;
size_t NUM_POW = 1;
size_t locals_num = 0;

ASM_VAR_t asm_vars [ARRAY_SIZE];
int temp [ARRAY_SIZE];

void ASM::GetVarIdx () {
	//Временно запомним число переменных в каждой функции
	for (size_t i = 0; i < func_idx; ++i) {
		temp[i] = funcs[i].val;
		funcs[i].val = 0;
	}
	size_t glob_idx = 1;
	for (size_t i = 0; i < var_idx; ++i) {
		if ((int) vars[i].val == GLOBAL) {
			asm_vars[i].is_global = true;
			asm_vars[i].shift = glob_idx++;
		}
		else if (funcs[(int)vars[i].val].val < temp[(int)vars[i].val]) {
			asm_vars[i].is_global = false;
			asm_vars[i].shift = 8 * (++ funcs[(int) vars[i].val].val + 1); //ещё +1 из-за того, что сначала лежит адрес возврата
			if (funcs[(int) vars[i].val].val == temp[(int)vars[i].val])
				temp[(int)vars[i].val] = 0;
		}
		else {
			asm_vars[i].is_global = false;
			asm_vars[i].shift = 8 * (-- temp[(int)vars[i].val]);
		}
	}
	//После выполнения цикла все переменные получат свои сдвиги и количества аргументов у функций вернутся к начальному значению
	//А в массиве temp будут лежать количества локальных переменных (разве что, со знаком минус)
}

void ASM::TreeToASM (Node *node) {
	//TODO УБЕРИ ЭТО ПОЖАЛУЙСТА
	//FILE *writefile = fopen ("../my_ast/temp.asm", "w");
	FILE *writefile = fopen ("../my_programs/test/main.s", "w");
	//TODO УБЕРИ ЭТО ПОЖАЛУЙСТА
	ASM::GetVarIdx ();
	Node *base = node;
	while (node->left && node->left->type != TYPE_FUNC) {
		node = node->right;
	}
	//Вход в программу
	fprintf (writefile, "section .text\n"
	                    "global _start\n"
	                    "_start: ; entry point\n");
	fprintf (writefile, "call main ; start main function\n");
	//Выход из программы
	fprintf (writefile, "mov rax, 60 ; rax = exit\n"
	                    "mov rdi, 0 ; rdi = 0 code\n"
	                    "syscall ; program exit\n"
					    "ret\n");
	ASM::NodeToASM (writefile, node);
	ASM::ExtraFuncs (writefile);
	ASM::AddGlobals (writefile);
	fclose (writefile);
}

void ASM::NodeToASM (FILE *writefile, Node *node) {
	if (node->type == TYPE_NUM) {
		fprintf (writefile, "push %dd ; push number\n", (int) (REAL_ACCURACY * node->data));
	}
	else if (node->type == TYPE_VAR) {
		if (asm_vars[(int)node->data].is_global)
			fprintf (writefile, "push qword [g%zu] ; push global variable\n", asm_vars[(int)node->data].shift);
		else
			fprintf (writefile, "push qword [rbp%+d] ; push local variable\n", asm_vars[(int)node->data].shift);
	}
	else if (node->type == TYPE_OP) {
		ASM::NodeToASM (writefile, node->left);
		ASM::NodeToASM (writefile, node->right);
		switch ((int)node->data) {
			case OP_SUM: {
				fprintf (writefile, "pop rcx ; sum_start\npop rbx\nadd rbx, rcx\npush rbx ; sum_end\n");
				break;
			}
			case OP_SUB: {
				fprintf (writefile, "pop rcx ; sub_start\npop rbx\nsub rbx, rcx\npush rbx ; sub_end\n");
				break;
			}
			case OP_MUL: {
				fprintf (writefile, "pop rbx ; mul_start\npop rax\n"
				//ACCURACY
				"xor rdx, rdx ; clean rdx\nmov rcx, %dd\nxchg rax, rbx\nxchg rbx, rcx\ncall mydiv\nxchg rbx, rcx\nxchg rax, rbx\n"
	            //ACCURACY
				"call mymul\npush rax ; mul_end\n", REAL_ACCURACY);
				break;
			}
			case OP_DIV: {
				fprintf (writefile, "pop rbx ; div_start\npop rax\n"
				//ACCURACY
				"xor rdx, rdx ; clean rdx\nmov rcx, %dd\nxchg rax, rbx\nxchg rbx, rcx\ncall mydiv\nxchg rbx, rcx\nxchg rax, rbx\n"
				//ACCURACY
				"call mydiv\npush rax ; div_end\n", REAL_ACCURACY);
				break;
			}
			case OP_POW: {
				size_t num = NUM_POW++;
				fprintf (writefile, "; POWER FUNCTION\n"
				"pop rcx\n"
				"pop rax\n"
				"cmp rcx, %lgd ; compare if sqrt or not\n"
				"je sqrt_%zu\n"
				"call pow ; just power\n"
				"jmp endpow_%zu\n"
				"sqrt_%zu:\n"
				"call sqrt ; just sqrt\n"
				"endpow_%zu:\n"
	            "push rax\n"
				"; POWER FUNCTION\n", 0.5 * REAL_ACCURACY, num, num, num, num);
				break;
			}
			case OP_ABOVE: {
				fprintf (writefile, "pop rax ; jmp start\npop rbx\ncmp rax, rbx\nja ");
				break;
			}
			case OP_ABOVE_EQUAL: {
				fprintf (writefile, "pop rax ; jmp start\npop rbx\ncmp rax, rbx\njae ");
				break;
			}
			case OP_BELOW: {
				fprintf (writefile, "pop rax ; jmp start\npop rbx\ncmp rax, rbx\njb ");
				break;
			}
			case OP_BELOW_EQUAL: {
				fprintf (writefile, "pop rax ; jmp start\npop rbx\ncmp rax, rbx\njbe ");
				break;
			}
			case OP_EQUAL: {
				fprintf (writefile, "pop rax ; jmp start\npop rbx\ncmp rax, rbx\njne ");
				break;
			}
			case OP_UNEQUAL: {
				fprintf (writefile, "pop rax ; jmp start\npop rbx\ncmp rax, rbx\nje ");
				break;
			}
		}
	}
	else if (node->type == TYPE_FUNC) {
		if (node->parent->type == TYPE_SYS && node->parent->data == SEMICOLON) { //Текст функции
			fprintf (writefile, "\n%s:\npush rbp ; typical function entry\nmov rbp, rsp\n", funcs[(int) node->data].name);
			locals_num =  -1 * temp[(int) node->data]; //Количество локальных переменных
			fprintf (writefile, "sub rsp, %dd ; space for locals\n", 8 * locals_num); //Выделяем место под локальные переменные
			ASM::NodeToASM (writefile, node->right);
		}
		else { //Вызов функции
			Node *base = node;
			while (node->left && node->left->type == TYPE_SYS && node->left->data == COMMA)
				node = node->left;
			while (node != base) { //Передача параметров в обратном порядке
				ASM::NodeToASM (writefile, node->right);
				node = node->parent;
			}
			fprintf (writefile, "call %s\n", funcs[(int) node->data].name);
			fprintf (writefile, "add rsp, %lgd ; clean space for arguments\n", 8 * funcs[(int) node->data].val); //подчищаем стек (назад на количество переданных аргументов)
			fprintf (writefile, "push qword rax ; push the value just returned\n"); //return value
		}
	}
	else if (node->type == TYPE_SYS) {
		switch ((int)node->data) {
			case EQUAL: {
				ASM::NodeToASM (writefile, node->right);
				if (asm_vars[(int)node->left->data].is_global)
					fprintf (writefile, "pop qword [g%zu] ; equal\n", asm_vars[(int)node->left->data].shift);
				else
					fprintf (writefile, "pop qword [rbp%+d] ; equal\n", asm_vars[(int)node->left->data].shift);
				break;
			}
			case RET: {
				if (node->left->type == TYPE_NUM)
					fprintf (writefile, "mov rax, %dd ; return value in rax\n", (int) (REAL_ACCURACY * node->left->data));
				else if (node->left->type == TYPE_VAR) {
					if (asm_vars[(int)node->left->data].is_global)
						fprintf (writefile, "mov rax, qword [g%zu] ; return value in rax\n", asm_vars[(int)node->left->data].shift);
					else
						fprintf (writefile, "mov rax, qword [rbp%+d] ; return value in rax\n", asm_vars[(int)node->left->data].shift);
				}
				else {
					ASM::NodeToASM (writefile, node->left);
					if (node->left)
						fprintf (writefile, "pop rax ; return value in rax\n");
				}
				fprintf (writefile, "add rsp, %dd ; clean space for locals\n", 8 * locals_num); //Подчищаем место, выделенное под локальные переменные
				fprintf (writefile, "pop rbp ; typical function exit\nret\n");
				break;
			}
			case PUT: {
				if (node->left->type == TYPE_NUM)
					fprintf (writefile, "push %dd ; put_start\npop rax\n", (int) (REAL_ACCURACY * node->left->data));
				else if (node->left->type == TYPE_VAR) {
					if (asm_vars[(int)node->left->data].is_global)
						fprintf (writefile, "mov rax, [g%zu]\n", asm_vars[(int)node->left->data].shift);
					else
						fprintf (writefile, "mov rax, [rbp%+d]\n", asm_vars[(int)node->left->data].shift);
				}
				else {
					ASM::NodeToASM (writefile, node->left);
					fprintf (writefile, "pop rax ; expression is passed\n");
				}
				fprintf (writefile, "call put ; put_end\n");
				break;
			}
			case GET: {
				if (asm_vars[(int)node->left->data].is_global)
					fprintf (writefile, "call get ; get_start\nmov [g%zu], rax ; get_end\n", asm_vars[(int)node->left->data].shift);
				else
					fprintf (writefile, "call get ; get_start\nmov [rbp%+d], rax ; get_end\n", asm_vars[(int)node->left->data].shift);
				break;
			}
			case IF: {
				size_t num = NUM_IF++;
				ASM::NodeToASM (writefile, node->left);
				fprintf (writefile, "elseif_%d\n", num);
				ASM::NodeToASM (writefile, node->right->left);
				fprintf (writefile, "jmp endif_%d\n", num);
				fprintf (writefile, "elseif_%d:\n", num);
				ASM::NodeToASM (writefile, node->right->right);
				fprintf (writefile, "endif_%d:\n", num);
				break;
			}
			case SEMICOLON: {
				ASM::NodeToASM (writefile, node->left);
				if (node->right)
					ASM::NodeToASM (writefile, node->right);
				break;
			}
			case OP: {
				ASM::NodeToASM (writefile, node->left);
				if (node->right)
					ASM::NodeToASM (writefile, node->right);
				break;
			}
		}
	}
}

void ASM::ExtraFuncs (FILE *writefile) {
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
	                    "div rbx\n"
	                    "cmp r8, 0d\n"
	                    "je enddiv\n"
	                    "neg rax\n"
	                    "enddiv:\n"
	                    "pop r8 ; return saved r8\n"
	                    "ret\n");
	fprintf (writefile, "\n"
			            "put:\n"
	                    "xor r8, r8 ; r8 <- extra char '-' if number is negative\n"
	                    "add rax, 0d ; check if rax is negative\n"
	                    "jns startput ; not negative -> startput\n"
	                    "neg rax ; rax *= -1\n"
	                    "mov rcx, output ; string offset\n"
	                    "mov byte [rcx], 02dh ; '-' char\n"
	                    "inc r8 ; because the nuber is negative\n"
	                    "startput:\n"
	                    "mov rdi, 10d ; rdi = 10 <- decimal\n"
					    "xor rsi, rsi\n"
	                    "repput:\n"
	                    "xor rdx, rdx ; or divident will be dx_ax\n"
	                    "idiv rdi ; rax /= 10, rdx = rax mod 10\n"
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
	                    "mov bl, 02ch ; point char\n"
	                    "mov byte [rcx], bl\n"
	                    "inc rcx ; next char\n"
	                    "jmp repput2\n"
	                    "endput:\n"
	                    "; PUT FUNCTION\n"
	                    "mov rax, 4 ; sys_write\n"
	                    "mov rbx, 1 ; file descriptor = stdout\n"
	                    "mov rcx, output ; string offset\n"
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
	                    "imul rdi ; result *= 10\n"
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
	                    "imul rdi ; result *= 10\n"
	                    "add rax, rbx ; result += new_char\n"
	                    "inc rcx ; next char\n"
	                    "dec rsi ; --counter\n"
	                    "jnz repget2 ; continue cycle\n"
	                    "jmp retget\n"
	                    "endget:\n"
	                    "; ACCURACY\n"
	                    "mov rcx, %dd\n"
	                    "imul rcx\n"
	                    "; ACCURACY\n"
	                    "jmp retget\n"
	                    "endget2:\n"
	                    "repget3:\n"
	                    "imul rdi\n"
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
					    "xor rdx, rdx ; clean rdx\n"
	                    "idiv rdi\n"
	                    "xchg rax, rbx ; rbx = rbx / ACCURACY\n"
	                    "; ACCURACY\n"
	                    "sub rcx, rdi\n"
	                    "reppow:\n"
	                    "imul rbx ; multiply by a factor\n"
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
	                    "imul rdi\n"
	                    "; ACCURACY\n"
	                    "pop rbp\n"
	                    "ret\n", REAL_ACCURACY, SQRT_REAL_ACCURACY);
	fprintf (writefile, "\n"
	                    "section .data\n"
	                    "; FOR PUT\n"
	                    "output dq 1\n"
	                    "; FOR PUT\n"
					    "; FOR GET\n"
	                    "input dq 1\n"
	                    "; FOR GET\n"
	                    "; FOR SQRT OPERATION\n"
	                    "num dq 1\n"
	                    "; FOR SQRT OPERATION\n");
}

void ASM::AddGlobals (FILE *writefile) {
	for (size_t i = 0; i < var_idx; ++i) {
		if ((int) vars[i].val == GLOBAL) {
			fprintf (writefile, "g%d dq 1\n", asm_vars[i].shift);
		}
	}
}