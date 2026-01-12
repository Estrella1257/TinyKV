#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tinykv.h"

#define MAGIC_HEADER "TKV1"

void kv_save(HashTable *ht, const char *filename) {
    FILE *fp = fopen(filename, "wb");
    if (!fp) {
        perror("Failed to open file for wiriting");
        return;
    }

    fwrite(MAGIC_HEADER, 4, 1, fp);

    for (size_t i = 0; i < ht->size; i++) {
        Entry *entry = ht->buckets[i];

        while (entry) {
            unsigned char type = (unsigned char)entry->value->type;
            fwrite(&type, sizeof(unsigned char), 1, fp);

            size_t key_len = strlen(entry->key);
            fwrite(&key_len, sizeof(size_t), 1, fp);
            fwrite(entry->key, key_len, 1, fp);

            if (entry->value->type == T_INT) {
                int val = *(int *)entry->value->ptr;
                fwrite(&val, sizeof(int), 1, fp);
            }
            else if (entry->value->type == T_STRING) {
                char *str = (char *)entry->value->ptr;
                size_t val_len = strlen(str);
                fwrite(&val_len, sizeof(size_t), 1, fp);
                fwrite(str, val_len, 1, fp);
            }
            entry = entry->next;
        }
    }
    fclose(fp);
    printf("[Persistence] Saved to %s\n", filename);
}

HashTable* kv_load(const char *filename) {
    FILE *fp = fopen(filename, "rb");
    if (!fp) return NULL;

    char magic[5] = {0};
    if (fread(magic, 4, 1, fp) != 1 || strcmp(magic, MAGIC_HEADER) != 0) {
        printf("Invalid file format\n");
        fclose(fp);
        return NULL;
    }

    HashTable *ht = ht_create();

    while (1) {
        unsigned char type_u8;
        if (fread(&type_u8, sizeof(unsigned char), 1 ,fp) != 1) break;
        TinyType type = (TinyType)type_u8;
        
        size_t key_len;
        fread(&key_len, sizeof(size_t), 1, fp);
        char *key = malloc(key_len + 1);
        fread(key, key_len, 1, fp);
        key[key_len] = '\0';

        TinyObj *obj = NULL;
        if (type == T_INT) {
            int val;
            fread(&val, sizeof(int), 1, fp);
            obj = obj_create_int(val);
        }
        else if(type == T_STRING) {
            size_t val_len;
            fread(&val_len, sizeof(size_t), 1 ,fp);
            char *str = malloc(val_len + 1);
            fread(str, val_len, 1, fp);
            str[val_len] = '\0';

            obj = obj_create_string(str);
            free(str);
        }
        if (obj) {
            ht_set(ht, key, obj);
        }
        free(key);
    }
    fclose(fp);
    printf("[Persistence] Loaded from %s\n", filename);
    return ht;
}