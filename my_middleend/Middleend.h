#ifndef Middleend_h
#define Middleend_h

#include "../my_general/General.h"

namespace Diff {
    Node &Copy (Node *node);
    Node &Diff (Node *node);
    Node *PreHandle (Node *node, size_t diff_var = 0);
    Node *PostHandle (Node *node, size_t diff_var = 0);
}

namespace Optimize {
    Node *Optimizer (Node *node);
    Node *Differentiator (Node *node);
    Node *MulZero (Node *node);
    Node *SumZero (Node *node);
    Node *DivZero (Node *node);
    Node *PowZero (Node *node);
    Node *MulUnit (Node *node);
    Node *DivUnit (Node *node);
    Node *PowUnit (Node *node);
    Node *NumSum (Node *node);
    //Node *SimplePower (Node *node);
}

#endif