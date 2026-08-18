/*
 * Copyright (C) 2026 Mark Karaman
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This file is part of KDSE.
 */

#ifndef KDSE8_H
#define KDSE8_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Number of payload bits in a KDSE-8 container. */
#define KDSE8_PAYLOAD_BITS 7u

/** Mask selecting the seven-bit KDSE-8 payload. Bit 7 is unassigned. */
#define KDSE8_PAYLOAD_MASK UINT8_C(0x7f)

/** Largest terminal depth possible in a seven-bit binary KDSE payload. */
#define KDSE8_MAX_DEPTH 4u

/** Maximum number of reconstructed nodes, including implicit terminals. */
#define KDSE8_MAX_NODES 15u

/**
 * A KDSE-8 physical container.
 *
 * Bits 0 through 6 contain the right-aligned natural-width payload. Bit 7
 * is unassigned, is not part of KDSE, and is ignored by every KDSE API.
 */
typedef uint8_t kdse8_t;

#ifdef __cplusplus
}
#endif

#endif
