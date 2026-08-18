/*
 * Copyright (C) 2026 Mark Karaman
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This file is part of KDSE.
 */

#include "kdse/kdse16_checked.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CLASS_CAPACITY 256u

static int failures = 0;

#define CHECK(condition) do { \
    if (!(condition)) { \
        (void)fprintf(stderr, "%s:%d: check failed: %s\n", \
                      __FILE__, __LINE__, #condition); \
        ++failures; \
    } \
} while (0)

typedef struct reference_node {
    uint8_t branch;
    uint8_t child[2];
} reference_node_t;

typedef struct reference_tree {
    reference_node_t nodes[KDSE16_MAX_NODES];
    uint8_t count;
} reference_tree_t;

typedef struct topology_class {
    char key[KDSE16_MAX_NODES + 1u];
    uint16_t minimum;
} topology_class_t;

static uint8_t reference_length(uint16_t payload)
{
    uint8_t length = 0u;

    if (payload == 0u) {
        return 1u;
    }
    while (payload != 0u) {
        ++length;
        payload = (uint16_t)(payload >> 1u);
    }
    return length;
}

static uint8_t reference_bit(uint16_t payload,
                             uint8_t length,
                             uint8_t position)
{
    const uint8_t shift = (uint8_t)((unsigned)length -
        (unsigned)position - 1u);
    return (uint8_t)((payload >> shift) & UINT16_C(1));
}

static void reference_tree_build(uint16_t payload, reference_tree_t *tree)
{
    const uint8_t length = reference_length(payload);
    uint8_t queue[KDSE16_MAX_NODES];
    uint8_t head = 0u;
    uint8_t tail = 1u;
    uint8_t position;

    memset(tree, 0, sizeof(*tree));
    tree->count = 1u;
    queue[0] = 0u;
    for (position = 0u; position < length; ++position) {
        const uint8_t node_index = queue[head++];

        if (reference_bit(payload, length, position) != 0u) {
            reference_node_t *node = &tree->nodes[node_index];
            node->branch = 1u;
            node->child[0] = tree->count++;
            node->child[1] = tree->count++;
            queue[tail++] = node->child[0];
            queue[tail++] = node->child[1];
        }
    }
}

static size_t reference_key_node(
    const reference_tree_t *tree,
    uint8_t node_index,
    char key[KDSE16_MAX_NODES + 1u])
{
    const reference_node_t *node = &tree->nodes[node_index];
    char left[KDSE16_MAX_NODES + 1u];
    char right[KDSE16_MAX_NODES + 1u];
    const char *first;
    const char *second;
    size_t left_length;
    size_t right_length;
    size_t first_length;
    size_t second_length;

    if (node->branch == 0u) {
        key[0] = '0';
        key[1] = '\0';
        return 1u;
    }
    left_length = reference_key_node(tree, node->child[0], left);
    right_length = reference_key_node(tree, node->child[1], right);
    if (strcmp(left, right) <= 0) {
        first = left;
        first_length = left_length;
        second = right;
        second_length = right_length;
    } else {
        first = right;
        first_length = right_length;
        second = left;
        second_length = left_length;
    }
    key[0] = '1';
    memcpy(&key[1], first, first_length);
    memcpy(&key[1u + first_length], second, second_length);
    key[1u + first_length + second_length] = '\0';
    return 1u + first_length + second_length;
}

static void reference_key(uint16_t payload,
                          char key[KDSE16_MAX_NODES + 1u])
{
    reference_tree_t tree;

    reference_tree_build(payload, &tree);
    (void)reference_key_node(&tree, 0u, key);
}

static size_t find_or_add_class(
    topology_class_t classes[CLASS_CAPACITY],
    size_t *class_count,
    const char *key,
    uint16_t payload)
{
    size_t index;

    for (index = 0u; index < *class_count; ++index) {
        if (strcmp(classes[index].key, key) == 0) {
            return index;
        }
    }
    CHECK(*class_count < CLASS_CAPACITY);
    if (*class_count >= CLASS_CAPACITY) {
        return 0u;
    }
    (void)strcpy(classes[*class_count].key, key);
    classes[*class_count].minimum = payload;
    return (*class_count)++;
}

static void check_profile_invariants(uint16_t payload)
{
    kdse16_profile_t profile;
    uint32_t kraft_numerator = 0u;
    uint8_t depth;

    CHECK(kdse16_decode_profile(payload, &profile) == KDSE_STATUS_OK);
    CHECK(profile.payload_length == reference_length(payload));
    CHECK((profile.payload_length & UINT8_C(1)) != 0u);
    CHECK(profile.max_depth <= KDSE16_MAX_DEPTH);
    CHECK(profile.terminal_count == (uint8_t)(profile.branch_count + 1u));
    for (depth = 0u; depth <= profile.max_depth; ++depth) {
        kraft_numerator += (uint32_t)profile.terminals[depth] <<
            (profile.max_depth - depth);
    }
    CHECK(kraft_numerator == (UINT32_C(1) << profile.max_depth));
}

static void test_complete_space(void)
{
    topology_class_t classes[CLASS_CAPACITY];
    size_t class_count = 0u;
    uint32_t valid_count = 0u;
    uint32_t payload;

    memset(classes, 0, sizeof(classes));
    for (payload = 0u; payload <= KDSE16_PAYLOAD_MASK; ++payload) {
        const kdse_status_t status = kdse16_validate((uint16_t)payload);
        const kdse_status_t high_status = kdse16_validate(
            (uint16_t)(payload | UINT16_C(0x8000)));

        CHECK(status == high_status);
        if (status == KDSE_STATUS_OK) {
            char key[KDSE16_MAX_NODES + 1u];
            kdse16_t ordered = UINT16_MAX;
            kdse16_t ordered_high = UINT16_MAX;
            kdse16_t ordered_again = UINT16_MAX;
            bool is_ordered = false;
            size_t class_index;

            ++valid_count;
            reference_key((uint16_t)payload, key);
            class_index = find_or_add_class(
                classes, &class_count, key, (uint16_t)payload);
            CHECK(kdse16_canonicalize((uint16_t)payload, &ordered) ==
                  KDSE_STATUS_OK);
            CHECK(kdse16_canonicalize(
                      (uint16_t)(payload | UINT16_C(0x8000)), &ordered_high) ==
                  KDSE_STATUS_OK);
            CHECK(ordered == classes[class_index].minimum);
            CHECK(ordered_high == ordered);
            CHECK(ordered <= payload);
            CHECK(kdse16_validate(ordered) == KDSE_STATUS_OK);
            CHECK(kdse16_canonicalize(ordered, &ordered_again) ==
                  KDSE_STATUS_OK);
            CHECK(ordered_again == ordered);
            CHECK(kdse16_is_ordered((uint16_t)payload, &is_ordered) ==
                  KDSE_STATUS_OK);
            CHECK(is_ordered == (ordered == payload));
            check_profile_invariants((uint16_t)payload);
        }
    }
    CHECK(valid_count == UINT32_C(4397));
    CHECK(class_count == 198u);
}

static void test_profiles_and_errors(void)
{
    static const uint8_t chain_profile[KDSE16_MAX_DEPTH + 1u] =
        {0u, 1u, 1u, 1u, 1u, 1u, 1u, 1u, 2u};
    static const uint8_t full_profile[KDSE16_MAX_DEPTH + 1u] =
        {0u, 0u, 0u, 0u, 16u, 0u, 0u, 0u, 0u};
    kdse16_profile_t profile;
    kdse16_t ordered = UINT16_MAX;
    bool is_ordered = false;

    CHECK(kdse16_decode_profile(UINT16_C(0x5555), &profile) ==
          KDSE_STATUS_OK);
    CHECK(profile.payload_length == 15u);
    CHECK(profile.max_depth == 8u);
    CHECK(profile.branch_count == 8u);
    CHECK(profile.terminal_count == 9u);
    CHECK(memcmp(profile.terminals, chain_profile,
                 sizeof(chain_profile)) == 0);

    CHECK(kdse16_decode_profile(UINT16_C(0x7fff), &profile) ==
          KDSE_STATUS_OK);
    CHECK(profile.max_depth == 4u);
    CHECK(profile.branch_count == 15u);
    CHECK(profile.terminal_count == 16u);
    CHECK(memcmp(profile.terminals, full_profile,
                 sizeof(full_profile)) == 0);

    CHECK(kdse16_validate(2u) == KDSE_STATUS_INVALID_PAYLOAD_LENGTH);
    CHECK(kdse16_validate(4u) ==
          KDSE_STATUS_EXPLICIT_FINAL_TERMINAL_LEVEL);
    CHECK(kdse16_validate(17u) ==
          KDSE_STATUS_DATA_AFTER_TERMINAL_LEVEL);
    CHECK(kdse16_validate(31u) == KDSE_STATUS_TRUNCATED_LEVEL);
    CHECK(kdse16_decode_profile(0u, NULL) == KDSE_STATUS_NULL_ARGUMENT);
    CHECK(kdse16_canonicalize(0u, NULL) == KDSE_STATUS_NULL_ARGUMENT);
    CHECK(kdse16_is_ordered(0u, NULL) == KDSE_STATUS_NULL_ARGUMENT);
    CHECK(kdse16_canonicalize(4u, &ordered) ==
          KDSE_STATUS_EXPLICIT_FINAL_TERMINAL_LEVEL);
    CHECK(ordered == UINT16_MAX);
    CHECK(kdse16_is_ordered(4u, &is_ordered) ==
          KDSE_STATUS_EXPLICIT_FINAL_TERMINAL_LEVEL);
    CHECK(!is_ordered);
    CHECK(kdse16_payload(UINT16_C(0x8005)) == 5u);
    CHECK(kdse16_payload_length(0u) == 1u);
    CHECK(kdse16_payload_length(UINT16_C(0x8005)) == 3u);
}

int main(void)
{
    test_complete_space();
    test_profiles_and_errors();

    if (failures != 0) {
        (void)fprintf(stderr, "KDSE-16 checked tests: %d failure(s)\n",
                      failures);
        return 1;
    }
    (void)puts("KDSE-16 checked tests: pass (4,397 valid; 198 Ordered)");
    return 0;
}
