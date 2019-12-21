#include "BalletLang.h"
#include "TXLin.h"

BL_VAR_t bl_vars[ARRAY_SIZE];
bool bl_just_skipped_line = false;
size_t bl_shift = 0;

void BalletLang::PrintShift (FILE *writefile) {
	for (size_t i = 0; i < bl_shift; ++i)
		fprintf (writefile, "\t");
}

void BalletLang::NodeToLang (FILE *writefile, Node *node) {
	if (bl_just_skipped_line) {
		BalletLang::PrintShift (writefile);
		bl_just_skipped_line = false;
	}
	if (node->type == TYPE_NUM) {
		if (node->data < 0) {
			fprintf (writefile, "enface 0 battement ");
			fprintf (writefile, NUM_T_FORMAT, fabs (node->data));
			fprintf (writefile, " croisee");
		}
		else
			fprintf (writefile, NUM_T_FORMAT, node->data);
	}
	else if (node->type == TYPE_VAR) {
		if (!bl_vars[(int) node->data].appeared && !just_entered_function) {
			fprintf (writefile, "allegro %s", vars[(int)node->data].name);
			bl_vars[(int) node->data].appeared = true;
		} else {
			if (just_entered_function)
				bl_vars[(int) node->data].appeared = true;
			fprintf (writefile, "%s", vars[(int) node->data].name);
		}
	}
	else if (node->type == TYPE_OP) {
		switch ((int)node->data) {
			case OP_SUM: {
				BalletLang::NodeToLang (writefile, node->left);
				fprintf (writefile, " attitude ");
				BalletLang::NodeToLang (writefile, node->right);
				break;
			}
			case OP_SUB: {
				BalletLang::NodeToLang (writefile, node->left);
				fprintf (writefile, " battement ");
				BalletLang::NodeToLang (writefile, node->right);
				break;
			}
			case OP_MUL: {
				BalletLang::NodeToLang (writefile, node->left);
				fprintf (writefile, " battement-tendu-jete ");
				BalletLang::NodeToLang (writefile, node->right);
				break;
			}
			case OP_DIV: {
				BalletLang::NodeToLang (writefile, node->left);
				fprintf (writefile, " assemble ");
				BalletLang::NodeToLang (writefile, node->right);
				break;
			}
			case OP_POW: {
				BalletLang::NodeToLang (writefile, node->left);
				fprintf (writefile, " jete ");
				BalletLang::NodeToLang (writefile, node->right);
				break;
			}
			case OP_DERIV: {
				fprintf (writefile, "echappe enface ");
				BalletLang::NodeToLang (writefile, node->right);
				fprintf (writefile, " tour ");
				BalletLang::NodeToLang (writefile, node->left);
				fprintf (writefile, " croisee");
				break;
			}
			case OP_ABOVE: {
				BalletLang::NodeToLang (writefile, node->left);
				fprintf (writefile, " releve-grand-plie ");
				BalletLang::NodeToLang (writefile, node->right);
				break;
			}
			case OP_ABOVE_EQUAL: {
				BalletLang::NodeToLang (writefile, node->left);
				fprintf (writefile, " grand-plie ");
				BalletLang::NodeToLang (writefile, node->right);
				break;
			}
			case OP_BELOW: {
				BalletLang::NodeToLang (writefile, node->left);
				fprintf (writefile, " releve-petit-plie ");
				BalletLang::NodeToLang (writefile, node->right);
				break;
			}
			case OP_BELOW_EQUAL: {
				BalletLang::NodeToLang (writefile, node->left);
				fprintf (writefile, " petit-plie ");
				BalletLang::NodeToLang (writefile, node->right);
				break;
			}
			case OP_EQUAL: {
				BalletLang::NodeToLang (writefile, node->left);
				fprintf (writefile, " demi-plie ");
				BalletLang::NodeToLang (writefile, node->right);
				break;
			}
			case OP_UNEQUAL: {
				BalletLang::NodeToLang (writefile, node->left);
				fprintf (writefile, " releve-demi-plie ");
				BalletLang::NodeToLang (writefile, node->right);
				break;
			}
		}
	}
	else if (node->type == TYPE_FUNC) {
		if (node->parent->type == TYPE_SYS && node->parent->data == SEMICOLON) {
			just_entered_function = true;
			fprintf (writefile, "adagio %s enface ", funcs[(int)node->data].name);
			if (node->left) {
				BalletLang::NodeToLang (writefile, node->left);
				fprintf (writefile, " croisee {\n");
			}
			else {
				fprintf (writefile, "croisee {\n");
			}
			++bl_shift;
			bl_just_skipped_line = true;
			just_entered_function = false;
			BalletLang::NodeToLang (writefile, node->right);
			--bl_shift;
			fprintf (writefile, "}\n");
			if (node->parent->right)
				fprintf (writefile, "\n");
			bl_just_skipped_line = true;
		}
		else {
			fprintf (writefile, "grande %s enface ", funcs[(int)node->data].name);
			if (node->left) {
				BalletLang::NodeToLang (writefile, node->left);
				fprintf (writefile, " croisee");
			}
			else {
				fprintf (writefile, "croisee");
			}
		}
	}
	else if (node->type == TYPE_SYS) {
		switch ((int)node->data) {
			case RET: {
				fprintf (writefile, "renverse ");
				BalletLang::NodeToLang (writefile, node->left);
				break;
			}
			case PUT: {
				fprintf (writefile, "endehors enface ");
				BalletLang::NodeToLang (writefile, node->left);
				fprintf (writefile, " croisee");
				break;
			}
			case GET: {
				fprintf (writefile, "endedans enface ");
				BalletLang::NodeToLang (writefile, node->left);
				fprintf (writefile, " croisee");
				break;
			}
			case IF: {
				//TODO Рассмотреть случай без ELSE
				fprintf (writefile, "port-de-bras enface ");
				BalletLang::NodeToLang (writefile, node->left);
				fprintf (writefile, " croisee {\n");
				++bl_shift;
				bl_just_skipped_line = true;
				BalletLang::NodeToLang (writefile, node->right->left);
				--bl_shift;
				BalletLang::PrintShift (writefile);
				fprintf (writefile, "}\n");
				bl_just_skipped_line = true;
				BalletLang::NodeToLang (writefile, node->right);
				break;
			}
			case IF_ELSE: {
				fprintf (writefile, "pas-de-bourree {\n");
				++bl_shift;
				bl_just_skipped_line = true;
				BalletLang::NodeToLang (writefile, node->right);
				--bl_shift;
				BalletLang::PrintShift (writefile);
				fprintf (writefile, "}\n");
				bl_just_skipped_line = true;
				break;
			}
			case EQUAL: {
				BalletLang::NodeToLang (writefile, node->left);
				fprintf (writefile, " arabesque ");
				BalletLang::NodeToLang (writefile, node->right);
				break;
			}
			case SEMICOLON: {
				BalletLang::NodeToLang (writefile, node->left);
				if (node->left->type == TYPE_SYS && node->left->data == EQUAL)
					fprintf (writefile, " pas\n");
				if (node->right)
					BalletLang::NodeToLang (writefile, node->right);
				break;
			}
			case COMMA: {
				BalletLang::NodeToLang (writefile, node->right);
				if (node->left) {
					fprintf (writefile, " tour ");
					BalletLang::NodeToLang (writefile, node->left);
				}
				break;
			}
			case OP: {
				BalletLang::NodeToLang (writefile, node->left);
				if (!(node->left->type == TYPE_SYS && node->left->data == IF))
					fprintf (writefile, " pas\n");
				bl_just_skipped_line = true;
				if (node->right)
					BalletLang::NodeToLang (writefile, node->right);
				break;
			}
		}
	}
}
