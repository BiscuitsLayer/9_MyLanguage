#ifndef Frontend_h
#define Frontend_h

#include "../my_lang/General.h"

namespace Tokens {
    Elem_t *Tokenization (FILE *readfile);
    void TokenHandle (Elem_t *tokens, size_t pass);
}

//Рекурсивный спуск
namespace RD {
    Node *GetG (Elem_t *tokens);
    Node *GetF (Elem_t *tokens);
    Node *GetFArgs (Elem_t *tokens);
    Node *GetAs (Elem_t *tokens);
    Node *GetID (Elem_t *tokens);
    Node *GetE (Elem_t *tokens);
    Node *GetT (Elem_t *tokens);
    Node *GetS (Elem_t *tokens);
    Node *GetP (Elem_t *tokens);
    Node *GetN (Elem_t *tokens);
    Node *GetOp (Elem_t *tokens);
    Node *GetReturn (Elem_t *tokens);
    Node *GetDeriv (Elem_t *tokens);
    Node *GetFCall (Elem_t *tokens);
    Node *GetFCallArgs (Elem_t *tokens, size_t var_count);
    Node *GetIf (Elem_t *tokens);
    Node *GetWhile (Elem_t *tokens);
    Node *GetC (Elem_t *tokens);
    Node *GetPut (Elem_t *tokens);
    Node *GetGet (Elem_t *tokens);
}

#endif