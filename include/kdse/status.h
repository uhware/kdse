/*
 * Copyright (C) 2026 Mark Karaman
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This file is part of KDSE.
 */

#ifndef KDSE_STATUS_H
#define KDSE_STATUS_H

#ifdef __cplusplus
extern "C" {
#endif

/** Result codes returned by the checked KDSE admission boundary. */
typedef enum kdse_status {
    KDSE_STATUS_OK = 0,
    KDSE_STATUS_NULL_ARGUMENT,
    KDSE_STATUS_INVALID_PAYLOAD_LENGTH,
    KDSE_STATUS_TRUNCATED_LEVEL,
    KDSE_STATUS_DATA_AFTER_TERMINAL_LEVEL,
    KDSE_STATUS_EXPLICIT_FINAL_TERMINAL_LEVEL
} kdse_status_t;

/**
 * Return a stable human-readable description of a KDSE status code.
 *
 * @param status Status code returned by a checked API.
 * @return Pointer to immutable static storage. The caller must not free it.
 */
static inline const char *kdse_status_string(kdse_status_t status)
{
    switch (status) {
    case KDSE_STATUS_OK:
        return "valid KDSE payload";
    case KDSE_STATUS_NULL_ARGUMENT:
        return "null output argument";
    case KDSE_STATUS_INVALID_PAYLOAD_LENGTH:
        return "invalid natural-width payload length";
    case KDSE_STATUS_TRUNCATED_LEVEL:
        return "truncated breadth-first level";
    case KDSE_STATUS_DATA_AFTER_TERMINAL_LEVEL:
        return "data follows a level with no branches";
    case KDSE_STATUS_EXPLICIT_FINAL_TERMINAL_LEVEL:
        return "final all-terminal level must be omitted";
    default:
        return "unknown KDSE status";
    }
}

#ifdef __cplusplus
}
#endif

#endif
