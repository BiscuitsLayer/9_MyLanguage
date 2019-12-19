#include "Frontend_1.h"

FE_1_VAR_t fe_vars[ARRAY_SIZE];
bool just_skipped_line = false;
size_t shift = 0;

void Lang::TreeToLang (Node *node) {
    FILE *writefile = fopen ("../my_programs/ast_program.my_lang", "w");
    Lang::NodeToLang (writefile, node);
    fclose (writefile);
}

void Lang::PrintShift (FILE *writefile) {
	for (size_t i = 0; i < shift; ++i)
		fprintf (writefile, "\t");
}

void Lang::NodeToLang (FILE *writefile, Node *node) {
	if (just_skipped_line) {
		Lang::PrintShift (writefile);
		just_skipped_line = false;
	}
    if (node->type == TYPE_NUM) {
    	if (node->data < 0) {
    		fprintf (writefile, "( 0 - ");
		    fprintf (writefile, NUM_T_FORMAT, fabs (node->data));
		    fprintf (writefile, " )");
    	}
    	else
	        fprintf (writefile, NUM_T_FORMAT, node->data);
    }
    else if (node->type == TYPE_VAR) {
	    if (!fe_vars[(int) node->data].appeared && !just_entered_function) {
		    fprintf (writefile, "var %s", vars[(int)node->data].name);
		    fe_vars[(int) node->data].appeared = true;
	    } else {
	    	if (just_entered_function)
			    fe_vars[(int) node->data].appeared = true;
		    fprintf (writefile, "%s", vars[(int) node->data].name);
	    }
    }
    else if (node->type == TYPE_OP) {
	    if (node->data == OP_DERIV) {
		    fprintf (writefile, "deriv ( ");
		    Lang::NodeToLang (writefile, node->right);
		    fprintf (writefile, " , ");
		    Lang::NodeToLang (writefile, node->left);
		    fprintf (writefile, " )");
	    }
	    else {
		    Lang::NodeToLang (writefile, node->left);
		    fprintf (writefile, " ");
		    fprintf (writefile, Operations[(int) node->data]);
		    fprintf (writefile, " ");
		    Lang::NodeToLang (writefile, node->right);
	    }
    }
    else if (node->type == TYPE_FUNC) {
        if (node->parent->type == TYPE_SYS && node->parent->data == SEMICOLON) {
        	just_entered_function = true;
            fprintf (writefile, "function %s ( ", funcs[(int)node->data].name);
	        if (node->left) {
		        Lang::NodeToLang (writefile, node->left);
		        fprintf (writefile, " ) {\n");
	        }
	        else {
		        fprintf (writefile, ") {\n");
	        }
	        ++shift;
	        just_skipped_line = true;
	        just_entered_function = false;
	        Lang::NodeToLang (writefile, node->right);
	        --shift;
	        fprintf (writefile, "}\n");
	        if (node->parent->right)
		        fprintf (writefile, "\n");
	        just_skipped_line = true;
        }
        else {
			fprintf (writefile, "call %s ( ", funcs[(int)node->data].name);
	        if (node->left) {
		        Lang::NodeToLang (writefile, node->left);
		        fprintf (writefile, " )");
	        }
	        else {
		        fprintf (writefile, ")");
	        }
        }
    }
    else if (node->type == TYPE_SYS) {
        switch ((int)node->data) {
            case RET: {
                fprintf (writefile, "ret ");
	            Lang::NodeToLang (writefile, node->left);
                break;
            }
            case PUT: {
                fprintf (writefile, "put ( ");
                Lang::NodeToLang (writefile, node->left);
                fprintf (writefile, " )");
                break;
            }
            case GET: {
	            fprintf (writefile, "get ( ");
	            Lang::NodeToLang (writefile, node->left);
	            fprintf (writefile, " )");
	            break;
            }
            case IF: {
                //TODO BEZ ELSE
                fprintf (writefile, "if ( ");
                Lang::NodeToLang (writefile, node->left);
	            fprintf (writefile, " ) {\n");
	            ++shift;
	            just_skipped_line = true;
	            Lang::NodeToLang (writefile, node->right->left);
	            --shift;
	            Lang::PrintShift (writefile);
	            fprintf (writefile, "}\n");
	            just_skipped_line = true;
	            Lang::NodeToLang (writefile, node->right);
	            break;
            }
            case IF_ELSE: {
	            fprintf (writefile, "if-else {\n");
	            ++shift;
	            just_skipped_line = true;
	            Lang::NodeToLang (writefile, node->right);
	            --shift;
	            Lang::PrintShift (writefile);
	            fprintf (writefile, "}\n");
	            just_skipped_line = true;
                break;
            }
	        case EQUAL: {
		        Lang::NodeToLang (writefile, node->left);
		        fprintf (writefile, " = ");
		        Lang::NodeToLang (writefile, node->right);
		        break;
	        }
            case SEMICOLON: {
                Lang::NodeToLang (writefile, node->left);
                if (node->left->type == TYPE_SYS && node->left->data == EQUAL)
                	fprintf (writefile, " ;\n");
                if (node->right)
                    Lang::NodeToLang (writefile, node->right);
                break;
            }
            case COMMA: {
	            Lang::NodeToLang (writefile, node->right);
	            if (node->left) {
		            fprintf (writefile, " , ");
		            Lang::NodeToLang (writefile, node->left);
	            }
	            break;
            }
            case OP: {
                Lang::NodeToLang (writefile, node->left);
                if (!(node->left->type == TYPE_SYS && node->left->data == IF))
                	fprintf (writefile, " ;\n");
	            just_skipped_line = true;
                if (node->right)
                    Lang::NodeToLang (writefile, node->right);
                break;
            }
        }
    }
}
