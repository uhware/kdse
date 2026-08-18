/*
 * Copyright (C) 2026 Mark Karaman
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This file is part of KDSE.
 */

#ifndef KDSE16_H
#define KDSE16_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Number of payload bits in a KDSE-16 container. */
#define KDSE16_PAYLOAD_BITS 15u

/** Mask selecting the 15-bit KDSE-16 payload. Bit 15 is unassigned. */
#define KDSE16_PAYLOAD_MASK UINT16_C(0x7fff)

/** Largest terminal depth possible in a 15-bit binary KDSE payload. */
#define KDSE16_MAX_DEPTH 8u

/** Maximum number of reconstructed nodes, including implicit terminals. */
#define KDSE16_MAX_NODES 31u

/**
 * A KDSE-16 physical container.
 *
 * Bits 0 through 14 contain the right-aligned natural-width payload. Bit 15
 * is unassigned, is not part of KDSE, and is ignored by every KDSE API.
 */
typedef uint16_t kdse16_t;

#ifdef __cplusplus
}
#endif

#endif
