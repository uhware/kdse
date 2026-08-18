/*
 * Copyright (C) 2026 Mark Karaman
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This file is part of KDSE.
 */

#ifndef KDSE8_CHECKED_H
#define KDSE8_CHECKED_H

#include <stdbool.h>
#include <stdint.h>

#include "kdse/kdse8.h"
#include "kdse/status.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Structural profile reconstructed from a valid KDSE-8 payload. */
typedef struct kdse8_profile {
    /** Terminal count at each depth, including implicit final terminals. */
    uint8_t terminals[KDSE8_MAX_DEPTH + 1u];
    /** Actual natural-width payload length: 1, 3, 5, or 7. */
    uint8_t payload_length;
    /** Deepest occupied terminal depth. */
    uint8_t max_depth;
    /** Number of explicit branch nodes in the payload. */
    uint8_t branch_count;
    /** Total reconstructed terminals, explicit and implicit. */
    uint8_t terminal_count;
} kdse8_profile_t;

/**
 * Extract the seven-bit payload from a KDSE-8 container.
 *
 * @param value Physical KDSE-8 container. Bit 7 is ignored.
 * @return The value of bits 0 through 6, in the range 0 through 127.
 */
uint8_t kdse8_payload(kdse8_t value);

/**
 * Return the natural-width length of the extracted payload.
 *
 * The isolated terminal value 0 has length 1 by definition. This function
 * reports representation length only and does not validate the payload.
 *
 * @param value Physical KDSE-8 container. Bit 7 is ignored.
 * @return A length from 1 through 7.
 */
uint8_t kdse8_payload_length(kdse8_t value);

/**
 * Validate a KDSE-8 minimal-form payload.
 *
 * @param value Physical KDSE-8 container. Bit 7 is ignored.
 * @return KDSE_STATUS_OK when the lower seven bits contain a complete,
 *         minimal-form KDSE-8 payload; otherwise a specific error code.
 */
kdse_status_t kdse8_validate(kdse8_t value);

/**
 * Decode the terminal-depth profile of a validated KDSE-8 payload.
 *
 * This checked function validates before decoding. On failure, *out is
 * unchanged. No storage is retained by the library.
 *
 * @param value Physical KDSE-8 container. Bit 7 is ignored.
 * @param out Destination profile owned by the caller; must not be NULL.
 * @return KDSE_STATUS_OK on success, or a validation/argument error.
 */
kdse_status_t kdse8_decode_profile(kdse8_t value,
                                        kdse8_profile_t *out);

/**
 * Find the lowest-numerical sibling-permutation representative.
 *
 * This checked function validates value first. The returned Ordered value is
 * the seven-bit KDSE payload; its unassigned container bit is cleared. On
 * failure, *ordered is unchanged. No storage is retained by the library.
 *
 * @param value Physical KDSE-8 container. Bit 7 is ignored.
 * @param ordered Destination owned by the caller; must not be NULL.
 * @return KDSE_STATUS_OK on success, or a validation/argument error.
 */
kdse_status_t kdse8_canonicalize(kdse8_t value,
                                      kdse8_t *ordered);

/**
 * Determine whether a KDSE-8 payload is already Ordered.
 *
 * This checked function validates value first. Bit 7 does not participate in
 * the comparison. On failure, *is_ordered is unchanged.
 *
 * @param value Physical KDSE-8 container. Bit 7 is ignored.
 * @param is_ordered Destination owned by the caller; must not be NULL.
 * @return KDSE_STATUS_OK on success, or a validation/argument error.
 */
kdse_status_t kdse8_is_ordered(kdse8_t value, bool *is_ordered);

#ifdef __cplusplus
}
#endif

#endif
