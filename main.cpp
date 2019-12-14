#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <cstring>
#include <cassert>
#include <cctype>

//TODO Прогнать через валгринд, убедиться в том, что пустые узлы освобождаются
//TODO Дописать в грамматику функции (математические)
//TODO Добавить [] в процессор
//TODO Сделать константы, наконец, неизменными!
//TODO Файл с синтаксисом
//TODO Выделить весь фронтэнд в либу
//TODO Начать делать бекэнд
//TODO Операции больше меньше равно
//TODO Придумать как на этом этапе отличать глобальные переменные от локальных

typedef size_t type_t;
typedef double num_t;
size_t idx = 0;
bool flag = true;
const int STR_LEN = 100;
const int ARRAY_SIZE = 1000;
const int EXTRA_BUF_SIZE = 1;

#define NDEBUG
#define NOTFOUND -1
#define NUM_T_FORMAT "%lg"
#define INPUTFILE "../my_programs/program1.my_lang"
#define NODE_REF (node->parent->left == node ? node->parent->left : node->parent->right)

struct Variable_t {
    char name [STR_LEN] = {};
    num_t val = 0;
};

typedef Variable_t Constant_t;
typedef Variable_t Function_t;

Variable_t vars [ARRAY_SIZE];
size_t var_idx = 0;
Constant_t consts [ARRAY_SIZE];
size_t const_idx = 0;
Function_t funcs [ARRAY_SIZE];
size_t func_idx = 0;

enum Types {
    TYPE_UNDEF = 0,
    TYPE_SYS,
    TYPE_OP,
    TYPE_NUM,
    TYPE_VAR,
    TYPE_CONST,
    TYPE_FUNC
};

enum LangCommands {
    VAR = 0,
    CONST,
    FUNCTION,
    PRINT,
    SCAN,
    IF,
    WHILE,
    ELSE,
    EQUAL,
    SEMICOLON,
    COMMA,
    OPEN_PARENTHESIS,
    CLOSE_PARENTHESIS,
    OPEN_BRACE,
    CLOSE_BRACE
};

enum Operations {
    OP_SUM = 0,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_POW
};

const char *Operations[] = {
        "+",
        "-",
        "*",
        "/",
        "^"
};

const char *LangCommands[] = {
        "var",
        "const",
        "function",
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

typedef struct Node {
    Node *parent = nullptr;
    num_t data = 0;
    size_t level = 0;
    type_t type = TYPE_UNDEF;
    Node *right = nullptr;
    Node *left = nullptr;
} Node;

typedef struct {
    char *name = nullptr;
    type_t type = 0;
    size_t line_num = 0;
} Elem_t;

namespace Tree {
    Node *NodeInit (Node *parent = nullptr, Node *left = nullptr, Node *right = nullptr);
    void TreeOffsetCorrecter (Node *node);
    void EmptyNodesCleaner (Node *node);
    void FreeNode (Node *node);
}

namespace Dot {
    void PrintTree (Node *node);
    void PrintNode (FILE *writefile, Node *node);
    char *MakeNodeLabel (Node *node);
    char *MakeNodeColor (Node *node);
    char *MakeNodeShape (Node *node);
}

namespace AST {
    void PrintTree (Node *node);
    void PrintNode (FILE *writefile, Node *node);
}

namespace Tokens {
    Elem_t *Tokenization (FILE *readfile);
    void TokenHandle (Elem_t *tokens);
    int VarSearch (char *name, bool allow_to_add = false);
    int ConstSearch (char *name, bool allow_to_add = false);
    int FuncSearch (char *name, bool allow_to_add = false);
}

//Рекурсивный спуск
namespace RD {
    Node *GetG (Elem_t *tokens);
    Node *GetF (Elem_t *tokens);
    Node *GetArgs (Elem_t *tokens);
    Node *GetAs (Elem_t *tokens);
    Node *GetID (Elem_t *tokens);
    Node *GetE (Elem_t *tokens);
    Node *GetT (Elem_t *tokens);
    Node *GetS (Elem_t *tokens);
    Node *GetP (Elem_t *tokens);
    Node *GetN (Elem_t *tokens);
    Node *GetOp (Elem_t *tokens);
    Node *GetFCall (Elem_t *tokens);
    Node *GetIf (Elem_t *tokens);
    Node *GetWhile (Elem_t *tokens);
    Node *GetPrint (Elem_t *tokens);
    Node *GetScan (Elem_t *tokens);
}

int main () {
    FILE *readfile = fopen (INPUTFILE, "rb");
    if (!readfile) {
        printf ("Error opening file\n");
        return 0;
    }

    Elem_t *tokens = Tokens::Tokenization (readfile);

    idx = 0;

    Node *root = RD::GetG (tokens);
    Dot::PrintTree (root);
    AST::PrintTree (root);
    Tree::FreeNode (root);
    return 0;
}

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
    } else if (node->type == TYPE_VAR) {
        sprintf (label, "%s", vars[(int)node->data].name);
    } else if (node->type == TYPE_CONST) {
        sprintf (label, "%s", consts[(int)node->data].name);
    } else if (node->type == TYPE_FUNC) {
        sprintf (label, "$%s", funcs[(int)node->data].name);
    } else if (node->type == TYPE_OP) {
        sprintf (label, "%s", Operations[(int)node->data]);
    } else if (node->type == TYPE_SYS) {
        sprintf (label, "%s", LangCommands[(int)node->data]);
    } else if (node->type == TYPE_UNDEF) {
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
                sprintf (color, "ivory");
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
            case FUNCTION: {
                sprintf (color, "yellow");
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

Elem_t *Tokens::Tokenization (FILE *readfile) {
    struct stat file_info = {};
    stat (INPUTFILE, &file_info);
    char *program = (char *) calloc (file_info.st_size + EXTRA_BUF_SIZE, sizeof (char));
    program[file_info.st_size] = EOF;
    fread (program, sizeof (char), file_info.st_size, readfile);
    Elem_t *tokens = nullptr;
    tokens = (Elem_t *) calloc (file_info.st_size, sizeof(Elem_t));
    char *s = program;
    size_t line_num = 1;
    while (*s != EOF) {
        (tokens[idx].name) = s;
        while (!isspace(*s) && *s != EOF) {
            ++s;
        }
        if (*s == EOF) {
            *s = '\0';
            break;
        } else {
            if (*s == '\n')
                ++line_num;
            *s = '\0';
        }
        ++s;
        if (strcmp (tokens[idx].name, "") != 0) {
            tokens[idx].line_num = line_num;
            Tokens::TokenHandle (tokens); //Предварительная обработка токена
            ++idx; //Переход к следующему токену
        }
    }
    return tokens;
}

void Tokens::TokenHandle (Elem_t *tokens) {
    size_t size = sizeof (LangCommands)/ sizeof (char *);
    for (size_t i = 0; i < size; ++i) {
        if (strcmp(tokens[idx].name, LangCommands[i]) == 0) {
            tokens[idx].type = TYPE_SYS;
            return;
        }
    }
    if (strcmp(tokens[idx].name, "+") == 0 || strcmp(tokens[idx].name, "-") == 0) {
        tokens[idx].type = TYPE_OP;
        return;
    } else if (strcmp(tokens[idx].name, "*") == 0 || strcmp(tokens[idx].name, "/") == 0) {
        tokens[idx].type = TYPE_OP;
        return;
    } else if (strcmp(tokens[idx].name, "^") == 0) {
        tokens[idx].type = TYPE_OP;
        return;
    } else if (isdigit(tokens[idx].name[0])) {
        tokens[idx].type = TYPE_NUM;
        return;
    } else if ((idx > 0 && strcmp(tokens[idx - 1].name, LangCommands[VAR]) == 0) || Tokens::VarSearch (tokens[idx].name) != NOTFOUND) {
        tokens[idx].type = TYPE_VAR;
        Tokens::VarSearch (tokens[idx].name, true);
        return;
    } else if ((idx > 0 && strcmp(tokens[idx - 1].name, LangCommands[CONST]) == 0) || Tokens::ConstSearch (tokens[idx].name) != NOTFOUND) {
        tokens[idx].type = TYPE_CONST;
        Tokens::ConstSearch (tokens[idx].name, true);
        return;
    } else if ((idx > 0 && strcmp(tokens[idx - 1].name, LangCommands[FUNCTION]) == 0) || Tokens::FuncSearch (tokens[idx].name) != NOTFOUND) {
        tokens[idx].type = TYPE_FUNC;
        Tokens::FuncSearch (tokens[idx].name, true);
        return;
    } else if (tokens[idx].type == TYPE_UNDEF) {
        printf ("Unknown token \"%s\" in line %d\n", tokens[idx].name, tokens[idx].line_num);
        exit (1);
    }
}

int Tokens::VarSearch (char *name, bool allow_to_add) {
    for (size_t i = 0; i < var_idx; ++i) {
        if (strcmp(vars[i].name, name) == 0) {
            return i;
        }
    }
    if (allow_to_add) {
        strcpy(vars[var_idx++].name, name);
        return var_idx - 1;
    } else {
        return NOTFOUND;
    }
}

int Tokens::ConstSearch (char *name, bool allow_to_add) {
    for (size_t i = 0; i < const_idx; ++i) {
        if (strcmp(consts[i].name, name) == 0) {
            return i;
        }
    }
    if (allow_to_add) {
        strcpy(consts[const_idx++].name, name);
        return const_idx - 1;
    } else {
        return NOTFOUND;
    }
}

int Tokens::FuncSearch (char *name, bool allow_to_add) {
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

Node *RD::GetG (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    Node *ans = node;
    node->data = SEMICOLON;
    node->type = TYPE_SYS;

    while (tokens[idx].name && strcmp (tokens[idx].name, LangCommands[FUNCTION]) == 0) {
        node->left = Tree::NodeInit();
        node->left = RD::GetF(tokens);

        node->right = Tree::NodeInit();
        node->right->data = SEMICOLON;
        node->right->type = TYPE_SYS;
        node = node->right;
    }

#ifdef NDEBUG
    Tree::TreeOffsetCorrecter (ans);
    while (flag) {
        flag = false;
        Tree::EmptyNodesCleaner(ans);
    }
#endif
    return ans;
}

Node *RD::GetF (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    Node *base = node;
    node->type = TYPE_FUNC;
    assert (strcmp (tokens[idx].name, LangCommands[FUNCTION]) == 0);
    ++idx;
    node->data = Tokens::FuncSearch (tokens[idx].name);
    ++idx;

    node->left = Tree::NodeInit ();
    node->left = RD::GetArgs (tokens);

    node->right = Tree::NodeInit ();
    node->right->type = TYPE_SYS;
    node->right->data = SEMICOLON;
    node = node->right;

    assert (strcmp (tokens[idx].name, LangCommands[OPEN_BRACE]) == 0);
    ++idx;

    while (strcmp (tokens[idx].name, LangCommands[CLOSE_BRACE]) != 0) {
        node->left = RD::GetOp(tokens);
        node->right = Tree::NodeInit ();
        node->right->type = TYPE_SYS;
        node->right->data = SEMICOLON;
        node = node->right;
    }
    ++idx;
    return base;
}

Node *RD::GetArgs (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    Node *ans = node;
    assert (strcmp (tokens[idx].name, LangCommands[OPEN_PARENTHESIS]) == 0);
    ++idx;

    node->left = Tree::NodeInit ();
    node->left->type = TYPE_SYS;
    node->left->data = COMMA;
    node = node->left;

    while (strcmp (tokens[idx].name, LangCommands[CLOSE_PARENTHESIS]) != 0) {
        node->right = RD::GetID (tokens);
        if (strcmp (tokens[idx].name, LangCommands[COMMA]) == 0) {
            node->left = Tree::NodeInit();
            node->left->type = TYPE_SYS;
            node->left->data = COMMA;
            node = node->left;
            ++idx;
        }
    }
    ++idx;
    return ans;
}

Node *RD::GetAs (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    Node *ans = node;
    node->left = RD::GetID (tokens);
    if (node->left->type == TYPE_CONST && strcmp(tokens[idx - 2].name, LangCommands[CONST]) != 0) {
        printf ("Error! Can't change constants (line %d)\n", tokens[idx].line_num);
        exit (2);
    }
    assert (strcmp (tokens[idx].name, LangCommands[EQUAL]) == 0);
    node->data = EQUAL;
    node->type = TYPE_SYS;
    ++idx;
    node->right = RD::GetE (tokens);
    assert (strcmp (tokens[idx].name, LangCommands[SEMICOLON]) == 0);
    ++idx;
    return ans;
}

Node *RD::GetID (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    if (tokens[idx].type == TYPE_VAR) {
        node->type = TYPE_VAR;
        node->data = Tokens::VarSearch (tokens[idx].name);
        if (node->data == NOTFOUND) {
            printf ("Error! Variable \"%s\" not found", tokens[idx].name);
            exit (1);
        }
    }
    else if (tokens[idx].type == TYPE_CONST) {
        node->type = TYPE_CONST;
        node->data = Tokens::ConstSearch (tokens[idx].name);
        if (node->data == NOTFOUND) {
            printf ("Error! Constant \"%s\" not found", tokens[idx].name);
            exit (1);
        }
    }
    ++idx;
    return node;
}

Node *RD::GetE (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    node->left = RD::GetT (tokens);
    while (strcmp (tokens[idx].name, Operations[OP_SUM]) == 0 || strcmp (tokens[idx].name, Operations[OP_SUB]) == 0) {
        node->type = TYPE_OP;
        char op = tokens[idx].name[0];
        ++idx;
        if (op == '+')
            node->data = OP_SUM;
        else
            node->data = OP_SUB;
        node->right = RD::GetT (tokens);
        if (strcmp (tokens[idx].name, Operations[OP_SUM]) == 0|| strcmp (tokens[idx].name, Operations[OP_SUB]) == 0) {
            Node *new_node = Tree::NodeInit (nullptr, node);
            node = new_node;
        }
    }
    if (node->right)
        return node;
    else
        return node->left;
}

Node *RD::GetT (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    node->left = RD::GetS (tokens);
    while (strcmp (tokens[idx].name, Operations[OP_MUL]) == 0 || strcmp (tokens[idx].name, Operations[OP_DIV]) == 0) {
        node->type = TYPE_OP;
        char op = tokens[idx].name[0];
        ++idx;
        if (op == '*')
            node->data = OP_MUL;
        else
            node->data = OP_DIV;
        node->right = RD::GetS (tokens);
        if (strcmp (tokens[idx].name, Operations[OP_MUL]) == 0 || strcmp (tokens[idx].name, Operations[OP_DIV]) == 0) {
            Node *new_node = Tree::NodeInit (nullptr, node);
            node = new_node;
        }
    }
    if (node->right)
        return node;
    else
        return node->left;
}

Node *RD::GetS (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    node->left = GetP (tokens);
    if (tokens[idx].name[0] == '^') {
        node->type = TYPE_OP;
        node->data = OP_POW;
        ++idx;
        node->right = GetP (tokens);
    }
    if (node->right)
        return node;
    else
        return node->left;
}

Node *RD::GetP (Elem_t *tokens) {
    Node *node = nullptr;
    if (strcmp (tokens[idx].name, LangCommands[OPEN_PARENTHESIS]) == 0) {
        ++idx;
        node = RD::GetE (tokens);
        assert (strcmp (tokens[idx].name, LangCommands[CLOSE_PARENTHESIS]) == 0);
        ++idx;
    } else if (isdigit (tokens[idx].name[0])) {
        node = RD::GetN (tokens);
    } else if (isalpha (tokens[idx].name[0])) {
        node = RD::GetID (tokens);
    } else {
        printf ("Unknown token \"%s\" in line %d\n", tokens[idx].name, tokens[idx].line_num);
        exit (1);
    }
    return node;
}

Node *RD::GetN (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    for (size_t i = 0; i < strlen(tokens[idx].name); ++i)
        assert (isdigit (tokens[idx].name[i]));
    node->data = strtod (tokens[idx].name, nullptr);
    node->type = TYPE_NUM;
    ++idx;
    return node;
}

Node *RD::GetOp (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    if (strcmp (tokens[idx + 1].name, LangCommands[EQUAL]) == 0) {
        node = RD::GetAs (tokens);
    }
    else if (strcmp (tokens[idx].name, LangCommands[IF]) == 0) {
        node = RD::GetIf (tokens);
    }
    else if (strcmp (tokens[idx].name, LangCommands[WHILE]) == 0) {
        node = RD::GetWhile (tokens);
    }
    else if (strcmp (tokens[idx].name, LangCommands[PRINT]) == 0) {
        node = RD::GetPrint (tokens);
    }
    else if (strcmp (tokens[idx].name, LangCommands[SCAN]) == 0) {
        node = RD::GetScan (tokens);
    }
    else if (strcmp (tokens[idx].name, LangCommands[FUNCTION]) == 0) {
        node = RD::GetFCall (tokens);
    }
    else if (strcmp (tokens[idx].name, LangCommands[OPEN_BRACE]) == 0) {
        Node *base = node;
        node->data = OPEN_BRACE;
        node->type = TYPE_SYS;
        ++idx;
        node->left = Tree::NodeInit();
        node->left = RD::GetOp(tokens);
        while (strcmp(tokens[idx].name, LangCommands[CLOSE_BRACE]) != 0) {
            node->right = Tree::NodeInit();
            node->right = RD::GetOp(tokens);
        }
        node = base;
        node->right = Tree::NodeInit();
        node->right->data = CLOSE_BRACE;
        node->right->type = TYPE_SYS;
        ++idx;
    }
    else if (strcmp (tokens[idx].name, LangCommands[FUNCTION]) == 0) {

    }
    else if (strcmp (tokens[idx].name, LangCommands[VAR]) == 0) {
        ++idx;
        node =  RD::GetAs (tokens);
    }
    else if (strcmp (tokens[idx].name, LangCommands[CONST]) == 0) {
        ++idx;
        node = RD::GetAs (tokens);
    }
    else {
        printf ("Error! Unknown operator \"%s\" in line %d\n", tokens[idx].name, tokens[idx].line_num);
        exit (1);
    }
    return node;
}

Node *RD::GetFCall (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    Node *base = node;
    node->type = TYPE_FUNC;
    assert (strcmp (tokens[idx].name, LangCommands[FUNCTION]) == 0);
    ++idx;
    node->data = Tokens::FuncSearch (tokens[idx].name);
    ++idx;
    node->left = Tree::NodeInit ();
    node->left = RD::GetArgs (tokens);
    assert (strcmp (tokens[idx].name, LangCommands[SEMICOLON]) == 0);
    ++idx;
    return base;
}

Node *RD::GetIf (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    assert (strcmp (tokens[idx].name, LangCommands[IF]) == 0);
    node->data = IF;
    node->type = TYPE_SYS;
    ++idx;
    assert (strcmp (tokens[idx].name, LangCommands[OPEN_PARENTHESIS]) == 0);
    ++idx;
    node->left = Tree::NodeInit ();
    node->left = RD::GetE (tokens);
    assert (strcmp (tokens[idx].name, LangCommands[CLOSE_PARENTHESIS]) == 0);
    ++idx;
    node->right = Tree::NodeInit ();
    node->right->data = ELSE;
    node->right->type = TYPE_SYS;
    node->right->left = Tree::NodeInit ();
    node->right->left = RD::GetOp (tokens);
    if (strcmp (tokens[idx].name, LangCommands[ELSE]) == 0) {
        ++idx;
        node->right->right = Tree::NodeInit ();
        node->right->right = RD::GetOp (tokens);
    }
    return node;
}

Node *RD::GetWhile (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    assert (strcmp (tokens[idx].name, LangCommands[WHILE]) == 0);
    node->data = WHILE;
    node->type = TYPE_SYS;
    ++idx;
    assert (strcmp (tokens[idx].name, LangCommands[OPEN_PARENTHESIS]) == 0);
    ++idx;
    node->left = Tree::NodeInit ();
    node->left = RD::GetE (tokens);
    assert (strcmp (tokens[idx].name, LangCommands[CLOSE_PARENTHESIS]) == 0);
    ++idx;
    node->right = Tree::NodeInit ();
    node->right = RD::GetOp (tokens);
    return node;
}

Node *RD::GetPrint (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    assert (strcmp (tokens[idx].name, LangCommands[PRINT]) == 0);
    node->data = PRINT;
    node->type = TYPE_SYS;
    ++idx;
    assert (strcmp (tokens[idx].name, LangCommands[OPEN_PARENTHESIS]) == 0);
    ++idx;
    node->right = Tree::NodeInit ();
    node->right = RD::GetE (tokens);
    assert (strcmp (tokens[idx].name, LangCommands[CLOSE_PARENTHESIS]) == 0);
    ++idx;
    assert (strcmp (tokens[idx].name, LangCommands[SEMICOLON]) == 0);
    ++idx;
    return node;
}

Node *RD::GetScan (Elem_t *tokens) {
    Node *node = Tree::NodeInit ();
    assert (strcmp (tokens[idx].name, LangCommands[SCAN]) == 0);
    node->data = SCAN;
    node->type = TYPE_SYS;
    ++idx;
    assert (strcmp (tokens[idx].name, LangCommands[OPEN_PARENTHESIS]) == 0);
    ++idx;
    node->right = Tree::NodeInit ();
    node->right = RD::GetID (tokens);
    assert (strcmp (tokens[idx].name, LangCommands[CLOSE_PARENTHESIS]) == 0);
    ++idx;
    assert (strcmp (tokens[idx].name, LangCommands[SEMICOLON]) == 0);
    ++idx;
    return node;
}