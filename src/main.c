#include <stdio.h>
#include "tinykv.h"

int main() {
    TinyObj *s = obj_create_string("Hello, TinyKV");
    TinyObj *n = obj_create_int(1024);

    obj_print(s);
    obj_print(n);

    obj_free(s);
    obj_free(n);

    return 0;
}