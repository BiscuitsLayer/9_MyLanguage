#pragma once
#include <cstddef>

const size_t LABEL_ARRAY_SIZE = 5;
const size_t LABEL_STR_LEN = 10; //Длина строки "labelNUM"
const size_t LABEL_WORD_LEN = 5; //Длина слова LABEL

struct Label_t {
    char *name = nullptr;
    size_t num = -1;
};