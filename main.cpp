#include "my_frontend/Frontend.h"
#include "my_middleend/Middleend.h"
#include "my_backend/Backend.h"

//TODO Прогнать через валгринд, убедиться в том, что пустые узлы освобождаются
//TODO Дописать в грамматику функции (математические)
//TODO Файл с синтаксисом
//TODO Добавить операций для дифференциатора
//TODO Проверку чтобы имя функции не начиналось с $ а имя переменной deriv

int main () {

    FILE *readfile = fopen (INPUTFILE, "rb");
    if (!readfile) {
        printf ("Error opening file\n");
        return 0;
    }
    Elem_t *tokens = Tokens::Tokenization (readfile);
        Tree::VarDump ();
    fclose (readfile);
    Node *root = RD::GetG (tokens);
        Tree::VarDump ();
    root = Optimize::Differentiator (root);
    root = Optimize::Optimizer (root);
    ASM::TreeToASM (root);

    /* //Если надо считать дерево с файла INPUTTREE
    FILE *readfile = fopen (INPUTTREE, "rb");
    if (!readfile) {
        printf ("Error opening file\n");
        return 0;
    }
    Node *root = Tree::NodeInit ();
    root->parent = Tree::NodeInit (nullptr, root);
    root = AST::ReadTree (AST::ReadHandle(readfile), root->parent);
    */

    Dot::PrintTree (root);
    AST::PrintTree (root);
    Tree::FreeNode (root);
    return 0;
}
