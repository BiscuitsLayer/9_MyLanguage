#include "my_frontend/Frontend.h"
#include "my_middleend/Middleend.h"
#include "my_backend/Backend.h"

//TODO Прогнать через валгринд, убедиться в том, что пустые узлы освобождаются
//TODO Дописать в грамматику функции (математические)
//TODO Файл с синтаксисом
//TODO Начать делать бекэнд
//TODO Добавить операций для дифференциатора
//TODO Отличать глобальные переменные от локальных
//TODO Проверку чтобы имя функции не начиналось с $ а имя переменной deriv

int main () {
    AssemblerMain ();
    CPUMain();
    return 0;
}
