/*
 * test_rift.c
 * Copyright (C) 2018 mk <mk@marvin>
 *
 * Distributed under terms of the MIT license.
 */

#include <stdio.h>
#include <string.h>
#include "minunit.h"
#include "../src/rift.h"

int tests_run = 0;


static char* test_rift_alloc_happy_path()
{
    printf("%s... ", __func__);
    char *expected = "0123456789";
    rift_buffer_t *gap;
    mu_assert(rift_alloc(&gap, expected, strlen(expected)) == RIFT_SUCCESS,
              "Could not allocate buffer");
    mu_assert(gap->siz == 16, "Unexpected allocation size");
    char data[32];
    size_t n = rift_read(gap, data, 32);
    data[n] = '\0';
    rift_free(gap);
    mu_assert(strncmp(expected, data, n) == 0,
              "Initialization string is not consistent");
    printf("OK\n");
    return 0;
}

static char* test_rift_alloc_zero_length()
{
    printf("%s... ", __func__);
    rift_buffer_t *gap;
    mu_assert(rift_alloc(&gap, "", 0) == RIFT_SUCCESS,
              "Could not allocate empty buffer");
    mu_assert(gap != NULL, "Empty buffer allocation failed");
    mu_assert(gap->siz == 0, "Unexpected allocation size");
    mu_assert(rift_insert(gap, "asdf", 4) == RIFT_SUCCESS,
              "Could not insert into empty buffer");
    char data[8];
    size_t n = rift_read(gap, data, 8);
    mu_assert(strncmp("asdf", data, n) == 0,
              "String inconsistency");
    rift_free(gap);
    printf("OK\n");
    return 0;
}

static char* test_rift_read_into_insufficient_buffer()
{
    printf("%s... ", __func__);
    rift_buffer_t *gap;
    mu_assert(rift_alloc(&gap, "0123456789", 10) == RIFT_SUCCESS,
              "Could not allocate buffer");
    char too_short[4];
    size_t n = rift_read(gap, too_short, 4);
    mu_assert(strncmp("0123456789", too_short, n) == 0,
              "String inconsistency for short buffer");
    printf("OK\n");
    return 0;
}

static char * test_suite()
{
    mu_run_test(test_rift_alloc_happy_path);
    mu_run_test(test_rift_alloc_zero_length);
    mu_run_test(test_rift_read_into_insufficient_buffer);
    return 0;
}

int main()
{
    char *result = test_suite();
    if (result != 0) {
        printf("%s\n", result);
    }
    else {
        printf("ALL TESTS PASSED\n");
    }
    printf("Tests run: %d\n", tests_run);

    return result != 0;
}

