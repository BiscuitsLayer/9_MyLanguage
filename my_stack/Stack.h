#pragma once
#include <cstdio>

#define LOCATION __FILE__, __LINE__, __PRETTY_FUNCTION__
#define DATA_OUTPUT_FORMAT "[%d] = %d;\n"
#define POISON 666

//#define console_log stdout

typedef long long int Parkour_t;
typedef long long int Elem_t;
typedef struct {
    int size = 0, maxsize = 0, dump_count = 0;
    Parkour_t *pk1 = nullptr;
    Elem_t *data = nullptr;
    Parkour_t *pk2 = nullptr;
    unsigned int hashcode = 0;
} Stack_t;

const int DELTA_1 = 1, DELTA_2 = 5, PK_LEN = sizeof (Parkour_t) / sizeof (Elem_t);

Stack_t StackInit ();
void StackFillWithPoison (Stack_t *stk, size_t begin, size_t end);
void StackPush (Stack_t *stk, Elem_t value);
Elem_t StackPop (Stack_t *stk);
size_t StackGetSize (Stack_t *stk);
void StackResize (Stack_t *stk, size_t delta_size);
void StackDestruct (Stack_t *stk);
unsigned int Hash (Stack_t *stk);
void StackDump (Stack_t *stk, const char *reason, const char *s1, int s2, const char *s3);
void StackOK (Stack_t *stk);
void StackPtrDump (Stack_t *stk, const char *reason, const char *file, int line, const char *func);