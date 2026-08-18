/*
 * Copyright (C) 2026 Mark Karaman
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This file is part of KDSE.
 */

#include "kdse/kdse8_checked.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define ARRAY_LENGTH(a) (sizeof(a) / sizeof((a)[0]))

static int failures = 0;

#define CHECK(condition) do { \
    if (!(condition)) { \
        (void)fprintf(stderr, "%s:%d: check failed: %s\n", \
                      __FILE__, __LINE__, #condition); \
        ++failures; \
    } \
} while (0)

typedef struct ordered_case {
    uint8_t payload;
    uint8_t length;
    uint8_t max_depth;
    uint8_t branches;
    uint8_t terminals;
    uint8_t profile[KDSE8_MAX_DEPTH + 1u];
} ordered_case_t;

static const uint8_t valid_payloads[] = {
    0u, 1u, 5u, 6u, 7u, 21u, 22u, 23u, 25u, 26u, 27u,
    85u, 86u, 87u, 89u, 90u, 91u, 101u, 102u, 103u, 105u,
    106u, 107u, 113u, 114u, 115u, 116u, 117u, 118u, 119u,
    120u, 121u, 122u, 123u, 124u, 125u, 126u, 127u
};

static const ordered_case_t ordered_cases[] = {
    {0u,   1u, 0u, 0u, 1u, {1u, 0u, 0u, 0u, 0u}},
    {1u,   1u, 1u, 1u, 2u, {0u, 2u, 0u, 0u, 0u}},
    {5u,   3u, 2u, 2u, 3u, {0u, 1u, 2u, 0u, 0u}},
    {7u,   3u, 2u, 3u, 4u, {0u, 0u, 4u, 0u, 0u}},
    {21u,  5u, 3u, 3u, 4u, {0u, 1u, 1u, 2u, 0u}},
    {23u,  5u, 3u, 4u, 5u, {0u, 1u, 0u, 4u, 0u}},
    {85u,  7u, 4u, 4u, 5u, {0u, 1u, 1u, 1u, 2u}},
    {87u,  7u, 4u, 5u, 6u, {0u, 1u, 1u, 0u, 4u}},
    {113u, 7u, 3u, 4u, 5u, {0u, 0u, 3u, 2u, 0u}},
    {115u, 7u, 3u, 5u, 6u, {0u, 0u, 2u, 4u, 0u}},
    {117u, 7u, 3u, 5u, 6u, {0u, 0u, 2u, 4u, 0u}},
    {119u, 7u, 3u, 6u, 7u, {0u, 0u, 1u, 6u, 0u}},
    {127u, 7u, 3u, 7u, 8u, {0u, 0u, 0u, 8u, 0u}}
};

typedef struct canonical_case {
    uint8_t ordered;
    const uint8_t *members;
    size_t member_count;
} canonical_case_t;

static const uint8_t c0[] = {0u};
static const uint8_t c1[] = {1u};
static const uint8_t c5[] = {5u, 6u};
static const uint8_t c7[] = {7u};
static const uint8_t c21[] = {21u, 22u, 25u, 26u};
static const uint8_t c23[] = {23u, 27u};
static const uint8_t c85[] = {85u, 86u, 89u, 90u, 101u, 102u, 105u, 106u};
static const uint8_t c87[] = {87u, 91u, 103u, 107u};
static const uint8_t c113[] = {113u, 114u, 116u, 120u};
static const uint8_t c115[] = {115u, 124u};
static const uint8_t c117[] = {117u, 118u, 121u, 122u};
static const uint8_t c119[] = {119u, 123u, 125u, 126u};
static const uint8_t c127[] = {127u};

static const canonical_case_t canonical_cases[] = {
    {0u, c0, ARRAY_LENGTH(c0)},
    {1u, c1, ARRAY_LENGTH(c1)},
    {5u, c5, ARRAY_LENGTH(c5)},
    {7u, c7, ARRAY_LENGTH(c7)},
    {21u, c21, ARRAY_LENGTH(c21)},
    {23u, c23, ARRAY_LENGTH(c23)},
    {85u, c85, ARRAY_LENGTH(c85)},
    {87u, c87, ARRAY_LENGTH(c87)},
    {113u, c113, ARRAY_LENGTH(c113)},
    {115u, c115, ARRAY_LENGTH(c115)},
    {117u, c117, ARRAY_LENGTH(c117)},
    {119u, c119, ARRAY_LENGTH(c119)},
    {127u, c127, ARRAY_LENGTH(c127)}
};

static bool listed_valid(uint8_t payload)
{
    size_t index;

    for (index = 0u; index < ARRAY_LENGTH(valid_payloads); ++index) {
        if (valid_payloads[index] == payload) {
            return true;
        }
    }
    return false;
}

static void test_exhaustive_validation(void)
{
    unsigned value;

    CHECK(ARRAY_LENGTH(valid_payloads) == 38u);
    for (value = 0u; value <= UINT8_MAX; ++value) {
        const bool expected = listed_valid((uint8_t)(value & 0x7fu));
        const bool actual =
            kdse8_validate((kdse8_t)value) == KDSE_STATUS_OK;
        CHECK(actual == expected);
    }
}

static void test_profiles(void)
{
    size_t index;

    for (index = 0u; index < ARRAY_LENGTH(ordered_cases); ++index) {
        const ordered_case_t *expected = &ordered_cases[index];
        kdse8_profile_t actual;

        CHECK(kdse8_decode_profile(expected->payload, &actual) ==
              KDSE_STATUS_OK);
        CHECK(actual.payload_length == expected->length);
        CHECK(actual.max_depth == expected->max_depth);
        CHECK(actual.branch_count == expected->branches);
        CHECK(actual.terminal_count == expected->terminals);
        CHECK(memcmp(actual.terminals, expected->profile,
                     sizeof(actual.terminals)) == 0);
    }
}

static void test_canonicalization(void)
{
    size_t class_index;

    for (class_index = 0u;
         class_index < ARRAY_LENGTH(canonical_cases);
         ++class_index) {
        const canonical_case_t *expected = &canonical_cases[class_index];
        size_t member_index;

        for (member_index = 0u;
             member_index < expected->member_count;
             ++member_index) {
            const uint8_t member = expected->members[member_index];
            kdse8_t ordered = UINT8_MAX;
            bool is_ordered = false;

            CHECK(kdse8_canonicalize(member, &ordered) == KDSE_STATUS_OK);
            CHECK(ordered == expected->ordered);
            CHECK(kdse8_canonicalize((uint8_t)(member | 0x80u),
                                          &ordered) == KDSE_STATUS_OK);
            CHECK(ordered == expected->ordered);
            CHECK(kdse8_is_ordered(member, &is_ordered) ==
                  KDSE_STATUS_OK);
            CHECK(is_ordered == (member == expected->ordered));
        }
    }
}

static void test_errors_and_arguments(void)
{
    kdse8_profile_t profile;
    kdse8_t ordered = UINT8_MAX;
    bool is_ordered = false;

    CHECK(kdse8_validate(2u) == KDSE_STATUS_INVALID_PAYLOAD_LENGTH);
    CHECK(kdse8_validate(4u) ==
          KDSE_STATUS_EXPLICIT_FINAL_TERMINAL_LEVEL);
    CHECK(kdse8_validate(17u) ==
          KDSE_STATUS_DATA_AFTER_TERMINAL_LEVEL);
    CHECK(kdse8_validate(31u) == KDSE_STATUS_TRUNCATED_LEVEL);
    CHECK(kdse8_decode_profile(0u, NULL) == KDSE_STATUS_NULL_ARGUMENT);
    CHECK(kdse8_canonicalize(0u, NULL) == KDSE_STATUS_NULL_ARGUMENT);
    CHECK(kdse8_is_ordered(0u, NULL) == KDSE_STATUS_NULL_ARGUMENT);
    CHECK(kdse8_decode_profile(4u, &profile) ==
          KDSE_STATUS_EXPLICIT_FINAL_TERMINAL_LEVEL);
    CHECK(kdse8_canonicalize(4u, &ordered) ==
          KDSE_STATUS_EXPLICIT_FINAL_TERMINAL_LEVEL);
    CHECK(ordered == UINT8_MAX);
    CHECK(kdse8_is_ordered(4u, &is_ordered) ==
          KDSE_STATUS_EXPLICIT_FINAL_TERMINAL_LEVEL);
    CHECK(!is_ordered);
    CHECK(kdse8_payload(0x85u) == 5u);
    CHECK(kdse8_payload_length(0u) == 1u);
    CHECK(kdse8_payload_length(0x85u) == 3u);
    CHECK(kdse_status_string((kdse_status_t)999) != NULL);
}

int main(void)
{
    test_exhaustive_validation();
    test_profiles();
    test_canonicalization();
    test_errors_and_arguments();

    if (failures != 0) {
        (void)fprintf(stderr, "checked tests: %d failure(s)\n", failures);
        return 1;
    }
    (void)puts("checked tests: pass");
    return 0;
}
