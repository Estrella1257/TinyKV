#include "tinykv.h"
#include <stdio.h>
#include <stdlib.h> // remove()

int main() {
    printf("=== TinyKV Full Recovery Test (Snapshot + WAL) ===\n");
    
    // 1. 清理环境
    remove("tinykv.log");
    remove("tinykv.db");

    // 2. 阶段一：写入数据并触发 GC (生成快照)
    printf("\n>> Phase 1: Writing data to trigger GC...\n");
    HashTable *ht = ht_create();
    
    // 写入 1-5 (会触发 GC，进入 .db)
    for (int i = 1; i <= 5; i++) {
        char key[32]; sprintf(key, "data:%d", i);
        ht_set(ht, key, obj_create_int(i * 100));
        wal_log_set(ht, key, ht_get(ht, key));
    }
    printf("   (At this point, old data should be in .db snapshot)\n");

    // 写入 6-7 (不会触发 GC，留在 .log)
    printf(">> Phase 2: Writing recent data (in WAL only)...\n");
    ht_set(ht, "recent:1", obj_create_string("I am new"));
    wal_log_set(ht, "recent:1", ht_get(ht, "recent:1"));
    
    ht_set(ht, "recent:2", obj_create_string("Me too"));
    wal_log_set(ht, "recent:2", ht_get(ht, "recent:2"));

    // 3. 模拟断电重启
    printf("\nSimulating Restart...\n");
    ht_free(ht); // 销毁内存
    ht = ht_create(); // 新的空表

    // 4. 阶段三：执行恢复
    // 预期：先加载 data:1~5 (from .db)，再加载 recent:1~2 (from .log)
    wal_recover(ht, "tinykv.log");

    // 5. 验证数据
    printf("\n>> Phase 3: Verifying Data integrity...\n");
    
    // 验证来自快照的数据
    TinyObj *obj1 = ht_get(ht, "data:1");
    if (obj1 && *(int*)obj1->ptr == 100) printf("[PASS] Snapshot data (data:1) recovered.\n");
    else printf("[FAIL] Snapshot data missing!\n");

    // 验证来自日志的数据
    TinyObj *objNew = ht_get(ht, "recent:1");
    if (objNew) printf("[PASS] WAL data (recent:1) recovered: %s\n", (char*)objNew->ptr);
    else printf("[FAIL] WAL data missing!\n");

    ht_free(ht);
    return 0;
}