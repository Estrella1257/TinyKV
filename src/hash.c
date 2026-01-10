#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tinykv.h"

#define INITIAL_SIZE 32

static unsigned long hash_function(const char *str) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash =((hash << 5) +hash) + c;
    }
    return hash;
}

HashTable* ht_create() {
    HashTable *ht = malloc(sizeof(HashTable));
    if(!ht) return NULL;

    ht->size = INITIAL_SIZE;
    ht->count = 0;
    ht->buckets = calloc(ht->size, sizeof(Entry*));
    if(!ht->buckets) {
        free(ht);
        return NULL;
    }
    return ht;
}

void ht_set(HashTable *ht,char *key, TinyObj *value) {
    if (!ht  || !key || !value) return;

    unsigned long index = hash_function(key) % ht->size;
    Entry *current = ht->buckets[index];

    while (current != NULL) {
        if (strcmp(current->key, key) == 0)
        {
            obj_free(current->value);
            current->value = value;
            return;
        }
        current = current->next;
    }

    Entry *new_entry = malloc(sizeof(Entry));
    if(!new_entry) return;

    new_entry->key = strdup(key);
    new_entry->value = value;

    new_entry->next = ht->buckets[index];
    ht->buckets[index] = new_entry;

    ht->count++;
}

TinyObj *ht_get(HashTable *ht,char *key) {
    if (!ht || !key) return NULL;
    unsigned long index = hash_function(key) % ht->size;
    Entry *entry = ht->buckets[index];

    while (entry != NULL) {
        if(strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }
    return NULL;
}

void ht_free(HashTable * ht) {
    if (!ht) return;

    for (size_t i = 0; i < ht->size; i++) {
        Entry *entry = ht->buckets[i];
        while (entry != NULL) {
            Entry *next = entry->next;

            free(entry->key);
            obj_free(entry->value);
            free(entry);

            entry = next;
        }
    } 
    free(ht->buckets);
    free(ht);
}

void ht_dump(HashTable *ht) {
    printf("--- Hash Table Dump ---\n");
    for (size_t i = 0; i < ht->size; i++) {
        Entry *e = ht->buckets[i];
        if(e) {
            printf("[%ld]: ", i);
            while (e) {
                printf("%s -> ", e->key);
                e = e->next;
            }
            printf("NULL\n");
        }
    }
}