#ifndef TINYKV_H
#define TINYKV_H

#include <stddef.h>

typedef enum {
    T_STRING,
    T_INT
} TinyType;

typedef struct TinyObj
{
    TinyType type;
    void *ptr;
} TinyObj;

TinyObj* obj_create_string(const char *str);
TinyObj* obj_create_int(int value);
void obj_free(TinyObj *obj);
void obj_print(TinyObj *obj);


#endif