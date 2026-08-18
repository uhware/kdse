/*
 * Copyright (C) 2026 Mark Karaman
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This file is part of KDSE.
 */

#ifndef KDSE8_COMPUTE_H
#define KDSE8_COMPUTE_H

#include "kdse/kdse8.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Evaluate thresholded equal-split propagation with uniform branch loss.
 *
 * This is the streamlined trusted-input path. It performs no KDSE validity
 * check, no canonicalization, no assertion of validity, and no malformed-input
 * recovery. The supplied KDSE-8 value must already be valid. Ordered form is
 * not required. Bit 7 remains outside the payload and is ignored.
 *
 * Each branch retains (1 - branch_loss) of its input and splits that retained
 * value equally between two children. A terminal contributes its received
 * value when that value is greater than or equal to threshold.
 *
 * Preconditions:
 * - validated_value contains a valid minimal-form KDSE-8 payload;
 * - input is finite and nonnegative;
 * - threshold is finite and greater than zero;
 * - branch_loss is finite and in the half-open interval [0, 1).
 *
 * Violating a precondition produces an unspecified result. No storage is
 * allocated or retained.
 *
 * @param validated_value Previously validated KDSE-8 container.
 * @param input Nonnegative root input magnitude.
 * @param threshold Positive terminal activation threshold.
 * @param branch_loss Per-branch loss fraction, not a percentage.
 * @return Sum of all active terminal contributions.
 */
double kdse8_compute(kdse8_t validated_value,
                          double input,
                          double threshold,
                          double branch_loss);

#ifdef __cplusplus
}
#endif

#endif
