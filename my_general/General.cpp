#include "General.h"

size_t idx = 0;
bool flag = true;
int brace_flag = 0; //Счётчик (колво "{" - колво "}"), чтобы знать в функции сейчас находимся или нет
bool just_added_variable = false;
bool just_entered_function = false; //Необходимо для пропусков названия функции и списка ее переменных (они находятся после слова function и до открывающейся скобки ,=>
//необходимо чтобы function flag при чтении этих токенов не изменялся
int function_flag = -1;

size_t var_idx = 0;
size_t func_idx = 0;

size_t move = 0;
size_t delta = 0;
int main_flag = -1;

Variable_t vars [ARRAY_SIZE];
Function_t funcs [ARRAY_SIZE];

const char *Operations[] = {
        "+",
        "-",
        "*",
        "/",
        "^",
        "deriv",
        ">",
        ">=",
        "<",
        "<=",
        "==",
        "!="
};

const char *LangCommands[] = {
        "op",
        "var",
        "function",
        "call",
        "ret",
        "put",
        "get",
        "if",
        "while",
        "if-else",
        "=",
        ";",
        ",",
        "(",
        ")",
        "{",
        "}"
};

const char *BalletOperations[] = {
		"attitude",
		"battement",
		"battement-tendu-jete",
		"assemble",
		"jete",
		"echappe",
		"releve-grand-plie",
		"grand-plie",
		"releve-petit-plie",
		"petit-plie",
		"demi-plie",
		"releve-demi-plie"
};

const char *BalletLangCommands[] = {
		"op",
		"allegro",
		"adagio",
		"grande",
		"renverse",
		"endehors",
		"endedans",
		"port-de-bras",
		"while", //TODO Добавить
		"pas-de-bourree",
		"arabesque",
		"pas",
		"tour",
		"enface",
		"croisee",
		"{",
		"}"
};

Node *Tree::NodeInit (Node *parent, Node *left, Node *right) {
    Node *node = (Node *) calloc (1, sizeof (Node));
    if (parent)
        node->parent = parent;
    if (left)
        node->left = left;
    if (right)
        node->right = right;
    return node;
}

int Tree::VarSearch (char *name, bool allow_to_add) {
    for (size_t i = 0; i < var_idx; ++i) {
        if (strcmp(vars[i].name, name) == 0 && (vars[i].val == function_flag || vars[i].val == -1)) {
            return i;
        }
    }
    if (allow_to_add) {
        strcpy(vars[var_idx].name, name);
        vars[var_idx++].val = function_flag;
        just_added_variable = true;
        return var_idx - 1;
    } else {
        return NOTFOUND;
    }
}

int Tree::FuncSearch (char *name, bool allow_to_add) {
    for (size_t i = 0; i < func_idx; ++i) {
        if (strcmp(funcs[i].name, name) == 0) {
            return i;
        }
    }
    if (allow_to_add) {
        strcpy(funcs[func_idx++].name, name);
        return func_idx - 1;
    } else {
        return NOTFOUND;
    }
}

void Tree::TreeOffsetCorrecter (Node *node) {
    if (!node->parent)
        node->level = 1;
    if (node->left) {
        node->left->parent = node;
        node->left->level = node->level + 1;
        Tree::TreeOffsetCorrecter (node->left);
    }
    if (node->right) {
        node->right->parent = node;
        node->right->level = node->level + 1;
        Tree::TreeOffsetCorrecter (node->right);
    }
}

void Tree::EmptyNodesCleaner (Node *node) {
    if (node->left)
        Tree::EmptyNodesCleaner (node->left);
    if (node->right)
        Tree::EmptyNodesCleaner (node->right);
    if (node->type == TYPE_UNDEF) {
        if (!node->left && node->right) {
            flag = true;
            NODE_REF = node->right;
        } else if (node->left && !node->right) {
            flag = true;
            NODE_REF = node->left;
        } else if (!node->left && !node->right) {
            flag = true;
            NODE_REF = nullptr;
        }
    }
    else if (node->type == TYPE_SYS && node->data == OP && !node->left)
        NODE_REF = node->right;
    else if (node->type == TYPE_SYS && node->data == SEMICOLON && !node->left)
        NODE_REF = node->right;
}

void Tree::FreeNode (Node *node) {
    if (!node)
        return;
    if (node->left)
        Tree::FreeNode (node->left);
    if (node->right)
        Tree::FreeNode (node->right);
    free (node);
}

void Dot::PrintTree (Node *node) {
    FILE *dot_writefile = fopen ("../my_dot/temp.dot", "w");
    fprintf (dot_writefile,"digraph G {\nfontsize = 50\n");
    Dot::PrintNode (dot_writefile, node);
    fprintf (dot_writefile, "}");
    fclose (dot_writefile);
    system ("dot -Tpng /home/biscuitslayer/CLionProjects/9_MyLanguage/my_dot/temp.dot -o/home/biscuitslayer/CLionProjects/9_MyLanguage/my_dot/temp.png");
}

void Dot::PrintNode (FILE *writefile, Node *node) {
    if (node == nullptr)
        return;
    char *label = Dot::MakeNodeLabel (node);
    char *color = Dot::MakeNodeColor (node);
    char *shape = Dot::MakeNodeShape (node);
    if (node->left) {
        char *label1 = Dot::MakeNodeLabel (node->left);
        char *color1 = Dot::MakeNodeColor (node->left);
        char *shape1 = Dot::MakeNodeShape (node->left);
        fprintf (writefile, "%zu[label = \"%s\", style = \"filled\", fillcolor = \"%s\", shape = \"%s\"]\n%zu[label = \"%s\", style = \"filled\", fillcolor = \"%s\", shape = \"%s\"]\n", node, label, color, shape, node->left, label1, color1, shape1);
        fprintf (writefile, "%zu -> %zu\n", node, node->left);
        PrintNode (writefile, node->left);
        free (label1);
    }
    if (node->right) {
        char *label2 = Dot::MakeNodeLabel (node->right);
        char *color2 = Dot::MakeNodeColor (node->right);
        char *shape2 = Dot::MakeNodeShape (node->right);
        fprintf (writefile, "%zu[label = \"%s\", style = \"filled\", fillcolor = \"%s\", shape = \"%s\"]\n%zu[label = \"%s\", style = \"filled\", fillcolor = \"%s\", shape = \"%s\"]\n", node, label, color, shape, node->right, label2, color2, shape2);
        fprintf (writefile, "%zu -> %zu\n", node, node->right);
        PrintNode (writefile, node->right);
        free (label2);
    }
    if (!node->left && !node->right) {
        fprintf (writefile, "%zu[label = \"%s\", style = \"filled\", fillcolor = \"%s\", shape = \"%s\"]\n", node, label, color, shape);
    }
    free (label);
}

char *Dot::MakeNodeLabel (Node *node) {
    char *label = (char *) calloc (STR_LEN, sizeof (char));
    if (node->type == TYPE_NUM) {
        sprintf (label, NUM_T_FORMAT, node->data);
    }
    else if (node->type == TYPE_VAR) {
        sprintf (label, "%s", vars[(int)node->data].name);
    }
    else if (node->type == TYPE_FUNC) {
        sprintf (label, "$%s", funcs[(int)node->data].name);
    }
    else if (node->type == TYPE_OP) {
        sprintf (label, "%s", Operations[(int)node->data]);
    }
    else if (node->type == TYPE_SYS) {
        sprintf (label, "%s", LangCommands[(int)node->data]);
    }
    else if (node->type == TYPE_UNDEF) {
        sprintf (label, "UNDEF");
        printf ("Error! Undefined command\n");
#ifdef NDEBUG
        exit (1);
#endif
    }
    return label;
}

char *Dot::MakeNodeColor (Node *node) {
    char *color = (char *) calloc (STR_LEN, sizeof (char));
    if (node->type == TYPE_NUM) {
        sprintf (color, "darkseagreen2");
    } else if (node->type == TYPE_VAR) {
        sprintf (color, "darkolivegreen1");
    } else if (node->type == TYPE_FUNC) {
        sprintf (color, "bisque");
    } else if (node->type == TYPE_OP) {
        switch ((int)node->data) {
            case OP_SUM: {
                sprintf (color, "gold");
                break;
            }
            case OP_SUB: {
                sprintf (color, "orchid2");
                break;
            }
            case OP_MUL: {
                sprintf (color, "pink");
                break;
            }
            case OP_DIV: {
                sprintf (color, "peachpuff");
                break;
            }
            case OP_POW: {
                sprintf (color, "skyblue");
                break;
            }
            default : {
                sprintf (color, "lawngreen");
                break;
            }
        }
    } else if (node->type == TYPE_SYS) {
        switch ((int)node->data) {
            case OP: {
                sprintf (color, "hotpink");
                break;
            }
            case VAR: {
                sprintf (color, "yellow");
                break;
            }
            case RET: {
                sprintf (color, "sandybrown");
                break;
            }
            case PUT: {
                sprintf (color, "steelblue1");
                break;
            }
            case GET: {
                sprintf (color, "steelblue1");
                break;
            }
            case IF: {
                sprintf (color, "springgreen");
                break;
            }
            case WHILE: {
                sprintf (color, "olivedrab1");
                break;
            }
            case IF_ELSE: {
                sprintf (color, "springgreen");
                break;
            }
            case EQUAL: {
                sprintf (color, "aquamarine");
                break;
            }
            case SEMICOLON: {
                sprintf (color, "khaki1");
                break;
            }
            case COMMA: {
                sprintf (color, "violet");
                break;
            }
            case OPEN_PARENTHESIS: {
                sprintf (color, "wheat1");
                break;
            }
            case CLOSE_PARENTHESIS: {
                sprintf (color, "wheat1");
                break;
            }
            case OPEN_BRACE: {
                sprintf (color, "plum1");
                break;
            }
            case CLOSE_BRACE: {
                sprintf (color, "plum1");
                break;
            }
            default : {
                sprintf (color, "black");
                break;
            }
        }
    } else if (node->type == TYPE_UNDEF) {
        sprintf (color, "firebrick1");
        printf ("Error! Undefined command\n");
#ifdef NDEBUG
        exit (1);
#endif
    }
    return color;
}

char *Dot::MakeNodeShape (Node *node) {
    char *shape = (char *) calloc (STR_LEN, sizeof (char));
    if (node->type == TYPE_NUM) {
        sprintf (shape, "egg");
    } else if (node->type == TYPE_VAR) {
        sprintf (shape, "egg");
    } else if (node->type == TYPE_FUNC) {
        sprintf (shape, "box");
    } else if (node->type == TYPE_OP) {
        sprintf (shape, "diamond");
    } else if (node->type == TYPE_SYS) {
        switch ((int)node->data) {
            case OP: {
                sprintf (shape, "diamond");
                break;
            }
            case VAR: {
                sprintf (shape, "plaintext");
                break;
            }
            case RET: {
                sprintf (shape, "septagon");
                break;
            }
            case PUT: {
                sprintf (shape, "note");
                break;
            }
            case GET: {
                sprintf (shape, "note");
                break;
            }
            case IF: {
                sprintf (shape, "hexagon");
                break;
            }
            case WHILE: {
                sprintf (shape, "pentagon");
                break;
            }
            case IF_ELSE: {
                sprintf (shape, "hexagon");
                break;
            }
            case EQUAL: {
                sprintf (shape, "larrow");
                break;
            }
            case SEMICOLON: {
                sprintf (shape, "circle");
                break;
            }
            case COMMA: {
                sprintf (shape, "circle");
                break;
            }
            case OPEN_PARENTHESIS: {
                sprintf (shape, "circle");
                break;
            }
            case CLOSE_PARENTHESIS: {
                sprintf (shape, "circle");
                break;
            }
            case OPEN_BRACE: {
                sprintf (shape, "circle");
                break;
            }
            case CLOSE_BRACE: {
                sprintf (shape, "circle");
                break;
            }
            default : {
                sprintf (shape, "plaintext");
                break;
            }
        }
    } else if (node->type == TYPE_UNDEF) {
        sprintf (shape, "box");
        printf ("Error! Undefined command\n");
#ifdef NDEBUG
        exit (1);
#endif
    }
    return shape;
}

char *AST::ReadHandle (FILE *readfile) {
    char *readstr = (char *) calloc (STR_LEN, sizeof (char));
    fscanf(readfile, "%s", readstr);
    char *new_readstr = (char *) calloc (STR_LEN, sizeof (char));
    char *ans = new_readstr;
    while (*readstr != '\0') {
        if (*readstr == '@') {
            *new_readstr = '{';
            ++new_readstr;
            *new_readstr = *readstr;
            ++readstr;
            ++new_readstr;
            *new_readstr = '}';
            ++new_readstr;
        }
        else {
            *new_readstr = *readstr;
            ++readstr;
            ++new_readstr;
        }
    }
    return ans;
}

Node *AST::ReadTree (char *readstr, Node *parent) {
    char str[STR_LEN];
    Node *node = nullptr;
    if (sscanf (readstr + move, "{%[^\{}]%n", str, &delta) > 0) {
        move += delta;
        if (strcmp(str, "@") == 0)
            node = nullptr;
        else {
            node = Tree::NodeInit(parent);
            std::pair<type_t, num_t> temp = AST::GetNodeInfo(str);
            node->type = temp.first;
            node->data = temp.second;
            node->left = AST::ReadTree(readstr, node);
        }
        sscanf(readstr + move, "}%n", &delta);
        move += delta;
        if (sscanf(readstr + move, "{%[^\{}]%n", str, &delta) > 0) {
            move += delta;
            if (strcmp(str, "@") == 0)
                parent->right = nullptr;
            else {
                parent->right = Tree::NodeInit(parent);
                std::pair<type_t, num_t> temp = AST::GetNodeInfo(str);
                parent->right->type = temp.first;
                parent->right->data = temp.second;
                parent->right->left = AST::ReadTree(readstr, parent->right);
            }
            sscanf(readstr + move, "}%n", &delta);
            move += delta;
        }
    }
    return node;
}

std::pair <type_t, num_t> AST::GetNodeInfo (char *str) {
	if (isdigit (str[0])) {
		return {TYPE_NUM, strtod (str, nullptr)};
	}
	if (str[0] == '$') {
		++ str;
		return {TYPE_FUNC, Tree::FuncSearch (str, true)};
	}
	size_t size = sizeof (LangCommands) / sizeof (char *);
	for (size_t i = 0; i < size; ++ i) {
		if (strcmp (str, LangCommands[i]) == 0) {
			return {TYPE_SYS, i};
		}
	}
	size = sizeof (Operations) / sizeof (char *);
	for (size_t i = 0; i < size; ++ i) {
		if (strcmp (str, Operations[i]) == 0) {
			return {TYPE_OP, i};
		}
	}
	if (isalpha (str[0])) {
		return {TYPE_VAR, Tree::VarSearch (str, true)};
	}
	return {TYPE_UNDEF, 0};
}

void AST::PrintNode (FILE *writefile, Node *node) {
    fprintf (writefile, "{");
    char *label = Dot::MakeNodeLabel (node);
    fprintf (writefile, "%s", label);
    if (node->left || node->right) {
        if (node->left)
            AST::PrintNode(writefile, node->left);
        else
            fprintf(writefile, "@");
        if (node->right)
            AST::PrintNode(writefile, node->right);
        else
            fprintf(writefile, "@");
    }
    fprintf (writefile, "}");
}