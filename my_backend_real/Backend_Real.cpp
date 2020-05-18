#include "Backend_Real.h"

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
	for (size_t i = 0; i < var_idx; ++i) {
		if ((int)vars[i].val == GLOBAL)
			printf ("globals available soon\n");
		else if (funcs[(int)vars[i].val].val < temp[(int)vars[i].val]) {
			asm_vars[i].shift = 8 * (++ funcs[(int) vars[i].val].val + 1); //ещё +1 из-за того, что сначала лежит адрес возврата
			if (funcs[(int) vars[i].val].val == temp[(int)vars[i].val])
				temp[(int)vars[i].val] = 0;
		}
		else {
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
		// TODO походу тут какая то муть в глобальными
		// ASM::NodeToASM (writefile, node->left);
		node = node->right;
	}
	//TODO что нибудь с глобальными
	//Вход в программу
	fprintf (writefile, "section .text\n"
	                    "global _start\n"
	                    "_start:\n");
	fprintf (writefile, "call main\n");
	//Выход из программы
	fprintf (writefile, "mov eax, 60\n"
	                    "mov rdi, 0\n"
	                    "syscall\n"
					    "ret\n");
	ASM::NodeToASM (writefile, node);
	ASM::ExtraFuncs (writefile);
	fclose (writefile);
}

void ASM::NodeToASM (FILE *writefile, Node *node) {
	if (node->type == TYPE_NUM) {
		fprintf (writefile, "push %lgd\n", REAL_ACCURACY * node->data);
	}
	else if (node->type == TYPE_VAR) {
		if (asm_vars[(int)node->data].is_global)
			fprintf (writefile, "push global!!(%d)\n", asm_vars[(int)node->data].shift);
		else
			fprintf (writefile, "push qword [rbp%+d]\n", asm_vars[(int)node->data].shift);
	}
	else if (node->type == TYPE_OP) {
		ASM::NodeToASM (writefile, node->left);
		ASM::NodeToASM (writefile, node->right);
		switch ((int)node->data) {
			case OP_SUM: {
				fprintf (writefile, "pop rcx\npop rbx\nadd rbx, rcx\npush rbx\n");
				break;
			}
			case OP_SUB: {
				fprintf (writefile, "pop rcx\npop rbx\nsub rbx, rcx\npush rbx\n");
				break;
			}
			case OP_MUL: {
				fprintf (writefile, "pop rbx\npop rax\n"
						//ACCURACY
						"mov rcx, %dd\nxchg rax, rbx\ndiv rcx\nxchg rax, rbx\n"
	                    //ACCURACY
						"mul rbx\npush rax\n", REAL_ACCURACY);
				break;
			}
			case OP_DIV: {
				fprintf (writefile, "pop rbx\npop rax\n"
				//ACCURACY
				"mov rcx, %dd\nxchg rax, rbx\ndiv rcx\nxchg rax, rbx\n"
				//ACCURACY
				"div rbx\npush rax\n", REAL_ACCURACY);
				break;
			}
			case OP_POW: { //TODO Разобрать случай когда степень дробная, но не 0.5
				size_t num = NUM_POW++;
				fprintf (writefile, "; POWER FUNCTION\n"
						            "pop rcx\n"
				                    "pop rax\n"
				                    "cmp rcx, %lgd\n"
				                    "je sqrt_%zu\n"
				                    "call pow\n"
				                    "jmp endpow_%zu\n"
				                    "sqrt_%zu:\n"
				                    "call sqrt\n"
				                    "endpow_%zu:\n"
				                    "push rax\n"
				                    "; POWER FUNCTION\n", 0.5 * REAL_ACCURACY, num, num, num, num);
				break;
			}
			case OP_ABOVE: {
				fprintf (writefile, "pop rax\npop rbx\ncmp rax, rbx\nja ");
				break;
			}
			case OP_ABOVE_EQUAL: {
				fprintf (writefile, "pop rax\npop rbx\ncmp rax, rbx\njae ");
				break;
			}
			case OP_BELOW: {
				fprintf (writefile, "pop rax\npop rbx\ncmp rax, rbx\njb ");
				break;
			}
			case OP_BELOW_EQUAL: {
				fprintf (writefile, "pop rax\npop rbx\ncmp rax, rbx\njbe ");
				break;
			}
			case OP_EQUAL: {
				fprintf (writefile, "pop rax\npop rbx\ncmp rax, rbx\nje ");
				break;
			}
			case OP_UNEQUAL: {
				fprintf (writefile, "pop rax\npop rbx\ncmp rax, rbx\njne ");
				break;
			}
		}
	}
	else if (node->type == TYPE_FUNC) {
		if (node->parent->type == TYPE_SYS && node->parent->data == SEMICOLON) { //Текст функции
			fprintf (writefile, "\n%s:\npush rbp\nmov rbp, rsp\n", funcs[(int) node->data].name);
			locals_num =  -1 * temp[(int) node->data]; //Количество локальных переменных
			fprintf (writefile, "sub rsp, %dd\n", 8 * locals_num); //Выделяем место под локальные переменные
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
			fprintf (writefile, "add rsp, %lgd\n", 8 * funcs[(int) node->data].val); //подчищаем стек (назад на количество переданных аргументов)
			fprintf (writefile, "push qword rax ; push the value just returned\n"); //return value
		}
	}
	else if (node->type == TYPE_SYS) {
		switch ((int)node->data) {
			case EQUAL: {
				ASM::NodeToASM (writefile, node->right);
				if (asm_vars[(int)node->left->data].is_global)
					fprintf (writefile, "pop global!!(%d)\n", asm_vars[(int)node->left->data].shift);
				else
					fprintf (writefile, "pop qword [rbp%+d]\n", asm_vars[(int)node->left->data].shift);
				break;
			}
			case RET: {
				if (node->left->type == TYPE_NUM)
					fprintf (writefile, "mov rax, %lgd ; return value in rax\n", node->left->data);
				else if (node->left->type == TYPE_VAR) {
					if (asm_vars[(int)node->left->data].is_global)
						fprintf (writefile, "mov rax global!!(%d) ; return value in rax\n", asm_vars[(int)node->left->data].shift);
					else
						fprintf (writefile, "mov rax, qword [rbp%+d] ; return value in rax\n", asm_vars[(int)node->left->data].shift);
				}
				else {
					ASM::NodeToASM (writefile, node->left);
					if (node->left)
						fprintf (writefile, "pop rax\n"); //TODO может всё таки push qword rax ???
				}
				fprintf (writefile, "add rsp, %dd\n", 8 * locals_num); //Подчищаем место, выделенное под локальные переменные
				fprintf (writefile, "pop rbp\nret\n");
				break;
			}
			case PUT: {
				if (node->left->type == TYPE_NUM)
					fprintf (writefile, "push %lg\n", node->left->data);
				else if (node->left->type == TYPE_VAR) {
					if (asm_vars[(int)node->left->data].is_global)
						fprintf (writefile, "push global(%d)\n", asm_vars[(int)node->left->data].shift);
					else
						fprintf (writefile, "push qword [rbp%+d]\n", asm_vars[(int)node->left->data].shift);
				}
				else
					ASM::NodeToASM (writefile, node->left);
				fprintf (writefile, "out\n");
				break;
			}
			case GET: {
				if (asm_vars[(int)node->left->data].is_global)
					fprintf (writefile, "in\npop global!!(%d)\n", asm_vars[(int)node->left->data].shift);
				else
					fprintf (writefile, "in\npop qword [rbp%+d]\n", asm_vars[(int)node->left->data].shift);
				break;
			}
			case IF: {
				size_t num = NUM_IF++;
				//TODO Рассмотреть случай без ELSE
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
			            "pow:\n"
	                    "push rbp\n"
	                    "mov rbp, rsp\n"
	                    "mov rbx, rax\n"
	                    "; ACCURACY\n"
	                    "mov rdi, %dd ; rdx = ACCURACY\n"
	                    "div rdi\n"
	                    "xchg rax, rbx\n"
	                    "; ACCURACY\n"
	                    "sub rcx, rdi\n"
	                    ".for:\n"
	                    "mul rbx\n"
	                    "sub rcx, rdi\n"
	                    "jnz .for\n"
	                    "pop rbp\n"
	                    "ret\n"
	                    "\n"
	                    "sqrt:\n"
	                    "push rbp\n"
	                    "mov rbp, rsp\n"
	                    "mov rdx, num\n"
	                    "mov qword [rdx], rax ; num = rax\n"
	                    "fild qword [rdx] ; fpu_reg = num\n"
	                    "fsqrt ; fpu_reg = sqrt (fpu_reg)\n"
	                    "fistp qword [rdx] ; num = fpu_reg\n"
	                    "mov rax, qword [rdx] ; rax = num\n"
	                    "; ACCURACY\n"
	                    "mov rdi, %dd\n"
	                    "mul rdi\n"
	                    "; ACCURACY\n"
	                    "pop rbp\n"
	                    "ret\n"
	                    "\n"
	                    "section .data\n"
	                    "; FOR SQRT OPERATION\n"
	                    "num db 8\n"
	                    "; FOR SQRT OPERATION", REAL_ACCURACY, SQRT_REAL_ACCURACY);
}