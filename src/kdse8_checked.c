/*
 * Copyright (C) 2026 Mark Karaman
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This file is part of KDSE.
 */

#include "kdse/kdse8_checked.h"

#include <stddef.h>
#include <string.h>

typedef struct kdse_tree_node {
    uint8_t branch;
    uint8_t child[2];
} kdse_tree_node_t;

typedef struct kdse_tree {
    kdse_tree_node_t nodes[KDSE8_MAX_NODES];
    uint8_t count;
} kdse_tree_t;

static uint8_t bit_length(uint8_t payload)
{
    uint8_t length = 0u;

    if (payload == 0u) {
        return 1u;
    }
    while (payload != 0u) {
        ++length;
        payload = (uint8_t)(payload >> 1u);
    }
    return length;
}

static uint8_t payload_bit(uint8_t payload, uint8_t length, uint8_t position)
{
    const uint8_t shift = (uint8_t)(length - position - 1u);
    return (uint8_t)((payload >> shift) & UINT8_C(1));
}

uint8_t kdse8_payload(kdse8_t value)
{
    return (uint8_t)(value & KDSE8_PAYLOAD_MASK);
}

uint8_t kdse8_payload_length(kdse8_t value)
{
    return bit_length(kdse8_payload(value));
}

kdse_status_t kdse8_validate(kdse8_t value)
{
    const uint8_t payload = kdse8_payload(value);
    const uint8_t length = bit_length(payload);
    uint8_t position = 0u;
    uint8_t width = 1u;

    if ((length & UINT8_C(1)) == 0u) {
        return KDSE_STATUS_INVALID_PAYLOAD_LENGTH;
    }

    for (;;) {
        uint8_t branches = 0u;
        uint8_t index;

        if ((uint8_t)(position + width) > length) {
            return KDSE_STATUS_TRUNCATED_LEVEL;
        }
        for (index = 0u; index < width; ++index) {
            branches = (uint8_t)(branches +
                payload_bit(payload, length, (uint8_t)(position + index)));
        }
        position = (uint8_t)(position + width);

        if (position == length) {
            if (payload == 0u || branches != 0u) {
                return KDSE_STATUS_OK;
            }
            return KDSE_STATUS_EXPLICIT_FINAL_TERMINAL_LEVEL;
        }
        if (branches == 0u) {
            return KDSE_STATUS_DATA_AFTER_TERMINAL_LEVEL;
        }
        width = (uint8_t)(2u * branches);
    }
}

static void decode_profile_unchecked(uint8_t payload,
                                     kdse8_profile_t *profile)
{
    const uint8_t length = bit_length(payload);
    uint8_t position = 0u;
    uint8_t width = 1u;
    uint8_t depth = 0u;

    memset(profile, 0, sizeof(*profile));
    profile->payload_length = length;

    for (;;) {
        uint8_t branches = 0u;
        uint8_t index;

        for (index = 0u; index < width; ++index) {
            branches = (uint8_t)(branches +
                payload_bit(payload, length, (uint8_t)(position + index)));
        }
        profile->terminals[depth] =
            (uint8_t)(profile->terminals[depth] + width - branches);
        profile->branch_count =
            (uint8_t)(profile->branch_count + branches);
        position = (uint8_t)(position + width);

        if (position == length) {
            if (branches != 0u) {
                profile->terminals[depth + 1u] =
                    (uint8_t)(2u * branches);
                profile->max_depth = (uint8_t)(depth + 1u);
            } else {
                profile->max_depth = depth;
            }
            break;
        }
        width = (uint8_t)(2u * branches);
        ++depth;
    }

    for (depth = 0u; depth <= profile->max_depth; ++depth) {
        profile->terminal_count =
            (uint8_t)(profile->terminal_count + profile->terminals[depth]);
    }
}

kdse_status_t kdse8_decode_profile(kdse8_t value,
                                        kdse8_profile_t *out)
{
    kdse8_profile_t decoded;
    kdse_status_t status;

    if (out == NULL) {
        return KDSE_STATUS_NULL_ARGUMENT;
    }
    status = kdse8_validate(value);
    if (status != KDSE_STATUS_OK) {
        return status;
    }
    decode_profile_unchecked(kdse8_payload(value), &decoded);
    *out = decoded;
    return KDSE_STATUS_OK;
}

static void build_tree_unchecked(uint8_t payload, kdse_tree_t *tree)
{
    const uint8_t length = bit_length(payload);
    uint8_t queue[KDSE8_MAX_NODES];
    uint8_t queue_head = 0u;
    uint8_t queue_tail = 1u;
    uint8_t position;

    memset(tree, 0, sizeof(*tree));
    tree->count = 1u;
    queue[0] = 0u;

    for (position = 0u; position < length; ++position) {
        const uint8_t node_index = queue[queue_head++];

        if (payload_bit(payload, length, position) != 0u) {
            kdse_tree_node_t *node = &tree->nodes[node_index];
            node->branch = 1u;
            node->child[0] = tree->count++;
            node->child[1] = tree->count++;
            queue[queue_tail++] = node->child[0];
            queue[queue_tail++] = node->child[1];
        }
    }
}

static size_t canonical_key(const kdse_tree_t *tree,
                            uint8_t node_index,
                            char key[KDSE8_MAX_NODES + 1u])
{
    const kdse_tree_node_t *node = &tree->nodes[node_index];
    char left[KDSE8_MAX_NODES + 1u];
    char right[KDSE8_MAX_NODES + 1u];
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

    left_length = canonical_key(tree, node->child[0], left);
    right_length = canonical_key(tree, node->child[1], right);
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

static void unordered_key(uint8_t payload,
                          char key[KDSE8_MAX_NODES + 1u])
{
    kdse_tree_t tree;

    build_tree_unchecked(payload, &tree);
    (void)canonical_key(&tree, 0u, key);
}

kdse_status_t kdse8_canonicalize(kdse8_t value,
                                      kdse8_t *ordered)
{
    char wanted_key[KDSE8_MAX_NODES + 1u];
    uint16_t candidate;
    kdse_status_t status;

    if (ordered == NULL) {
        return KDSE_STATUS_NULL_ARGUMENT;
    }
    status = kdse8_validate(value);
    if (status != KDSE_STATUS_OK) {
        return status;
    }

    unordered_key(kdse8_payload(value), wanted_key);
    for (candidate = 0u; candidate <= KDSE8_PAYLOAD_MASK; ++candidate) {
        char candidate_key[KDSE8_MAX_NODES + 1u];

        if (kdse8_validate((kdse8_t)candidate) != KDSE_STATUS_OK) {
            continue;
        }
        unordered_key((uint8_t)candidate, candidate_key);
        if (strcmp(wanted_key, candidate_key) == 0) {
            *ordered = (kdse8_t)candidate;
            return KDSE_STATUS_OK;
        }
    }

    return KDSE_STATUS_TRUNCATED_LEVEL;
}

kdse_status_t kdse8_is_ordered(kdse8_t value, bool *is_ordered)
{
    kdse8_t ordered;
    kdse_status_t status;

    if (is_ordered == NULL) {
        return KDSE_STATUS_NULL_ARGUMENT;
    }
    status = kdse8_canonicalize(value, &ordered);
    if (status != KDSE_STATUS_OK) {
        return status;
    }
    *is_ordered = kdse8_payload(value) == ordered;
    return KDSE_STATUS_OK;
}
