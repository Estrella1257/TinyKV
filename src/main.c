#include <stdio.h>
#include "tinykv.h"

int main() {
    printf("=== TinyKV Hash Table Test ===\n");

    // 1. 创建哈希表
    HashTable *ht = ht_create();
    if (!ht) {
        fprintf(stderr, "Failed to create hash table\n");
        return 1;
    }
    printf("[Init] Hash table created.\n");

    // 2. 插入数据 (Set)
    // 插入字符串
    ht_set(ht, "user:1", obj_create_string("Estrella"));
    // 插入整数
    ht_set(ht, "user:1:age", obj_create_int(22));
    printf("[Set] Values inserted.\n");

    // 3. 读取数据 (Get)
    TinyObj *name = ht_get(ht, "user:1");
    TinyObj *age = ht_get(ht, "user:1:age");

    if (name) {
        printf("Found user:1 -> ");
        obj_print(name);
    } else {
        printf("user:1 not found!\n");
    }

    if (age) {
        printf("Found user:1:age -> ");
        obj_print(age);
    }

    // 4. 测试更新 (Update) - 这是检测内存泄漏的关键点
    // 我们把 user:1 的名字从 "Estrella" 改为 "Estrella_Pro"
    // 如果逻辑正确，旧的 "Estrella" 应该被自动释放，不会泄露
    printf("[Update] Updating user:1...\n");
    ht_set(ht, "user:1", obj_create_string("Estrella_Pro"));

    // 验证更新结果
    TinyObj *newName = ht_get(ht, "user:1");
    printf("Updated user:1 -> ");
    if (newName) obj_print(newName);

    // 5. 打印结构 (Dump)
    ht_dump(ht);

    // 6. 销毁 (Free)
    // 这将触发连锁反应：Free Table -> Free Buckets -> Free Entries -> Free Keys & Values
    ht_free(ht);
    printf("[Free] Hash table destroyed.\n");

    return 0;
}