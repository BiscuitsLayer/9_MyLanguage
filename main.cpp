#include "my_user_interface/UserInterface.h"

//TODO Прогнать через валгринд, убедиться в том, что пустые узлы освобождаются
//TODO Дописать в грамматику функции (математические)
//TODO Файл с синтаксисом
//TODO Добавить операций для дифференциатора и ассемблера
//TODO Проверку чтобы имя функции не начиналось с $ а имя переменной
//TODO Комменты в моем языке
//TODO Скобки у математических выражений в фронт-1
//TODO Токенизация из своего языка
//TODO Перегруппировать папки
//TODO Скинуть системные файлы в отдельную папку
//TODO Допилить видосики
//TODO return со значением и без

int main () {
	UserInterfaceMain ();
/*  FOR DEBUG   */
/*
	Node *root = nullptr;
	char *filepath = (char *) calloc (STR_LEN, sizeof (char));
	sprintf (filepath, "../my_programs/globals.my_lang", filename);
	FILE *readfile = fopen(filepath, "rb");
	if (!readfile) {
		printf("Error opening file\n");
		return 0;
	}
	Tree::FreeNode (root);
	Elem_t *tokens = Tokens::Tokenization (readfile, false);
	fclose(readfile);
	root = RD::GetG(tokens);

	printf ("Read successfully\n\n");
	root = Optimize::Differentiator (root);
	root = Optimize::Optimizer (root);
	Dot::PrintTree (root);
	printf ("Handled successfully\n\n");

	//printf ("Turning on ASM...\n");
	ASM::TreeToASM (root);
	//AssemblerMain ();
	//CPUMain ();
	//printf ("Turning off ASM...\nSuccess\n\n");

	printf ("Turning on ELF Generator...\n");
	ELF::CreateELF (root);
	printf ("Turning off ELF Generator...\nSuccess\n\n");

	free (filepath);
	Tree::FreeNode (root);
*/
    return 0;
}
