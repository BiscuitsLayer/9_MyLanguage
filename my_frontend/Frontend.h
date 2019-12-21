#ifndef Frontend_h
#define Frontend_h

#include "../my_general/General.h"

namespace Tokens {
    Elem_t *Tokenization (FILE *readfile, bool is_ballet = false);
	void BalletTokenHandle (Elem_t *tokens, size_t pass);
    void TokenHandle (Elem_t *tokens, size_t pass);
}

//Рекурсивный спуск
namespace RD {
    Node *GetG (Elem_t *tokens, bool is_ballet = false);
    Node *GetF (Elem_t *tokens, bool is_ballet = false);
    Node *GetFArgs (Elem_t *tokens, bool is_ballet = false);
    Node *GetAs (Elem_t *tokens, bool is_ballet = false);
    Node *GetID (Elem_t *tokens, bool is_ballet = false);
    Node *GetE (Elem_t *tokens, bool is_ballet = false);
    Node *GetT (Elem_t *tokens, bool is_ballet = false);
    Node *GetS (Elem_t *tokens, bool is_ballet = false);
    Node *GetP (Elem_t *tokens, bool is_ballet = false);
    Node *GetN (Elem_t *tokens, bool is_ballet = false);
    Node *GetOp (Elem_t *tokens, bool is_ballet = false);
    Node *GetReturn (Elem_t *tokens, bool is_ballet = false);
    Node *GetDeriv (Elem_t *tokens, bool is_ballet = false);
    Node *GetFCall (Elem_t *tokens, bool is_ballet = false);
    Node *GetFCallArgs (Elem_t *tokens, size_t var_count, bool is_ballet = false);
    Node *GetIf (Elem_t *tokens, bool is_ballet = false);
    Node *GetWhile (Elem_t *tokens, bool is_ballet = false);
    Node *GetC (Elem_t *tokens, bool is_ballet = false);
    Node *GetPut (Elem_t *tokens, bool is_ballet = false);
    Node *GetGet (Elem_t *tokens, bool is_ballet = false);
}

namespace BalletRD {
	Node *BalletGetG (Elem_t *tokens, bool is_ballet = false);
	Node *BalletGetF (Elem_t *tokens, bool is_ballet = false);
	Node *BalletGetFArgs (Elem_t *tokens, bool is_ballet = false);
	Node *BalletGetAs (Elem_t *tokens, bool is_ballet = false);
	Node *BalletGetID (Elem_t *tokens, bool is_ballet = false);
	Node *BalletGetE (Elem_t *tokens, bool is_ballet = false);
	Node *BalletGetT (Elem_t *tokens, bool is_ballet = false);
	Node *BalletGetS (Elem_t *tokens, bool is_ballet = false);
	Node *BalletGetP (Elem_t *tokens, bool is_ballet = false);
	Node *BalletGetN (Elem_t *tokens, bool is_ballet = false);
	Node *BalletGetOp (Elem_t *tokens, bool is_ballet = false);
	Node *BalletGetReturn (Elem_t *tokens, bool is_ballet = false);
	Node *BalletGetDeriv (Elem_t *tokens, bool is_ballet = false);
	Node *BalletGetFCall (Elem_t *tokens, bool is_ballet = false);
	Node *BalletGetFCallArgs (Elem_t *tokens, size_t var_count, bool is_ballet = false);
	Node *BalletGetIf (Elem_t *tokens, bool is_ballet = false);
	Node *BalletGetWhile (Elem_t *tokens, bool is_ballet = false);
	Node *BalletGetC (Elem_t *tokens, bool is_ballet = false);
	Node *BalletGetPut (Elem_t *tokens, bool is_ballet = false);
	Node *BalletGetGet (Elem_t *tokens, bool is_ballet = false);
}

#endif