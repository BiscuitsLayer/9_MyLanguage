#include "Middleend.h"

Node &Diff::Copy (Node *node) {
    Node *new_node = Tree::NodeInit (node->parent);
    new_node->type = node->type;
    if (node->left)
        new_node->left = &Copy (node->left);
    if (node->right)
        new_node->right = &Copy (node->right);
    new_node->level = node->level;
    return *new_node;
}

#define OPERATOR_OVERLOAD(SYM, OP) Node &operator SYM (Node &left, Node &right) { \
    Node *node = nullptr;                                                         \
    if (left.parent == nullptr) {                                                 \
        node = Tree::NodeInit ();                                                 \
        node->type = TYPE_OP;                                                     \
        node->data = OP;                                                          \
        left.parent = node;                                                       \
        right.parent = node;                                                      \
    }                                                                             \
    else                                                                          \
        node = Tree::NodeInit (left.parent->parent);                              \
        node->type = TYPE_OP;                                                     \
        node->data = OP;                                                          \
    node->level = left.level - 1;                                                 \
    node->left = &left;                                                           \
    node->right = &right;                                                         \
    return *node;                                                                 \
}

OPERATOR_OVERLOAD (+, OP_SUM)
OPERATOR_OVERLOAD (-, OP_SUB)
OPERATOR_OVERLOAD (*, OP_MUL)
OPERATOR_OVERLOAD (/, OP_DIV)

Node &Diff::Diff (Node *node) {
#define c(node) Copy (node)
#define d(node) Diff (node)
#define L node->left
#define R node->right

    Node *NUM_0 = Tree::NodeInit (node->parent);
    NUM_0->type = TYPE_NUM;
    NUM_0->data = 0;

    Node *NUM_1 = Tree::NodeInit (node->parent);
    NUM_1->type = TYPE_NUM;
    NUM_1->data = 1;

    Node *NUM_2 = Tree::NodeInit (node->parent);
    NUM_2->type = TYPE_NUM;
    NUM_2->data = 2;

    Node *HALF = Tree::NodeInit (node->parent);
    HALF->type = TYPE_NUM;
    HALF->data = 0.5;

    Node *SQR = Tree::NodeInit (node->parent, &c(L), NUM_2);
    HALF->type = TYPE_NUM;
    HALF->data = OP_POW;

#define OP(name) Tree::NodeInit(node, OP_##name, TYPE_OP, &c(L))

    Node *new_node = nullptr;
    switch (node->type) {
        case TYPE_NUM: {
            new_node = &Diff::Copy (NUM_0);
            return *new_node;
        }
        case TYPE_VAR: {
            new_node = &Diff::Copy (NUM_1);
            return *new_node;
        }
        case TYPE_CONST: {
            new_node = &Diff::Copy (NUM_0);
            return *new_node;
        }
        case TYPE_OP:
            switch ((int)node->data) {
                case OP_SUM: {
                    new_node = &(d(L) + d(R));
                    return *new_node;
                }
                case OP_SUB: {
                    new_node = &(d(L) - d(R));
                    return *new_node;
                }
                case OP_MUL: {
                    new_node = &(d(L) * c(R) + d(R) * c(L));
                    return *new_node;
                }
                case OP_DIV: {
                    new_node = &((d(L) * c(R) - d(R) * c(L)) / (c(R) * c(R)));
                    return *new_node;
                }
                /*
                case OP_SIN: return (*OP(COS) * d(L));
                case OP_COS: return ((NUM(-1) * *OP(SIN)) * d(L));
                case OP_TG: return ((NUM(1) / (*NodeInit(node, OP_POW2, TYPE_OP, OP(COS), &NUM(2)))) * d(L));
                case OP_CTG: return ((NUM(-1) / (*NodeInit(node, OP_POW2, TYPE_OP, OP(SIN), &NUM(2)))) * d(L));
                case OP_SH: return (*OP(CH) * d(L));
                case OP_CH: return (*OP(SH) * d(L));
                case OP_TH: return ((NUM(1) / (*NodeInit(node, OP_POW2, TYPE_OP, OP(CH), &NUM(2)))) * d(L));
                case OP_CTH: return ((NUM(-1) / (*NodeInit(node, OP_POW2, TYPE_OP, OP(SH), &NUM(2)))) * d(L));
                case OP_POW1: return (c(node) * d(NodeInit(node, OP_MUL, TYPE_OP, &c(R), OP(LN))));
                case OP_POW2: return ((c(R) * *NodeInit(node, OP_POW2, TYPE_OP, &c(L), NodeInit (node->right, OP_SUB, TYPE_OP, &c(R), &NUM (1)))) * d(L));
                case OP_LN: {
                    new_node = &((NUM(1) / c(L)) * d(L));
                    return *new_node;
                }
                case OP_ARCSIN: return ((NUM(1) / *NodeInit(node, OP_POW2, TYPE_OP, NodeInit (node, OP_SUB, TYPE_OP, &NUM(1), SQR), HALF)) * d(L));
                case OP_ARCCOS: return ((NUM(-1) / *NodeInit(node, OP_POW2, TYPE_OP, NodeInit (node, OP_SUB, TYPE_OP, &NUM(1), SQR), HALF)) * d(L));
                case OP_ARCTG: return ((NUM(1) / *NodeInit (node, OP_SUM, TYPE_OP, &NUM(1), SQR)) * d(L));
                case OP_ARCCTG: return ((NUM(-1) / *NodeInit (node, OP_SUM, TYPE_OP, &NUM(1), SQR)) * d(L));
                case OP_ARCSH: return ((NUM(1) / *NodeInit(node, OP_POW2, TYPE_OP, NodeInit (node, OP_SUM, TYPE_OP, SQR, &NUM(1)), HALF)) * d(L));
                case OP_ARCCH: return ((NUM(1) / *NodeInit(node, OP_POW2, TYPE_OP, NodeInit (node, OP_SUB, TYPE_OP, SQR, &NUM(1)), HALF)) * d(L));
                case OP_ARCTH: return ((NUM(1) / *NodeInit (node, OP_SUB, TYPE_OP, &NUM(1), SQR)) * d(L));
                case OP_ARCCTH: return ((NUM(1) / *NodeInit (node, OP_SUB, TYPE_OP, &NUM(1), SQR)) * d(L));
                 */
            }
    }
    return *NUM_0;
#undef c
#undef d
#undef L
#undef R
}

Node *Optimize::MulZero (Node *node) {
    if (!node->parent)
        Tree::TreeOffsetCorrecter (node);
    if (node->left)
        Optimize::MulZero (node->left);
    if (node->right)
        Optimize::MulZero (node->right);
    if (node->type == TYPE_OP && node->data == OP_MUL) {
        if ((node->left->type == TYPE_NUM && node->left->data == 0) || (node->right->type == TYPE_NUM && node->right->data == 0)) {
            Node *new_node = NodeInit (node->parent, 0, TYPE_NUM, nullptr, nullptr);
            FreeNode (node->left);
            FreeNode (node->right);
            if (node->parent)
                (node->parent->left == node ? node->parent->left : node->parent->right) = new_node;
            flag = true;
            return new_node;
        }
    }
    return node;
}

Node *Optimize::SumZero (Node *node) {
    if (!node->parent)
        Tree::TreeOffsetCorrecter (node);
    if (node->left)
        Optimize::SumZero (node->left);
    if (node->right)
        Optimize::SumZero (node->right);
    if (node->type == TYPE_OP && node->data == OP_SUM) {
        if (node->left->type == TYPE_NUM && node->left->data == 0) {
            Tree::FreeNode (node->left);
            if (node->parent)
                (node->parent->left == node ? node->parent->left : node->parent->right) = node->right;
            flag = true;
            return node->right;
        }
        else if (node->right->type == TYPE_NUM && node->right->data == 0) {
            Tree::FreeNode (node->right);
            if (node->parent)
                (node->parent->left == node ? node->parent->left : node->parent->right) = node->left;
            flag = true;
            return node->left;
        }
    }
    else if (node->type == TYPE_OP && node->data == OP_SUB) { //Обработка вычитания нуля и из нуля
        if (node->left->type == TYPE_NUM && node->left->data == 0) { //Замена на -1
            Node *new_node = NodeInit (node->parent, OP_MUL, TYPE_OP, node->left, node->right);
            if (node->parent)
                (node->parent->left == node ? node->parent->left : node->parent->right) = new_node;
            new_node->left = NodeInit (node, -1, TYPE_NUM, nullptr, nullptr);
            flag = true;
            return new_node;
        }
        else if (node->right->type == TYPE_NUM && node->right->data == 0) { //Сдвиг
            Tree::FreeNode (node->right);
            if (node->parent)
                (node->parent->left == node ? node->parent->left : node->parent->right) = node->left;
            flag = true;
            return node->left;
        }
    }
    return node;
}

Node *Optimize::DivZero (Node *node) {
    if (!node->parent)
        Tree::TreeOffsetCorrecter (node);
    if (node->left)
        Optimize::DivZero (node->left);
    if (node->right)
        Optimize::DivZero (node->right);
    if (node->type == TYPE_OP && node->data == OP_DIV) {
        if (node->left->type == TYPE_NUM && node->left->data == 0) {
            Tree::FreeNode(node->right);
            if (node->parent)
                (node->parent->left == node ? node->parent->left : node->parent->right) = node->left;
            flag = true;
            return node->left;
        }
    }
    return node;
}

Node *Optimize::PowZero (Node *node) {
    if (!node->parent)
        Tree::TreeOffsetCorrecter (node);
    if (node->left)
        Optimize::PowZero (node->left);
    if (node->right)
        Optimize::PowZero (node->right);
    if (node->type == TYPE_OP && (node->data == OP_POW1 || node->data == OP_POW2)) {
        if (node->right->type == TYPE_NUM && node->right->data == 0) {
            Node *new_node = NodeInit (node->parent, 1, TYPE_NUM, nullptr, nullptr);
            Tree::FreeNode(node->left);
            Tree::FreeNode(node->right);
            if (node->parent)
                (node->parent->left == node ? node->parent->left : node->parent->right) = new_node;
            flag = true;
            return new_node;
        }
    }
    return node;
}

Node *Optimize::MulUnit (Node *node) {
    if (!node->parent)
        Tree::TreeOffsetCorrecter (node);
    if (node->left)
        Optimize::MulUnit (node->left);
    if (node->right)
        Optimize::MulUnit (node->right);
    if (node->type == TYPE_OP && node->data == OP_MUL) {
        if (node->left->type == TYPE_NUM && node->left->data == 1) {
            Tree::FreeNode (node->left);
            if (node->parent)
                (node->parent->left == node ? node->parent->left : node->parent->right) = node->right;
            flag = true;
            return node->right;
        }
        else if (node->right->type == TYPE_NUM && node->right->data == 1) {
            Tree::FreeNode (node->right);
            if (node->parent)
                (node->parent->left == node ? node->parent->left : node->parent->right) = node->left;
            flag = true;
            return node->left;
        }
    }
    return node;
}

Node *Optimize::DivUnit (Node *node) {
    if (!node->parent)
        Tree::TreeOffsetCorrecter (node);
    if (node->left)
        Optimize::DivUnit (node->left);
    if (node->right)
        Optimize::DivUnit (node->right);
    if (node->type == TYPE_OP && node->data == OP_DIV) {
        if (node->right->type == TYPE_NUM && node->right->data == 1) {
            Tree::FreeNode(node->right);
            if (node->parent)
                (node->parent->left == node ? node->parent->left : node->parent->right) = node->left;
            flag = true;
            return node->left;
        }
    }
    return node;
}

Node *Optimize::PowUnit (Node *node) {
    if (!node->parent)
        Tree::TreeOffsetCorrecter (node);
    if (node->left)
        Optimize::PowUnit (node->left);
    if (node->right)
        Optimize::PowUnit (node->right);
    if (node->type == TYPE_OP && (node->data == OP_POW1 || node->data == OP_POW2)) {
        if (node->right->type == TYPE_NUM && node->right->data == 1) {
            Tree::FreeNode(node->right);
            if (node->parent)
                (node->parent->left == node ? node->parent->left : node->parent->right) = node->left;
            flag = true;
            return node->left;
        }
    }
    return node;
}

Node *Optimize::NumSum (Node *node) {
    if (!node->parent)
        Tree::TreeOffsetCorrecter (node);
    if (node->left)
        Optimize::NumSum (node->left);
    if (node->right)
        Optimize::NumSum (node->right);
    if (false) {}
#define CALCULATE(SYM, NAME) else if (node->type == TYPE_OP && node->data == NAME) {                                       \
        if (node->left->type == TYPE_NUM && node->right->type == TYPE_NUM) {                                               \
        Node *new_node = NodeInit (node->parent, node->left->data SYM node->right->data, TYPE_NUM, nullptr, nullptr);      \
            FreeNode (node->left);                                                                                         \
            FreeNode (node->right);                                                                                        \
            if (node->parent)                                                                                              \
                (node->parent->left == node ? node->parent->left : node->parent->right) = new_node;                        \
            return (*flag = true), new_node;                                                                               \
        }                                                                                                                  \
    }
    CALCULATE(+, OP_SUM)
    CALCULATE(-, OP_SUB)
    CALCULATE(*, OP_MUL)
    CALCULATE(/, OP_DIV)
    return node;
#undef CALCULATE
}

Node *Optimize::SimplePower (Node *node) {
    if (!node->parent)
        Tree::TreeOffsetCorrecter (node);
    if (node->left)
        Optimize::SimplePower (node->left);
    if (node->right)
        Optimize::SimplePower (node->right);
    if (node->type == TYPE_OP && node->data == OP_POW1) {
        if (!TreeContainsVar (node->right, vars, &vars_cur_size))
            node->data = OP_POW2;
    }
    return node;
}