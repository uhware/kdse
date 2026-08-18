/*
 * Copyright (C) 2026 Mark Karaman
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This file is part of KDSE.
 */

#ifndef KDSE16_CHECKED_H
#define KDSE16_CHECKED_H

#include <stdbool.h>
#include <stdint.h>

#include "kdse/kdse16.h"
#include "kdse/status.h"

#ifdef __cplusplus
extern "C" {
#endif

/** Structural profile reconstructed from a valid KDSE-16 payload. */
typedef struct kdse16_profile {
    /** Terminal count at each depth, including implicit final terminals. */
    uint8_t terminals[KDSE16_MAX_DEPTH + 1u];
    /** Actual natural-width payload length: 1, 3, 5, ..., or 15. */
    uint8_t payload_length;
    /** Deepest occupied terminal depth. */
    uint8_t max_depth;
    /** Number of explicit branch nodes in the payload. */
    uint8_t branch_count;
    /** Total reconstructed terminals, explicit and implicit. */
    uint8_t terminal_count;
} kdse16_profile_t;

/**
 * Extract the 15-bit payload from a KDSE-16 container.
 *
 * @param value Physical KDSE-16 container. Bit 15 is ignored.
 * @return The value of bits 0 through 14, in the range 0 through 32767.
 */
uint16_t kdse16_payload(kdse16_t value);

/**
 * Return the natural-width length of the extracted payload.
 *
 * The isolated terminal value 0 has length 1 by definition. This function
 * reports representation length only and does not validate the payload.
 *
 * @param value Physical KDSE-16 container. Bit 15 is ignored.
 * @return A length from 1 through 15.
 */
uint8_t kdse16_payload_length(kdse16_t value);

/**
 * Validate a KDSE-16 minimal-form payload.
 *
 * @param value Physical KDSE-16 container. Bit 15 is ignored.
 * @return KDSE_STATUS_OK when the lower 15 bits contain a complete,
 *         minimal-form KDSE-16 payload; otherwise a specific error code.
 */
kdse_status_t kdse16_validate(kdse16_t value);

/**
 * Decode the terminal-depth profile of a validated KDSE-16 payload.
 *
 * This checked function validates before decoding. On failure, *out is
 * unchanged. No storage is retained by the library.
 *
 * @param value Physical KDSE-16 container. Bit 15 is ignored.
 * @param out Destination profile owned by the caller; must not be NULL.
 * @return KDSE_STATUS_OK on success, or a validation/argument error.
 */
kdse_status_t kdse16_decode_profile(kdse16_t value,
                                         kdse16_profile_t *out);

/**
 * Find the lowest-numerical sibling-permutation representative.
 *
 * This checked function validates value first. The returned Ordered value is
 * the 15-bit KDSE payload; its unassigned container bit is cleared. On
 * failure, *ordered is unchanged. No storage is retained by the library.
 *
 * @param value Physical KDSE-16 container. Bit 15 is ignored.
 * @param ordered Destination owned by the caller; must not be NULL.
 * @return KDSE_STATUS_OK on success, or a validation/argument error.
 */
kdse_status_t kdse16_canonicalize(kdse16_t value,
                                       kdse16_t *ordered);

/**
 * Determine whether a KDSE-16 payload is already Ordered.
 *
 * This checked function validates value first. Bit 15 does not participate
 * in the comparison. On failure, *is_ordered is unchanged.
 *
 * @param value Physical KDSE-16 container. Bit 15 is ignored.
 * @param is_ordered Destination owned by the caller; must not be NULL.
 * @return KDSE_STATUS_OK on success, or a validation/argument error.
 */
kdse_status_t kdse16_is_ordered(kdse16_t value, bool *is_ordered);

#ifdef __cplusplus
}
#endif

#endif
