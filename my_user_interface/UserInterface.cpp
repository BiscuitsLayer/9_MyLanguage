#include "UserInterface.h"

//TODO УБЕРИ ВОТ ЭТО ВОТ СРОЧНО
//char filename[STR_LEN];
char filename[STR_LEN] = "test";
//TODO УБЕРИ ВОТ ЭТО ВОТ СРОЧНО

void Ask () {
	printf ("Select mode:\n");
	printf ("0. Exit\n"
			"1. Import program in Ballet Language\n"
	        "2. Import program in AST Language\n"
	        "3. Import tree\n"
	        "4. Handle data\n"
	        "5. Save in Ballet Language\n"
	        "6. Save in AST Language\n"
	        "7. Save as tree\n"
	        "8. Send to ASM\n");
}

void UserInterfaceMain () {
	Node *root = nullptr;
	char *filepath = (char *) calloc (STR_LEN, sizeof (char));
	double ans = 0;
	bool is_handled = false;
	bool is_loaded = false;
	printf ("Hello! This is Ballet Language Compiler!\n");
	Ask ();
	scanf ("%lg", &ans);
	while (ans != 0) {
		switch ((int)ans) {
			case 1: {
				printf ("Input filename:\n");
				scanf ("%s", filename);
				sprintf (filepath, "../my_programs/%s.ballet_lang", filename);
				FILE *readfile = fopen (filepath, "rb");
				if (!readfile) {
					printf("Error opening file\n");
					return;
				}
				Tree::FreeNode (root);
				Elem_t *tokens = Tokens::Tokenization (readfile, true);
				fclose(readfile);
				root = BalletRD::BalletGetG(tokens);
				is_loaded = true;
				is_handled = false;
				printf ("Read successfully\n\n");
				break;
			}
			case 2: {
				printf ("Input filename:\n");
				scanf ("%s", filename);
				sprintf (filepath, "../my_programs/%s.my_lang", filename);
				FILE *readfile = fopen(filepath, "rb");
				if (!readfile) {
					printf("Error opening file\n");
					return;
				}
				Tree::FreeNode (root);
				Elem_t *tokens = Tokens::Tokenization (readfile, false);
				fclose(readfile);
				root = RD::GetG(tokens);
				is_loaded = true;
				is_handled = false;
				printf ("Read successfully\n\n");
				break;
			}
			case 3: {
				printf ("Input filename:\n");
				scanf ("%s", filename);
				sprintf (filepath, "../my_ast/%s.ast", filename);
				FILE *readfile = fopen (filepath, "rb");
				if (!readfile) {
					printf("Error opening file\n");
					return;
				}
				Tree::FreeNode (root);
				root = Tree::NodeInit();
				root->parent = Tree::NodeInit(nullptr, root);
				root = AST::ReadTree(AST::ReadHandle(readfile), root->parent);
				is_loaded = true;
				is_handled = false;
				printf ("Read successfully\n\n");
				break;
			}
			case 4: {
				if (!is_loaded)
					printf ("You can't handle program until you load it\n");
				else {
					root = Optimize::Differentiator (root);
					root = Optimize::Optimizer (root);
					Dot::PrintTree (root);
					is_handled = true;
					printf ("Handled successfully\n\n");
				}
				break;
			}
			case 5: {
				if (!is_loaded)
					printf ("You can't save program until you load it\n");
				else {
					printf ("Output filename:\n");
					scanf ("%s", filename);
					sprintf (filepath, "../my_programs/%s.ballet_lang", filename);
					FILE *writefile = fopen (filepath, "w");
					BalletLang::NodeToLang (writefile, root);
					fclose (writefile);
					printf ("Saved successfully\n\n");
				}
				break;
			}
			case 6: {
				if (!is_loaded)
					printf ("You can't save program until you load it\n");
				else {
					printf ("Output filename:\n");
					scanf ("%s", filename);
					sprintf (filepath, "../my_programs/%s.my_lang", filename);
					FILE *writefile = fopen (filepath, "w");
					Lang::NodeToLang (writefile, root);
					fclose (writefile);
					printf ("Saved successfully\n\n");
				}
				break;
			}
			case 7: {
				if (!is_loaded)
					printf ("You can't save program until you load it\n");
				else {
					printf ("Output filename:\n");
					scanf ("%s", filename);
					sprintf (filepath, "../my_ast/%s.ast", filename);
					FILE *writefile = fopen (filepath, "w");
					AST::PrintNode (writefile, root);
					fclose (writefile);
					printf ("Saved successfully\n\n");
				}
				break;
			}
			case 8: {
				if (!is_loaded)
					printf ("You can't send program to ASM until you load it\n");
				else if (!is_handled)
					printf ("You can't send program to ASM until you handle it\n");
				else {
					printf ("Turning on ASM...\n");
					ASM::TreeToASM (root);
					AssemblerMain ();
					CPUMain ();
					printf ("Turning off ASM...\nSuccess\n\n");
				}
				break;
			}
		}
		Ask ();
		scanf ("%lg", &ans);
	}
	free (filepath);
	Tree::FreeNode (root);
}