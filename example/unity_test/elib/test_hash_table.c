/*
 * eLab Project
 * Copyright (c) 2023, EventOS Team, <event-os@outlook.com>
 */

#if defined(__linux__) || defined(_WIN32)

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include "../../os/cmsis_os.h"
#include "../../3rd/Unity/unity.h"
#include "../../3rd/Unity/unity_fixture.h"
#include "../../elib/hash_table.h"
#include "../../common/elab_common.h"
#include "../../common/elab_assert.h"

#define TAG                         "ut_hash_table"
#include "../../common/elab_log.h"

/* Private config ------------------------------------------------------------*/
#define UT_HASH_TABLE_SIZE_MAX                      (1000)
#define UT_HASH_TABLE_SIZE                          (128)
#define UT_STRING_LENGTH                            (64)
#define UT_HASH_TABLE_TEST_TIMES                    (10000)

#ifndef UT_HASH_TABLE_PRINT_EN
#define UT_HASH_TABLE_PRINT_EN                      (0)
#endif

/* Private function prototypes -----------------------------------------------*/
static void _random_string_generate(char *string, uint32_t size);
static uint16_t _get_max_prime(uint32_t size);
static void _put_string_into_table(char *str);
static elab_err_t _get_random_string_from_table(char *str);

/* Private variables ---------------------------------------------------------*/
static char *str_table[UT_HASH_TABLE_SIZE];
static uint32_t *payload = NULL;
static hash_table_t *hash_table = NULL;
static hash_table_data_t *ht_data = NULL;
static char *str_temp = NULL;
static char *str_change = NULL;

static const uint16_t prime_table[] = 
{
    2,3,5,7,11,13,17,19,23,29,31,37,41,43,47,53,59,61,67,71,73,79,83,89,97,101,
    103,107,109,113,127,131,137,139,149,151,157,163,167,173,179,181,191,193,197,
    199,211,223,227,229,233,239,241,251,257,263,269,271,277,281,283,293,307,311,
    313,317,331,337,347,349,353,359,367,373,379,383,389,397,401,409,419,421,431,
    433,439,443,449,457,461,463,467,479,487,491,499,503,509,521,523,541,547,557,
    563,569,571,577,587,593,599,601,607,613,617,619,631,641,643,647,653,659,661,
    673,677,683,691,701,709,719,727,733,739,743,751,757,761,769,773,787,797,809,
    811,821,823,827,829,839,853,857,859,863,877,881,883,887,907,911,919,929,937,
    941,947,953,967,971,977,983,991,997
};

/* Exported functions --------------------------------------------------------*/
/**
  * @brief  Define test group of hash table
  */
TEST_GROUP(hash_table);

/**
  * @brief  Define test fixture setup function of hash table
  */
TEST_SETUP(hash_table)
{
    TEST_ASSERT_LESS_OR_EQUAL_UINT32(UT_HASH_TABLE_SIZE_MAX, UT_HASH_TABLE_SIZE);

    for (uint32_t i = 0; i < UT_HASH_TABLE_SIZE; i ++)
    {
        str_table[i] = elab_malloc(UT_STRING_LENGTH);
        TEST_ASSERT_NOT_NULL(str_table[i]);
        memset(str_table[i], 0, UT_STRING_LENGTH);
    }

    payload = elab_malloc(UT_HASH_TABLE_TEST_TIMES * sizeof(uint32_t));
    TEST_ASSERT_NOT_NULL(payload);

    str_temp = elab_malloc(UT_STRING_LENGTH);
    TEST_ASSERT_NOT_NULL(str_temp);

    str_change = elab_malloc(UT_STRING_LENGTH);
    TEST_ASSERT_NOT_NULL(str_change);

    srand(time(0));
}

/**
  * @brief  Define test fixture tear down function of hash table
  */
TEST_TEAR_DOWN(hash_table)
{
    for (uint32_t i = 0; i < UT_HASH_TABLE_SIZE; i ++)
    {
        elab_free(str_table[i]);
    }
    elab_free(payload);
    elab_free(str_temp);
    elab_free(str_change);
}

/**
  * @brief  Test hash table init functions.
  */
TEST(hash_table, init)
{
    /* Testing for init. */
    hash_table = elab_malloc(sizeof(hash_table_t));
    TEST_ASSERT_NOT_NULL(hash_table);
    for (uint32_t i = 10; i < UT_HASH_TABLE_SIZE_MAX; i ++)
    {
        ht_data = elab_malloc(i * sizeof(hash_table_data_t));
        TEST_ASSERT_NOT_NULL(ht_data);

        hash_table_init(hash_table, ht_data, i);
        TEST_ASSERT_EQUAL_UINT32(i, hash_table->capacity);
        TEST_ASSERT_EQUAL_UINT32(_get_max_prime(i), hash_table->prime_max);

        elab_free(ht_data);
    }
    elab_free(hash_table);

    /* Testing for new and destroy. */
    for (uint32_t i = 10; i < UT_HASH_TABLE_SIZE_MAX; i ++)
    {
        hash_table = hash_table_new(i);
        TEST_ASSERT_EQUAL_UINT32(i, hash_table->capacity);
        TEST_ASSERT_EQUAL_UINT32(_get_max_prime(i), hash_table->prime_max);
        hash_table_destroy(hash_table);
    }
}

/**
  * @brief  Random adding and removing testing for hash table.
  */
TEST(hash_table, random_add_remove)
{
    elab_err_t ret;
    uint32_t count_fill = 0;
    uint32_t count_remove = 0;

    for (uint32_t i = 0; i < UT_HASH_TABLE_TEST_TIMES; i ++)
    {
        payload[i] = i;
    }

    /* Generate one hash table for testing. */
    hash_table_t *ht = hash_table_new(UT_HASH_TABLE_SIZE);

    bool fill = true;
    while (1)
    {
        fill = ((rand() % 2) == 0) ? false : true;

        /* Fill the data into the hash talbe. */
        if (fill && (count_fill < UT_HASH_TABLE_TEST_TIMES))
        {
            _random_string_generate(str_temp, UT_STRING_LENGTH);
            TEST_ASSERT_NULL(hash_table_get(ht, str_temp));
            TEST_ASSERT_FALSE(hash_table_existent(ht, str_temp));
            if (hash_table_add(ht, str_temp, &payload[count_fill]) == ELAB_OK)
            {
#if (UT_HASH_TABLE_PRINT_EN != 0)
                printf("\033[0;32m" "     + %s.\n" " \033[0m", str_temp);
#endif
                _put_string_into_table(str_temp);
                uint32_t *value = hash_table_get(ht, str_temp);
                TEST_ASSERT_EQUAL_UINT32(payload[count_fill], value[0]);
                TEST_ASSERT_TRUE(hash_table_existent(ht, str_temp));
                count_fill ++;

                memset(str_change, 0, UT_STRING_LENGTH);
                strcpy(str_change, str_temp);
                str_change[strlen(str_change) - 1] = 0;
                TEST_ASSERT_FALSE(hash_table_existent(ht, str_change));
                TEST_ASSERT_NULL(hash_table_get(ht, str_change));
                ret = hash_table_remove(ht, str_change);
                TEST_ASSERT(ret == ELAB_ERROR);
            }
        }

        /* Get the data from the hash talbe. */
        if (!fill && (count_remove < UT_HASH_TABLE_TEST_TIMES))
        {
            ret = _get_random_string_from_table(str_temp);
            if (ret == ELAB_OK)
            {
#if (UT_HASH_TABLE_PRINT_EN != 0)
                printf("\033[1;33m" "     - %s.\n" " \033[0m", str_temp);
#endif
                TEST_ASSERT_TRUE(hash_table_existent(ht, str_temp));
                TEST_ASSERT_NOT_NULL(hash_table_get(ht, str_temp));

                ret = hash_table_remove(ht, str_temp);
                TEST_ASSERT(ret == ELAB_OK);
                TEST_ASSERT_FALSE(hash_table_existent(ht, str_temp));
                TEST_ASSERT_NULL(hash_table_get(ht, str_temp));
                count_remove ++;
            }
        }

        /* Complete the unit test. */
        if ((count_fill >= UT_HASH_TABLE_TEST_TIMES) &&
            (count_remove >= UT_HASH_TABLE_TEST_TIMES))
        {
            break;
        }
    }

    /* Destroy the hash table. */
    hash_table_destroy(ht);
}

/**
  * @brief  Define run test cases of hash table
  */
TEST_GROUP_RUNNER(hash_table)
{
    RUN_TEST_CASE(hash_table, init);
    RUN_TEST_CASE(hash_table, random_add_remove);
}

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Put the string into string table.
  * @param  str     The input string.
  * @retval none
  */
static void _put_string_into_table(char *str)
{
    for (uint32_t i = 0; i < UT_HASH_TABLE_SIZE; i ++)
    {
        if (str_table[i][0] == 0)
        {
            strcpy(str_table[i], str);
            break;
        }
    }
}

/**
  * @brief  Get the string from string table.
  * @param  str     The input string.
  * @retval See elab_err_t.
  */
static elab_err_t _get_random_string_from_table(char *str)
{
    elab_err_t ret = ELAB_ERROR;
    uint32_t size_max_not_empty = 0;
    uint32_t index_start = rand() % UT_HASH_TABLE_SIZE;
    uint32_t index = 0;

    for (uint32_t i = 0; i < UT_HASH_TABLE_SIZE; i ++)
    {
        index = (index_start + i) % UT_HASH_TABLE_SIZE;
        if (str_table[index][0] != 0)
        {
            memset(str, 0, UT_STRING_LENGTH);
            strcpy(str, str_table[index]);
            memset(str_table[index], 0, UT_STRING_LENGTH);
            ret = ELAB_OK;
            break;
        }
    }

    return ret;
}

/**
  * @brief  Generate one random string.
  * @param  str     The input string.
  * @param  str     The string buffer size.
  * @retval None.
  */
static void _random_string_generate(char *str, uint32_t size)
{
    /* Generate the random length of the string. */
    uint32_t length = 0;
    while (1)
    {
        length = rand() % size;
        if (length > (size / 4) && length < size)
        {
            break;
        }
    }

    /* Generate the string: 0 - z. */
    char ch;
    uint16_t count = 0;
    memset(str, 0, size);
    for (uint32_t i = 0; i < length;)
    {
        ch = rand() % 128;
        if (ch >= '0' && ch <= 'z')
        {
            str[i ++] = ch;
        }
    }
}

/**
  * @brief  Get the maximum prime number in the specific range.
  * @param  size    The given range size.
  * @retval The maximum prime number.
  */
static uint16_t _get_max_prime(uint32_t size)
{
    elab_assert(size <= UT_HASH_TABLE_SIZE_MAX);

    uint16_t prime = prime_table[0];
    for (uint32_t i = 0; i < sizeof(prime_table) / sizeof(uint16_t); i ++)
    {
        if (size < prime_table[i])
        {
            break;
        }
        else
        {
            prime = prime_table[i];
        }
    }

    return prime;
}

#endif

/* ----------------------------- end of file -------------------------------- */
