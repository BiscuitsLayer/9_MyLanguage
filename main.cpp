#include "my_frontend/Frontend.h"
#include "my_middleend/Middleend.h"

//TODO Прогнать через валгринд, убедиться в том, что пустые узлы освобождаются
//TODO Дописать в грамматику функции (математические)
//TODO Добавить [] в процессор
//TODO Сделать константы, наконец, неизменными!
//TODO Файл с синтаксисом
//TODO Начать делать бекэнд
//TODO Форму узлов
//TODO Добавить операций для дифференциатора
//TODO Придумать как на этом этапе отличать глобальные переменные от локальных

int main () {
    FILE *readfile = fopen (INPUTFILE, "rb");
    if (!readfile) {
        printf ("Error opening file\n");
        return 0;
    }

    Elem_t *tokens = Tokens::Tokenization (readfile);

    Node *root = RD::GetG (tokens);
    root = Optimize::Differentiator (root);
    root = Optimize::Optimizer (root);
    Dot::PrintTree (root);
    AST::PrintTree (root);
    Tree::FreeNode (root);
    return 0;
}
