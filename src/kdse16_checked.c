/*
 * Copyright (C) 2026 Mark Karaman
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This file is part of KDSE.
 */

#include "kdse/kdse16_checked.h"

#include <stddef.h>
#include <string.h>

typedef struct kdse16_tree_node {
    uint8_t branch;
    uint8_t child[2];
} kdse16_tree_node_t;

typedef struct kdse16_tree {
    kdse16_tree_node_t nodes[KDSE16_MAX_NODES];
    uint8_t count;
} kdse16_tree_t;

static uint8_t bit_length(uint16_t payload)
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

static uint8_t payload_bit(uint16_t payload,
                           uint8_t length,
                           uint8_t position)
{
    const uint8_t shift = (uint8_t)((unsigned)length -
        (unsigned)position - 1u);
    return (uint8_t)((payload >> shift) & UINT16_C(1));
}

uint16_t kdse16_payload(kdse16_t value)
{
    return (uint16_t)(value & KDSE16_PAYLOAD_MASK);
}

uint8_t kdse16_payload_length(kdse16_t value)
{
    return bit_length(kdse16_payload(value));
}

kdse_status_t kdse16_validate(kdse16_t value)
{
    const uint16_t payload = kdse16_payload(value);
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
            branches = (uint8_t)(branches + payload_bit(
                payload, length, (uint8_t)(position + index)));
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

static void decode_profile_unchecked(uint16_t payload,
                                     kdse16_profile_t *profile)
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
            branches = (uint8_t)(branches + payload_bit(
                payload, length, (uint8_t)(position + index)));
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

kdse_status_t kdse16_decode_profile(kdse16_t value,
                                         kdse16_profile_t *out)
{
    kdse16_profile_t decoded;
    kdse_status_t status;

    if (out == NULL) {
        return KDSE_STATUS_NULL_ARGUMENT;
    }
    status = kdse16_validate(value);
    if (status != KDSE_STATUS_OK) {
        return status;
    }
    decode_profile_unchecked(kdse16_payload(value), &decoded);
    *out = decoded;
    return KDSE_STATUS_OK;
}

static void build_tree_unchecked(uint16_t payload, kdse16_tree_t *tree)
{
    const uint8_t length = bit_length(payload);
    uint8_t queue[KDSE16_MAX_NODES];
    uint8_t head = 0u;
    uint8_t tail = 1u;
    uint8_t position;

    memset(tree, 0, sizeof(*tree));
    tree->count = 1u;
    queue[0] = 0u;

    for (position = 0u; position < length; ++position) {
        const uint8_t node_index = queue[head++];

        if (payload_bit(payload, length, position) != 0u) {
            kdse16_tree_node_t *node = &tree->nodes[node_index];
            node->branch = 1u;
            node->child[0] = tree->count++;
            node->child[1] = tree->count++;
            queue[tail++] = node->child[0];
            queue[tail++] = node->child[1];
        }
    }
}

static uint8_t serialize_forest(const kdse16_tree_t *tree,
                                uint8_t first,
                                uint8_t second,
                                uint8_t bits[KDSE16_MAX_NODES])
{
    uint8_t queue[KDSE16_MAX_NODES];
    uint8_t head = 0u;
    uint8_t tail = 2u;
    uint8_t length = 0u;

    queue[0] = first;
    queue[1] = second;
    while (head < tail) {
        const kdse16_tree_node_t *node = &tree->nodes[queue[head++]];

        bits[length++] = node->branch;
        if (node->branch != 0u) {
            queue[tail++] = node->child[0];
            queue[tail++] = node->child[1];
        }
    }
    return length;
}

static void canonicalize_node(kdse16_tree_t *tree, uint8_t node_index)
{
    kdse16_tree_node_t *node = &tree->nodes[node_index];
    uint8_t forward[KDSE16_MAX_NODES];
    uint8_t reverse[KDSE16_MAX_NODES];
    uint8_t forward_length;
    uint8_t reverse_length;

    if (node->branch == 0u) {
        return;
    }
    canonicalize_node(tree, node->child[0]);
    canonicalize_node(tree, node->child[1]);
    forward_length = serialize_forest(
        tree, node->child[0], node->child[1], forward);
    reverse_length = serialize_forest(
        tree, node->child[1], node->child[0], reverse);

    if (forward_length == reverse_length &&
        memcmp(reverse, forward, forward_length) < 0) {
        const uint8_t swap = node->child[0];
        node->child[0] = node->child[1];
        node->child[1] = swap;
    }
}

static uint16_t serialize_minimal(const kdse16_tree_t *tree)
{
    uint8_t queue[KDSE16_MAX_NODES];
    uint8_t head = 0u;
    uint8_t tail = 1u;
    uint32_t payload = 0u;

    queue[0] = 0u;
    while (head < tail) {
        const uint8_t level_start = head;
        const uint8_t width = (uint8_t)(tail - head);
        uint8_t branches = 0u;
        uint8_t index;

        for (index = 0u; index < width; ++index) {
            const kdse16_tree_node_t *node = &tree->nodes[queue[head++]];
            branches = (uint8_t)(branches + node->branch);
            if (node->branch != 0u) {
                queue[tail++] = node->child[0];
                queue[tail++] = node->child[1];
            }
        }
        if (branches == 0u) {
            break;
        }
        for (index = 0u; index < width; ++index) {
            const kdse16_tree_node_t *node =
                &tree->nodes[queue[(uint8_t)(level_start + index)]];
            payload = (payload << 1u) | (uint32_t)node->branch;
        }
    }
    return (uint16_t)payload;
}

kdse_status_t kdse16_canonicalize(kdse16_t value,
                                       kdse16_t *ordered)
{
    kdse16_tree_t tree;
    kdse_status_t status;

    if (ordered == NULL) {
        return KDSE_STATUS_NULL_ARGUMENT;
    }
    status = kdse16_validate(value);
    if (status != KDSE_STATUS_OK) {
        return status;
    }
    build_tree_unchecked(kdse16_payload(value), &tree);
    canonicalize_node(&tree, 0u);
    *ordered = serialize_minimal(&tree);
    return KDSE_STATUS_OK;
}

kdse_status_t kdse16_is_ordered(kdse16_t value, bool *is_ordered)
{
    kdse16_t ordered;
    kdse_status_t status;

    if (is_ordered == NULL) {
        return KDSE_STATUS_NULL_ARGUMENT;
    }
    status = kdse16_canonicalize(value, &ordered);
    if (status != KDSE_STATUS_OK) {
        return status;
    }
    *is_ordered = kdse16_payload(value) == ordered;
    return KDSE_STATUS_OK;
}
