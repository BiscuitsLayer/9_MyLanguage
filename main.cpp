#include "my_frontend/Frontend.h"
#include "my_middleend/Middleend.h"

//TODO Прогнать через валгринд, убедиться в том, что пустые узлы освобождаются
//TODO Дописать в грамматику функции (математические)
//TODO Добавить [] в процессор
//TODO Файл с синтаксисом
//TODO Начать делать бекэнд
//TODO Добавить операций для дифференциатора
//TODO Отличать глобальные переменные от локальных
//TODO Проверку чтобы имя функции не начиналось с $ а имя переменной deriv

int main () {
    /*
    FILE *readfile = fopen (INPUTFILE, "rb");
    if (!readfile) {
        printf ("Error opening file\n");
        return 0;
    }

    Elem_t *tokens = Tokens::Tokenization (readfile);
    fclose (readfile);
    Node *root = RD::GetG (tokens);
    root = Optimize::Differentiator (root);
    root = Optimize::Optimizer (root);
    */

    FILE *readfile = fopen (INPUTTREE, "rb");
    if (!readfile) {
        printf ("Error opening file\n");
        return 0;
    }
    Node *root = Tree::NodeInit ();
    root->parent = Tree::NodeInit (nullptr, root);
    root = AST::ReadTree (readfile, root->parent);

    Dot::PrintTree (root);
    //AST::PrintTree (root);
    Tree::FreeNode (root);
    return 0;
}
