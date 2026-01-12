#include "tinykv.h"
#include <stdio.h>
#include <string.h>
#include <assert.h> // 用于 assert 报错

#define DB_FILE "tinykv.db"

/* --- 辅助测试函数：验证整数 --- */
void assert_kv_int(HashTable *ht, char *key, int expected_val) {
    TinyObj *obj = ht_get(ht, key);
    if (!obj) {
        printf("[FAIL] Key '%s' not found!\n", key);
        return;
    }
    if (obj->type != T_INT) {
        printf("[FAIL] Key '%s' type error! Expected T_INT.\n", key);
        return;
    }
    
    int actual_val = *(int*)(obj->ptr);
    if (actual_val != expected_val) {
        printf("[FAIL] Key '%s' value mismatch! Expected: %d, Got: %d\n", key, expected_val, actual_val);
    } else {
        printf("[PASS] Key '%s' is correctly %d\n", key, actual_val);
    }
}

/* --- 辅助测试函数：验证字符串 --- */
void assert_kv_string(HashTable *ht, char *key, const char *expected_val) {
    TinyObj *obj = ht_get(ht, key);
    if (!obj) {
        printf("[FAIL] Key '%s' not found!\n", key);
        return;
    }
    if (obj->type != T_STRING) {
        printf("[FAIL] Key '%s' type error! Expected T_STRING.\n", key);
        return;
    }
    
    char *actual_val = (char*)(obj->ptr);
    if (strcmp(actual_val, expected_val) != 0) {
        printf("[FAIL] Key '%s' value mismatch!\n  Expected: '%s'\n  Got:      '%s'\n", key, expected_val, actual_val);
    } else {
        printf("[PASS] Key '%s' is correctly '%s'\n", key, actual_val);
    }
}

int main() {
    printf("=== TinyKV Comprehensive Persistence Test ===\n\n");

    // 阶段 1：生成数据并保存 (Write Phase)
    printf("--- Phase 1: Generating Data & Saving ---\n");
    HashTable *ht_write = ht_create();

    // 1. 测试普通正整数
    ht_set(ht_write, "config:max_users", obj_create_int(1000));
    
    // 2. 测试负数 (验证二进制补码存储)
    ht_set(ht_write, "sensor:temp_min", obj_create_int(-40));
    
    // 3. 测试 0
    ht_set(ht_write, "status:errors", obj_create_int(0));

    // 4. 测试含空格和标点的长字符串 (验证长度读取)
    char *long_str = "Hello World! This is a persistence test.";
    ht_set(ht_write, "msg:welcome", obj_create_string(long_str));

    // 5. 测试短字符串
    ht_set(ht_write, "app:ver", obj_create_string("v1.0"));

    // 保存到硬盘
    kv_save(ht_write, DB_FILE);

    // 【关键步骤】销毁内存中的表！
    // 这样能确保我们下一阶段读取的数据绝对来自硬盘，而不是残留的内存。
    ht_free(ht_write);
    printf("Memory cleared. Simulating restart...\n\n");

    // 阶段 2：从硬盘加载并验证 (Read & Verify Phase)
    printf("--- Phase 2: Loading Data & Verifying ---\n");
    HashTable *ht_read = kv_load(DB_FILE);
    
    if (!ht_read) {
        fprintf(stderr, "[FATAL] Failed to load database file!\n");
        return 1;
    }

    // 开始严格比对
    assert_kv_int(ht_read, "config:max_users", 1000);
    assert_kv_int(ht_read, "sensor:temp_min", -40); // 重点看这个
    assert_kv_int(ht_read, "status:errors", 0);
    
    assert_kv_string(ht_read, "msg:welcome", "Hello World! This is a persistence test.");
    assert_kv_string(ht_read, "app:ver", "v1.0");

    // 测试不存在的 Key (应该找不到)
    if (ht_get(ht_read, "ghost:key") == NULL) {
        printf("[PASS] Non-existent key correctly returned NULL.\n");
    } else {
        printf("[FAIL] Found a key that shouldn't exist!\n");
    }

    // 阶段 3：清理
    ht_free(ht_read);
    printf("\n=== Test Finished ===\n");
    
    return 0;
}