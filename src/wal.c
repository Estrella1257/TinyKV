#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "tinykv.h"

#define WAL_FILE "tinykv.log"

void wal_log_set(char *key, TinyObj *value) {
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
} 

void wal_log_del(char *key) {
    FILE *fp = fopen(WAL_FILE, "a");
    if (!fp) return;

    fprintf(fp, "DEL %s\n", key);
    
    fflush(fp); 
    fclose(fp);
}

void wal_recover(HashTable *ht, const char *filename) {
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