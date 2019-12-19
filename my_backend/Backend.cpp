#include "Backend.h"

size_t NUM_IF = 1;
size_t global_vars = 0;
size_t main_vars = 0;

ASM_VAR_t asm_vars [ARRAY_SIZE];

void ASM::GetRAMIdx () {
    for (size_t i = 0; i < var_idx; ++i) {
        move = 0;
        asm_vars[i].is_global = false;
        if (vars[i].val == GLOBAL) {
            ++global_vars;
            asm_vars[i].is_global = true;
        } else if (vars[i].val == main_flag) {
            ++main_vars;
        }
        for (size_t j = 0; j < i; ++j) {
            if (vars[j].val == vars[i].val)
                ++move;
        }
        asm_vars[i].RAM_idx = move; //Сдвиг относительно bp в RAM
    }
}

void ASM::TreeToASM (Node *node) {
    FILE *writefile = fopen ("../my_ast/temp.asm", "w");
    ASM::GetRAMIdx ();
    Node *base = node;
    while (node->left && node->left->type != TYPE_FUNC) {
        ASM::NodeToASM (writefile, node->left);
        node = node->right;
    }
    if (base != node) {
        fprintf (writefile, "PUSH %d\nPOP BP\n", global_vars); //Пишем в асм глобальные переменные
    }
    //Сдвигаем bp и sp на размер глобальных переменных
    //fprintf (writefile, "PUSH %d\nPUSH %d\nPOP BP\nPOP SP\n", global_vars, global_vars);
    fprintf (writefile, "PUSH %d\nPOP SP\n", main_vars + global_vars);
    fprintf (writefile, "CALL main\nEND\n");
    ASM::NodeToASM (writefile, node);
    fclose (writefile);
}

void ASM::NodeToASM (FILE *writefile, Node *node) {
    if (node->type == TYPE_NUM) {
        fprintf (writefile, "PUSH %lg\n", node->data);
    }
    else if (node->type == TYPE_VAR) {
        if (asm_vars[(int)node->data].is_global)
            fprintf (writefile, "PUSH (%d)\n", asm_vars[(int)node->data].RAM_idx);
        else
            fprintf (writefile, "PUSH [%d]\n", asm_vars[(int)node->data].RAM_idx);
    }
    else if (node->type == TYPE_OP) {
        ASM::NodeToASM (writefile, node->left);
        ASM::NodeToASM (writefile, node->right);
        switch ((int)node->data) {
            case OP_SUM: {
                fprintf (writefile, "ADD\n");
                break;
            }
            case OP_SUB: {
                fprintf (writefile, "SUB\n");
                break;
            }
            case OP_MUL: {
                fprintf (writefile, "MUL\n");
                break;
            }
            case OP_DIV: {
                fprintf (writefile, "DIV\n");
                break;
            }
            case OP_POW: {
                fprintf (writefile, "POW\n");
                break;
            }
            case OP_ABOVE: {
                fprintf (writefile, "JBE ");
                break;
            }
            case OP_ABOVE_EQUAL: {
                fprintf (writefile, "JB ");
                break;
            }
            case OP_BELOW: {
                fprintf (writefile, "JAE ");
                break;
            }
            case OP_BELOW_EQUAL: {
                fprintf (writefile, "JA ");
                break;
            }
            case OP_EQUAL: {
                fprintf (writefile, "JNE ");
                break;
            }
            case OP_UNEQUAL: {
                fprintf (writefile, "JE ");
                break;
            }
        }
    }
    else if (node->type == TYPE_FUNC) {
        if (node->parent->type == TYPE_SYS && node->parent->data == SEMICOLON) {
            fprintf(writefile, "\n%s:\n", funcs[(int) node->data].name);
            ASM::NodeToASM (writefile, node->right);
        }
        else {
            Node *base = node;
            fprintf (writefile, "PUSH BP\nPOP [SP]\n");
            while (node->left && node->left->type == TYPE_SYS && node->left->data == COMMA) {
                ASM::NodeToASM (writefile, node->left->right);
                fprintf (writefile, "POP [SP]\n");
                node = node->left;
            }
            node = base;
            fprintf (writefile, "PUSH SP\nPUSH %d\nSUB\nPOP BP\n", (int)funcs[(int) node->data].val);
            fprintf (writefile, "CALL %s\n", funcs[(int) node->data].name);
            fprintf (writefile, "PUSH [-1]\nPUSH BP\nPUSH 1\nSUB\nPOP SP\nPOP BP\n");
        }
    }
    else if (node->type == TYPE_SYS) {
        switch ((int)node->data) {
            case EQUAL: {
                ASM::NodeToASM (writefile, node->right);
	            if (asm_vars[(int)node->left->data].is_global)
	            	fprintf (writefile, "POP (%d)\n", asm_vars[(int)node->left->data].RAM_idx);
	            else
		            fprintf (writefile, "POP [%d]\n", asm_vars[(int)node->left->data].RAM_idx);
                break;
            }
            case RET: {
                if (node->left->type == TYPE_NUM)
                    fprintf (writefile, "PUSH %lg\n", node->left->data);
                else if (node->left->type == TYPE_VAR) {
                    if (asm_vars[(int)node->left->data].is_global)
                        fprintf (writefile, "PUSH (%d)\n", asm_vars[(int)node->left->data].RAM_idx);
                    else
                        fprintf (writefile, "PUSH [%d]\n", asm_vars[(int)node->left->data].RAM_idx);
                }
                else
                    ASM::NodeToASM (writefile, node->left);
                fprintf (writefile, "RET\n");
                break;
            }
            case PUT: {
                if (node->left->type == TYPE_NUM)
                    fprintf (writefile, "PUSH %lg\n", node->left->data);
                else if (node->left->type == TYPE_VAR) {
                    if (asm_vars[(int)node->left->data].is_global)
                        fprintf (writefile, "PUSH (%d)\n", asm_vars[(int)node->left->data].RAM_idx);
                    else
                        fprintf (writefile, "PUSH [%d]\n", asm_vars[(int)node->left->data].RAM_idx);
                }
                else
                    ASM::NodeToASM (writefile, node->left);
                fprintf (writefile, "OUT\n");
                break;
            }
            case GET: {
	            if (asm_vars[(int)node->left->data].is_global)
		            fprintf (writefile, "IN\nPOP (%d)\n", asm_vars[(int)node->left->data].RAM_idx);
	            else
		            fprintf (writefile, "IN\nPOP [%d]\n", asm_vars[(int)node->left->data].RAM_idx);
                break;
            }
            case IF: {
                size_t num = NUM_IF++;
                //TODO Рассмотреть случай без ELSE
                ASM::NodeToASM (writefile, node->left);
                fprintf (writefile, "elseif_%d\n", num);
                ASM::NodeToASM (writefile, node->right->left);
                fprintf (writefile, "JMP endif_%d\n", num);
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