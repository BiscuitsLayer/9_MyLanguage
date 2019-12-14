#pragma once
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include "StackLib.h"

#define _NAME_OF_ARG_(ARG) #ARG
#define RE std::runtime_error
#define ERR_RETURN exit (1)

FILE *console_log = fopen ("console_log.txt", "w");

//! \brief Инициализирует стек с начальным размером DELTA_2 и данными типа Elem_t
//! \return копия созданного стека

Stack_t StackInit () {
    Stack_t stk = {};
    stk.size = 0, stk.maxsize = DELTA_2, stk.dump_count = 0;
    stk.data = (Elem_t *) calloc (2 * PK_LEN + DELTA_2, sizeof (Elem_t));
    stk.pk1 = (Parkour_t *) (stk.data), *stk.pk1 = 0xDEADBEEF;
    stk.pk2 = (Parkour_t *) (stk.data + PK_LEN + stk.maxsize), *stk.pk2 = 0xBEEFDEAD;
    stk.data += PK_LEN;
    StackFillWithPoison (&stk, 0, DELTA_2 - 1);
    stk.hashcode = Hash (&stk);
    try {
        StackOK (&stk);
    } catch (RE error) {
        StackDump (&stk, error.what (), LOCATION);
        ERR_RETURN;
    }
    StackDump (&stk, "Stack Initialized", LOCATION);
    return stk;
}

void StackFillWithPoison (Stack_t *stk, size_t begin, size_t end) {
    for (size_t i = begin; i <= end; ++i)
        *(stk->data + i) = POISON;
}

//! \brief Кладёт элемент типа Elem_t в конец стека
//! \param stk Стек
//! \param value Элемент, который кладётся в стек

void StackPush (Stack_t *stk, Elem_t value) {
    try {
        StackOK (stk);

        stk->data [stk->size] = value;
        stk->size++;

        stk->hashcode = Hash (stk);

        if (stk->maxsize - stk->size < DELTA_1)
            StackResize (stk, DELTA_2);

        StackOK (stk);
    } catch (RE error) {
        StackDump (stk, error.what (), LOCATION);
        ERR_RETURN;
    }
    StackDump (stk, "Element Pushed", LOCATION);
}

//! \brief Достаёт элемент типа Elem_t из конца стека
//! \param stk Стек
//! \return Значение последнего элемента стека

Elem_t StackPop (Stack_t *stk) {
    Elem_t ans = 0;
    try {
        stk->size--;
        StackOK (stk);
        ans = stk->data [stk->size];
        StackFillWithPoison (stk, stk->size, stk->size);
        stk->hashcode = Hash (stk);

        if (stk->maxsize - stk->size >= 2 * DELTA_2)
            StackResize (stk, -DELTA_2);

        StackOK (stk);
    } catch (RE error) {
        StackDump (stk, error.what (), LOCATION);
        ERR_RETURN;
    }
    StackDump (stk, "Element Popped", LOCATION);
    return ans;
}

void StackResize (Stack_t *stk, size_t delta_size) {
    Elem_t *new_data = (Elem_t *) calloc (stk->maxsize + 2 * PK_LEN + delta_size, sizeof(Elem_t));

    if (new_data != nullptr) {
        stk->maxsize += delta_size;
        memmove(new_data + PK_LEN, stk->data, stk->maxsize * sizeof(Elem_t));

        stk->data = new_data + PK_LEN;
        StackFillWithPoison (stk, stk->size, stk->maxsize);

        stk->pk1 = (Parkour_t *) (new_data), *stk->pk1 = 0xDEADBEEF;
        stk->pk2 = (Parkour_t *) (new_data + PK_LEN + stk->maxsize), *stk->pk2 = 0xBEEFDEAD;
        stk->hashcode = Hash(stk);
    }
}

//! \brief Возвращает размер стека
//! \param stk Стек
//! \return Размер стека

size_t StackGetSize (Stack_t *stk) {
    try {
        StackOK(stk);
    } catch (RE error) {
        StackDump (stk, error.what (), LOCATION);
        ERR_RETURN;
    }
    return stk->size;
}

//! \brief Уничтожает стек
//! \param stk Стек

void StackDestruct (Stack_t *stk) {
    try {
        StackOK (stk);
    } catch (RE error) {
        StackDump (stk, error.what (), LOCATION);
        ERR_RETURN;
    }
    stk->size = 0;
    free (stk->data - PK_LEN);
    free (stk);
}

//! \brief Находит хэш - сумму элементов стека
//! \param stk Стек
//! \return Хэш - сумма

unsigned int Hash (Stack_t *stk) {
    char *key = (char *) (stk->pk1);
    unsigned int len = (stk->pk2 - stk->pk1) * sizeof (char);

    const unsigned int m = 0x5bd1e995;
    const unsigned int seed = 0;
    const int r = 24;

    unsigned int h = seed ^ len;

    const unsigned char * data = (const unsigned char *) key;
    unsigned int k = 0;

    while (len >= 4) {
        k  = data [0];
        k |= data [1] << 8;
        k |= data [2] << 16;
        k |= data [3] << 24;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    switch (len) {
        case 3:
            h ^= data [2] << 16;
        case 2:
            h ^= data [1] << 8;
        case 1:
            h ^= data [0];
            h *= m;
    };

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}

//! \brief Дамп стека
//! \param stk Стек
//! \param reason Причина дампа
//! \param file Название файла вызова
//! \param line Номер строки вызова
//! \param func Название функции вызова

void StackDump (Stack_t *stk, const char *reason, const char *file, int line, const char *func) {
    if (stk == nullptr) {
        fprintf(console_log, "Dump #%d (%s) from %s (%d) %s:\n", 0, reason, file, line, func);
        return;
    }
    stk->dump_count++;
    fprintf (console_log, "Dump #%d (%s) from %s (%d) %s:\n", stk->dump_count, reason, file, line, func);
    fprintf (console_log, "Stack_t [%s] {\n", _NAME_OF_ARG_ (stk));
    fprintf (console_log, "\tsize = %d;\n", stk->size);
    fprintf (console_log, "\tmaxsize = %d;\n", stk->maxsize);
    if (stk->pk1 == nullptr)
        fprintf (console_log, "\tpk1 = NULL;\n");
    else
        fprintf (console_log, "\tpk1 = %x;\n", *stk->pk1);
    fprintf (console_log, "\tdata [%d] {\n", stk->maxsize);
    for (int i = 0; i < stk->maxsize; ++i) {
        fprintf (console_log, i < stk->size ? "\t\t* " : "\t\t  ");
        fprintf (console_log, DATA_OUTPUT_FORMAT, i, (stk->data)[i]);
    }
    if (stk->pk2 == nullptr)
        fprintf (console_log, "\tpk2 = NULL;\n");
    else
        fprintf (console_log, "\t}\n\tpk2 = %x;\n}", *stk->pk2);
    fprintf (console_log, "\n\n");
}

//! \brief Проверка стека на корректность
//! \param stk Стек
//! \return Пройдена ли проверка


void StackOK (Stack_t *stk) {
    if (stk == nullptr)           throw RE ("ERROR: Stack pointer equals NULL");
    if (stk->size < 0)            throw RE ("ERROR: Stack index is less than 0");
    if (stk->maxsize < 0)         throw RE ("ERROR: Stack size is less than 0");
    if (stk->maxsize < stk->size) throw RE ("ERROR: Stack index is greater than stack size");
    if (stk->dump_count < 0)      throw RE ("ERROR: Stack dump counter is less than 0");
    if (stk->data == nullptr)     throw RE ("ERROR: Stack data pointer equals NULL");
    if (stk->pk1 == nullptr) throw RE ("ERROR: Stack's first canary pointer equals NULL");
    if (stk->pk2 == nullptr) throw RE ("ERROR: Stack's second canary pointer equals NULL");
    if (*stk->pk1 != 0xDEADBEEF) throw RE ("ERROR: Stack's first canary value has been changed");
    if (*stk->pk2 != 0xBEEFDEAD) throw RE ("ERROR: Stack's second canary value has been changed");
    if (Hash (stk) != stk->hashcode) throw RE ("ERROR: Stack's hash value has been changed");
}

//! \brief Дамп указателей стека (для разработчика)
//! \param stk Стек
//! \param reason Причина дампа
//! \param file Название файла вызова
//! \param line Номер строки вызова
//! \param func Название функции вызова

void StackPtrDump (Stack_t *stk, const char *reason, const char *file, int line, const char *func) {
    stk->dump_count++;
    fprintf (console_log, "PtrDump #%d (%s) from %s (%d) %s:\n", stk->dump_count, reason, file, line, func);
    fprintf (console_log, "Stack_t [%p] {\n", stk);
    fprintf (console_log, "\tsize = %p;\n", &stk->size);
    fprintf (console_log, "\tmaxsize = %p;\n", &stk->maxsize);
    fprintf (console_log, "\tpk1 = %p;\n", stk->pk1);
    fprintf (console_log, "\tdata [%p] {\n", stk->data);
    for (int i = 0; i < stk->maxsize; ++i) {
        fprintf (console_log, i < stk->size ? "\t\t* " : "\t\t  ");
        fprintf (console_log, "[%d] = %p;\n", i, (stk->data) + i);
    }
    fprintf (console_log, "\t}\n\tpk2 = %p;\n}", stk->pk2);
    fprintf (console_log, "\n\n");
}