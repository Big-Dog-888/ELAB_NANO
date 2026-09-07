#include "unity.h"
#include "hash_table.h"
#include <string.h>

hash_table_t ht;
hash_table_data_t table[16];

void setUp(void)
{
    hash_table_init(&ht, table, 16);
}

void tearDown(void) {}

void test_init_should_clear_all_slots(void)
{
    for (uint32_t i = 0; i < 16; i++)
    {
        TEST_ASSERT_NULL(table[i].data);
        TEST_ASSERT_EQUAL_UINT32(UINT32_MAX, table[i].hash_time33);
    }
    TEST_ASSERT_EQUAL_UINT32(16, ht.capacity);
}

void test_add_and_get_should_roundtrip(void)
{
    int value = 42;
    TEST_ASSERT_EQUAL(ELAB_OK, hash_table_add(&ht, "answer", &value));

    void *result = hash_table_get(&ht, "answer");
    TEST_ASSERT_NOT_NULL(result);
    TEST_ASSERT_EQUAL_PTR(&value, result);
    TEST_ASSERT_EQUAL_INT(42, *(int *)result);
}

void test_get_nonexistent_should_return_null(void)
{
    TEST_ASSERT_NULL(hash_table_get(&ht, "nobody"));
}

void test_add_duplicate_name_does_not_check_duplicate(void)
{
    int v1 = 1, v2 = 2;
    hash_table_add(&ht, "key", &v1);
    hash_table_add(&ht, "key", &v2);

    TEST_ASSERT_TRUE(hash_table_existent(&ht, "key"));
    TEST_ASSERT_EQUAL_PTR(&v1, hash_table_get(&ht, "key"));
}

void test_remove_should_delete(void)
{
    int v = 100;
    hash_table_add(&ht, "to_remove", &v);
    TEST_ASSERT_TRUE(hash_table_existent(&ht, "to_remove"));

    TEST_ASSERT_EQUAL(ELAB_OK, hash_table_remove(&ht, "to_remove"));
    TEST_ASSERT_FALSE(hash_table_existent(&ht, "to_remove"));
    TEST_ASSERT_NULL(hash_table_get(&ht, "to_remove"));
}

void test_remove_nonexistent_should_return_error(void)
{
    TEST_ASSERT_EQUAL(ELAB_ERROR, hash_table_remove(&ht, "ghost"));
}

void test_add_many_until_full(void)
{
    int values[16];
    for (uint32_t i = 0; i < 16; i++)
    {
        values[i] = (int)i;
        char name[16];
        snprintf(name, sizeof(name), "key_%u", i);
        TEST_ASSERT_EQUAL(ELAB_OK, hash_table_add(&ht, name, &values[i]));
    }

    char name[16];
    snprintf(name, sizeof(name), "overflow");
    TEST_ASSERT_EQUAL(ELAB_ERR_FULL, hash_table_add(&ht, name, name));
}