#include "tinykv.h"
#include <stdio.h>
#include <assert.h>

int main() {
    printf("=== TinyKV Delete Test ===\n");
    HashTable *ht = ht_create();

    // 1. 准备数据：制造哈希冲突 (Chain)
    // 假设 buckets size 是 32。
    // 我们放入几个数据，虽然不一定冲突，但我们可以测试多种情况。
    ht_set(ht, "user:1", obj_create_string("Alice"));
    ht_set(ht, "user:2", obj_create_string("Bob"));
    ht_set(ht, "user:3", obj_create_string("Charlie"));

    printf("Count before delete: %ld\n", ht->count);

    // 2. 测试删除中间的数据
    printf("Deleting user:2 (Bob)...\n");
    ht_delete(ht, "user:2");

    // 验证：user:2 应该没了
    if (ht_get(ht, "user:2") == NULL) {
        printf("[PASS] user:2 is gone.\n");
    } else {
        printf("[FAIL] user:2 is still there!\n");
    }

    // 验证：user:1 和 user:3 还在 (防止断链误删别人)
    if (ht_get(ht, "user:1") && ht_get(ht, "user:3")) {
        printf("[PASS] Neighbors are safe.\n");
    } else {
        printf("[FAIL] You broke the linked list!\n");
    }

    // 3. 测试删除头节点
    printf("Deleting user:1 (Head node)...\n");
    ht_delete(ht, "user:1");
    if (ht_get(ht, "user:1") == NULL) printf("[PASS] user:1 is gone.\n");

    // 4. 测试删除不存在的 Key (程序不应该崩)
    printf("Deleting ghost key...\n");
    ht_delete(ht, "ghost_key");
    printf("[PASS] Survival from deleting non-existent key.\n");

    printf("Count after delete: %ld\n", ht->count);

    ht_free(ht);
    return 0;
}