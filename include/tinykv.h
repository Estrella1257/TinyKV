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

typedef struct Entry {
    char *key;
    TinyObj *value;
    struct Entry *next;
} Entry;

typedef struct HashTable {
    Entry **buckets;
    size_t size;
    size_t count;
} HashTable;


TinyObj* obj_create_string(const char *str);
TinyObj* obj_create_int(int value);
void obj_free(TinyObj *obj);
void obj_print(TinyObj *obj);

HashTable* ht_create();
void ht_free(HashTable *ht);
void ht_set(HashTable *ht, char *key, TinyObj *value);
TinyObj* ht_get(HashTable *ht, char *key);
void ht_delete(HashTable *ht, char *key);
void ht_dump(HashTable *ht);

void kv_save(HashTable *ht, const char *filename);
HashTable* kv_load(const char *filename);

void wal_check_gc(HashTable *ht);
void wal_log_set(HashTable *ht, char *key, TinyObj *value);
void wal_log_del(HashTable *ht, char *key); 
void wal_recover(HashTable *ht, const char *filename);

#endif