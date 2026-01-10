#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tinykv.h"

static TinyObj* make_obj(TinyType type) {
    TinyObj *obj = (TinyObj*)malloc(sizeof(TinyObj));
    if (obj == NULL) {
        perror("Failed to allocate TinyObj");
        return NULL;
    }

    obj->type = type;
    obj->ptr = NULL;

    return obj;
}

TinyObj* obj_create_string(const char* str) {
    if (str == NULL) return NULL;

    TinyObj *obj = make_obj(T_STRING);
    if (!obj) return NULL;

    size_t len = strlen(str);
    obj->ptr = malloc(len + 1);

    if (obj->ptr == NULL) {
        free(obj);
        return NULL;
    }

    strcpy((char *)obj->ptr, str);

    return obj;
}

TinyObj* obj_create_int(int value) {
    TinyObj *obj = make_obj(T_INT);
    if (!obj) return NULL;

    obj->ptr = malloc(sizeof(int));

    if (obj->ptr == NULL) {
        free(obj);
        return NULL;
    }

    *(int *)(obj->ptr) = value;

    return obj;
}

void obj_free(TinyObj *obj) {
    if (!obj) return;

    if (obj->ptr != NULL) {
        free(obj->ptr);
    }
    free(obj);
}

void obj_print(TinyObj *obj) {
    if (!obj) {
        printf("(NULL)\n");
        return;
    }

    if (obj->type == T_STRING) {
        printf("[String] Value: %s\n", (char *)obj->ptr);
    }
    else if (obj->type == T_INT) {
        printf("[Int] Value: %d\n", *(int *)obj->ptr);
    }
    else {
        printf("[Uknown] Type\n");
    }
}