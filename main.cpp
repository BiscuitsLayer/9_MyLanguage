#include "my_frontend/Frontend.h"
#include "my_middleend/Middleend.h"
#include "my_backend/Backend.h"
#include "my_frontend_1/Frontend_1.h"

const char *INPUTFILE = "../my_programs/solve_sqr.my_lang";

//TODO Прогнать через валгринд, убедиться в том, что пустые узлы освобождаются
//TODO Дописать в грамматику функции (математические)
//TODO Файл с синтаксисом
//TODO Добавить операций для дифференциатора и ассемблера
//TODO Проверку чтобы имя функции не начиналось с $ а имя переменной
//TODO Комменты в моем языке
//TODO Скобки у математических выражений в фронт-1

int main (int argc, char *argv[]) {
    //assert (argc == 1);
    //if (strtod (argv[0], nullptr) == 0) {
        //Если программа считывается из файла
        FILE *readfile = fopen(INPUTFILE, "rb");
        if (!readfile) {
            printf("Error opening file\n");
            return 0;
        }
        Elem_t *tokens = Tokens::Tokenization(readfile);
        fclose(readfile);
        Node *root = RD::GetG(tokens);
        //Tree::VarDump ();
        root = Optimize::Differentiator(root);
        root = Optimize::Optimizer(root);
        Dot::PrintTree (root);
        AST::PrintTree (root);
        Lang::TreeToLang (root);
        ASM::TreeToASM(root);
        AssemblerMain ();
        CPUMain ();
        Tree::FreeNode (root);
    //}
    /*
    //else if (strtod (argv[0], nullptr) == 1) {
    //Если надо считать дерево с файла INPUTTREE
    FILE *readfile = fopen (INPUTTREE, "rb");
    if (!readfile) {
        printf("Error opening file\n");
        return 0;
    }
    Node *root = Tree::NodeInit();
    root->parent = Tree::NodeInit(nullptr, root);
    root = AST::ReadTree(AST::ReadHandle(readfile), root->parent);
    //root = Optimize::Differentiator(root);
    root = Optimize::Optimizer(root);
    ASM::TreeToASM(root);

    Dot::PrintTree (root);
    AST::PrintTree (root);
    Tree::FreeNode (root);
    //}
     */
    return 0;
}
