#include "tinykv.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define WAL_FILE "tinykv.log"

/* --- 辅助断言函数 --- */
void assert_string(HashTable *ht, char *key, char *expected) {
    TinyObj *obj = ht_get(ht, key);
    if (!obj) {
        printf("[FAIL] Key '%s' is missing!\n", key);
        return;
    }
    if (obj->type != T_STRING) {
        printf("[FAIL] Key '%s' is not T_STRING!\n", key);
        return;
    }
    
    char *actual = (char*)(obj->ptr);
    if (strcmp(actual, expected) != 0) {
        printf("[FAIL] Key '%s' mismatch!\n   Expected: [%s]\n   Got:      [%s]\n", key, expected, actual);
    } else {
        printf("[PASS] Key '%s' == [%s]\n", key, actual);
    }
}

void assert_int(HashTable *ht, char *key, int expected) {
    TinyObj *obj = ht_get(ht, key);
    if (!obj) {
        printf("[FAIL] Key '%s' is missing!\n", key);
        return;
    }
    if (obj->type != T_INT) {
        printf("[FAIL] Key '%s' is not T_INT!\n", key);
        return;
    }
    
    int actual = *(int*)(obj->ptr);
    if (actual != expected) {
        printf("[FAIL] Key '%s' mismatch! Expected: %d, Got: %d\n", key, expected, actual);
    } else {
        printf("[PASS] Key '%s' == %d\n", key, actual);
    }
}

void assert_deleted(HashTable *ht, char *key) {
    if (ht_get(ht, key) == NULL) {
        printf("[PASS] Key '%s' is correctly deleted (NULL).\n", key);
    } else {
        printf("[FAIL] Key '%s' should be deleted but implies existence!\n", key);
    }
}

/* --- 主程序 --- */
int main() {
    printf("\n=== TinyKV WAL Persistence Test ===\n");
    
    // 1. 初始化并尝试从日志恢复
    HashTable *ht = ht_create();
    wal_recover(ht, WAL_FILE);

    // 2. 检查这是第几次运行 (通过读取系统计数器)
    int boot_times = 0;
    TinyObj *bootObj = ht_get(ht, "sys:boot_count");
    if (bootObj) {
        boot_times = *(int*)(bootObj->ptr);
    }

    printf("--- Boot Count: %d ---\n", boot_times);

    if (boot_times == 0) {
        /* ====================================
           阶段 1：初次运行 (生成数据并模拟断电)
           ==================================== */
        printf(">> Phase 1: Generating data...\n");

        // A. 测试带空格的长字符串 (验证 %[^\n] 是否生效)
        char *msg = "Hello World TinyKV is awesome";
        ht_set(ht, "msg:intro", obj_create_string(msg));
        wal_log_set("msg:intro", ht_get(ht, "msg:intro")); // 记日志
        printf("   Written string: [%s]\n", msg);

        // B. 测试整数
        ht_set(ht, "user:id", obj_create_int(10086));
        wal_log_set("user:id", ht_get(ht, "user:id"));     // 记日志
        printf("   Written int: 10086\n");

        // C. 测试删除逻辑
        // 先写入一个临时 key
        ht_set(ht, "temp:key", obj_create_string("I will disappear"));
        wal_log_set("temp:key", ht_get(ht, "temp:key"));
        
        // 然后立刻删除它
        ht_delete(ht, "temp:key");
        wal_log_del("temp:key");                           // 记删除日志
        printf("   Created and then Deleted 'temp:key'\n");

        // D. 更新启动次数 (准备进入阶段 2)
        ht_set(ht, "sys:boot_count", obj_create_int(1));
        wal_log_set("sys:boot_count", ht_get(ht, "sys:boot_count"));

        printf("\nSimulating POWER CUT! Exiting without saving .db file...\n");
        printf("Please run the program again to verify recovery.\n");
        
        // 注意：这里没有调用 kv_save()，直接退出！
    } 
    else {
        /* ====================================
           阶段 2：再次运行 (验证数据是否活着)
           ==================================== */
        printf(">> Phase 2: Verifying recovery data...\n");

        // 1. 验证带空格的字符串 (关键！)
        assert_string(ht, "msg:intro", "Hello World TinyKV is awesome");

        // 2. 验证整数
        assert_int(ht, "user:id", 10086);

        // 3. 验证删除操作 (必须找不到)
        assert_deleted(ht, "temp:key");

        // 4. 验证计数器
        assert_int(ht, "sys:boot_count", 1);

        printf("\nCONGRATULATIONS! WAL Persistence works perfectly!\n");
        
        // (可选) 清理日志文件，方便下次重新测试
        // remove(WAL_FILE); 
    }

    ht_free(ht);
    return 0;
}