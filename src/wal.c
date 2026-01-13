#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tinykv.h"

#define WAL_FILE "tinykv.log"
#define DB_FILE  "tinykv.db"
#define TMP_FILE "tinykv.tmp"
#define GC_THRESHOLD 200 

void wal_check_gc(HashTable *ht) {
    FILE *fp = fopen(WAL_FILE, "r");
    if (!fp) return;

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fclose(fp);

    if (size < GC_THRESHOLD) return;

    printf("\n[GC] Log size %ld > %d. Triggering COMPACTION...\n", size, GC_THRESHOLD);

    kv_save(ht, TMP_FILE);

    if (rename(TMP_FILE, DB_FILE) == 0) {
        FILE *fp_clear = fopen(WAL_FILE, "w");
        if (fp_clear) fclose(fp_clear);
        printf("[GC] Checkpoint created. Log cleared.\n");
    } else {
        perror("[GC] Rename failed");
    }
}

void wal_log_set(HashTable *ht, char *key, TinyObj *value) {
    FILE *fp = fopen(WAL_FILE, "a");
    if (!fp) {
        perror("WAL open failed!");
        return;
    }

    fprintf(fp, "SET %s ", key);

    if (value->type == T_INT) {
        int val =*(int *)(value->ptr);
        fprintf(fp, "%d %d\n", T_INT, val);
    }
    else if (value->type == T_STRING) {
        char *str = (char*)(value->ptr);
        fprintf(fp,"%d %s\n", T_STRING, str);
    }
    fflush(fp);
    fclose(fp);

    wal_check_gc(ht);
} 

void wal_log_del(HashTable *ht, char *key) {
    FILE *fp = fopen(WAL_FILE, "a");
    if (!fp) return;

    fprintf(fp, "DEL %s\n", key);
    
    fflush(fp); 
    fclose(fp);

    wal_check_gc(ht);
}

void wal_recover(HashTable *ht, const char *filename) {
    HashTable *snapshot = kv_load(DB_FILE);
    if (snapshot) {
        printf("[Recover] Found snapshot %s. Merging into memory...\n", DB_FILE);
        
        for (size_t i = 0; i < snapshot->size; i++) {
            Entry *entry = snapshot->buckets[i];
            while (entry) {
                TinyObj *new_obj = NULL;
                
                if (entry->value->type == T_INT) {
                    new_obj = obj_create_int(*(int*)(entry->value->ptr));
                } 
                else if (entry->value->type == T_STRING) {
                    new_obj = obj_create_string((char*)(entry->value->ptr));
                }
                
                if (new_obj) {
                    ht_set(ht, entry->key, new_obj);
                }
                
                entry = entry->next;
            }
        }
        
        ht_free(snapshot); 
        printf("[Recover] Snapshot merge complete.\n");
    } else {
        printf("[Recover] No snapshot found. Starting fresh or from WAL only.\n");
    }

    FILE *fp = fopen(filename, "r");
    if (!fp) return;

    printf("[WAL] Replaying log from %s...\n", filename);

    char line[1024]; 
    int line_num = 0;

    while (fgets(line, sizeof(line), fp)) {
        line_num++;
        line[strcspn(line, "\n")] = 0; 
        
        char cmd[10];
        char key[256];
        int type_int;
        
        if (strncmp(line, "SET", 3) == 0) {
            if (sscanf(line, "%s %s %d", cmd, key, &type_int) < 3) {
                printf("[WAL] Line %d: Parse error (Header)\n", line_num);
                continue;
            }

            TinyObj *obj = NULL;

            if (type_int == T_INT) {
                int val;
                sscanf(line, "%*s %*s %*d %d", &val); 
                obj = obj_create_int(val);
                printf("[WAL] Line %d: Recovered INT %s -> %d\n", line_num, key, val);
            } 
            else if (type_int == T_STRING) {
                char str_buf[1024];
                sscanf(line, "%*s %*s %*d %[^\n]", str_buf);
                obj = obj_create_string(str_buf);
                printf("[WAL] Line %d: Recovered STR %s -> %s\n", line_num, key, str_buf);
            }

            if (obj) ht_set(ht, key, obj);
        }
        else if (strncmp(line, "DEL", 3) == 0) {
            sscanf(line, "%s %s", cmd, key);
            ht_delete(ht, key);
            printf("[WAL] Line %d: Deleted %s\n", line_num, key);
        }
    }

    fclose(fp);
    printf("[WAL] Recovery complete.\n");
}