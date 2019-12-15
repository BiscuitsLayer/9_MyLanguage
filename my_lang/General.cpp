#include "General.h"

size_t idx = 0;
bool flag = true;

size_t var_idx = 0;
size_t const_idx = 0;
size_t func_idx = 0;

Variable_t vars [ARRAY_SIZE];
Constant_t consts [ARRAY_SIZE];
Function_t funcs [ARRAY_SIZE];

const char *Operations[] = {
        "+",
        "-",
        "*",
        "/",
        "^",
        "diff",
        ">",
        ">=",
        "<",
        "<=",
        "==",
        "!="
};

const char *LangCommands[] = {
        "var",
        "const",
        "function",
        "call",
        "return",
        "print",
        "scan",
        "if",
        "while",
        "else",
        "=",
        ";",
        ",",
        "(",
        ")",
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
    if (node->left) {
        char *label1 = Dot::MakeNodeLabel (node->left);
        char *color1 = Dot::MakeNodeColor (node->left);
        fprintf (writefile, "%zu[label = \"%s\", style = \"filled\", fillcolor = \"%s\"]\n%zu[label = \"%s\", style = \"filled\", fillcolor = \"%s\"]\n", node, label, color, node->left, label1, color1);
        fprintf (writefile, "%zu -> %zu\n", node, node->left);
        PrintNode (writefile, node->left);
        free (label1);
    }
    if (node->right) {
        char *label2 = Dot::MakeNodeLabel (node->right);
        char *color2 = Dot::MakeNodeColor (node->right);
        fprintf (writefile, "%zu[label = \"%s\", style = \"filled\", fillcolor = \"%s\"]\n%zu[label = \"%s\", style = \"filled\", fillcolor = \"%s\"]\n", node, label, color, node->right, label2, color2);
        fprintf (writefile, "%zu -> %zu\n", node, node->right);
        PrintNode (writefile, node->right);
        free (label2);
    }
    if (!node->left && !node->right) {
        fprintf (writefile, "%zu[label = \"%s\", style = \"filled\", fillcolor = \"%s\"]\n", node, label, color);
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
    else if (node->type == TYPE_CONST) {
        sprintf (label, "%s", consts[(int)node->data].name);
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
    } else if (node->type == TYPE_CONST) {
        sprintf (color, "indianred1");
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
            case VAR: {
                sprintf (color, "yellow");
                break;
            }
            case CONST: {
                sprintf (color, "yellow");
                break;
            }
            case RETURN: {
                sprintf (color, "sandybrown");
                break;
            }
            case PRINT: {
                sprintf (color, "steelblue1");
                break;
            }
            case SCAN: {
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
            case ELSE: {
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
        sprintf (shape, NUM_T_FORMAT, node->data);
    } else if (node->type == TYPE_VAR) {
        sprintf (shape, "%s", vars[(int)node->data].name);
    } else if (node->type == TYPE_CONST) {
        sprintf (shape, "%s", consts[(int)node->data].name);
    } else if (node->type == TYPE_FUNC) {
        sprintf (shape, "$%s", funcs[(int)node->data].name);
    } else if (node->type == TYPE_OP) {
        sprintf (shape, "%s", Operations[(int)node->data]);
    } else if (node->type == TYPE_SYS) {
        sprintf (shape, "%s", LangCommands[(int)node->data]);
    } else if (node->type == TYPE_UNDEF) {
        sprintf (shape, "UNDEF");
        printf ("Error! Undefined command\n");
#ifdef NDEBUG
        exit (1);
#endif
    }
    return shape;
}

void AST::PrintTree (Node *node) {
    FILE *ast_writefile = fopen ("../my_ast/temp.ast", "w");
    AST::PrintNode (ast_writefile, node);
    fclose (ast_writefile);
}

void AST::PrintNode (FILE *writefile, Node *node) {
    fprintf (writefile, "{ ");
    char *label = Dot::MakeNodeLabel (node);
    fprintf (writefile, "%s ", label);
    if (node->left)
        AST::PrintNode (writefile, node->left);
    else
        fprintf (writefile, "{ nil } ");
    if (node->right)
        AST::PrintNode (writefile, node->right);
    else
        fprintf (writefile, "{ nil } ");
    fprintf (writefile, "} ");
}