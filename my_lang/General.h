#ifndef General_h
#define General_h

#include <cstdio>
#include <cstdlib>
#include <sys/stat.h>
#include <cstring>
#include <cassert>
#include <cctype>
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
#define INPUTFILE "../my_programs/program1.my_lang"
#define INPUTTREE "../my_ast/temp.ast"

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
    WHILE,
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
    OP_DIFF,
    OP_ABOVE,
    OP_ABOVE_EQUAL,
    OP_BELOW,
    OP_BELOW_EQUAL,
    OP_EQUAL,
    OP_UNEQUAL
};

struct Variable_t {
    char name [STR_LEN] = {};
    num_t val = 0;
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
extern bool return_flag;

extern size_t var_idx;
extern size_t func_idx;

extern size_t move;
extern size_t delta;

extern Variable_t vars [ARRAY_SIZE];
extern Function_t funcs [ARRAY_SIZE];

extern const char *Operations[12];
extern const char *LangCommands[17];

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
    void PrintTree (Node *node);
    void PrintNode (FILE *writefile, Node *node);
}

#endif