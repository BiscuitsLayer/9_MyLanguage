#ifndef Middleend_h
#define Middleend_h

#include "../my_lang/General.h"

namespace Diff {
    Node &Copy (Node *node);
    Node &Diff (Node *node);
}

namespace Optimize {
    Node *MulZero (Node *node);
    Node *SumZero (Node *node);
    Node *DivZero (Node *node);
    Node *PowZero (Node *node);
    Node *MulUnit (Node *node);
    Node *DivUnit (Node *node);
    Node *PowUnit (Node *node);
    Node *NumSum (Node *node);
    Node *SimplePower (Node *node);
}

#endif