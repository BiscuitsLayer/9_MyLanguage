#ifndef General_h
#define General_h

#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <cstring>
#include <cassert>
#include <cctype>
#include <cmath>
#include <utility>
#include <sstream>

typedef size_t type_t;
typedef double num_t;
const int STR_LEN = 1000;
const int ARRAY_SIZE = 1000;
const int EXTRA_BUF_SIZE = 1;

//#define NDEBUG
#define NOTFOUND -1
#define NUM_T_FORMAT "%lg"
extern const char *INPUTFILE;
extern const char *INPUTTREE;

#define NODE_REF (node->parent->left == node ? node->parent->left : node->parent->right)

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
    OP = 0,
    VAR,
    FUNCTION,
    CALL,
    RET,
    PUT,
    GET,
    IF,
    WHILE, //TODO Добавить
    IF_ELSE,
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
    OP_POW,
    OP_DERIV,
    OP_ABOVE,
    OP_ABOVE_EQUAL,
    OP_BELOW,
    OP_BELOW_EQUAL,
    OP_EQUAL,
    OP_UNEQUAL
};

struct Variable_t {
    char name [STR_LEN] = {};
    num_t val = 0; //Для переменных - номер функции, для функций - количество переданных аргументов
    size_t line_num = 0; //Для переменных - номер строки в которой она появляется в данной функции, для функций - номер начальной строки
};

typedef Variable_t Function_t;

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

extern size_t idx;
extern bool flag;
extern int brace_flag; //Счётчик (колво "{" - колво "}"), чтобы знать в функции сейчас находимся или нет
extern int function_flag;
extern bool just_added_variable;
extern bool just_entered_function; //Необходимо для пропусков названия функции и списка ее переменных (они находятся после слова function и до открывающейся скобки ,=>
//необходимо чтобы function flag при чтении этих токенов не изменялся

extern size_t var_idx;
extern size_t func_idx;

extern size_t move;
extern size_t delta;
extern int main_flag;

extern Variable_t vars [ARRAY_SIZE];
extern Function_t funcs [ARRAY_SIZE];

extern const char *Operations[12];
extern const char *LangCommands[17];
extern const char *BalletOperations[12];
extern const char *BalletLangCommands[17];

namespace Tree {
    Node *NodeInit (Node *parent = nullptr, Node *left = nullptr, Node *right = nullptr);
    int VarSearch (char *name, bool allow_to_add = false);
    int FuncSearch (char *name, bool allow_to_add = false);
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
    char *ReadHandle (FILE *readfile);
    Node *ReadTree (char *readstr, Node *parent);
    std::pair <type_t, num_t > GetNodeInfo (char *str);
    void PrintNode (FILE *writefile, Node *node);
}

#endif