/*
 * Copyright (C) 2026 Mark Karaman
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This file is part of KDSE.
 */

#include "kdse/kdse8_compute.h"

#include <stdint.h>

static uint8_t trusted_payload_length(uint8_t payload)
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

double kdse8_compute(kdse8_t validated_value,
                          double input,
                          double threshold,
                          double branch_loss)
{
    const uint8_t payload =
        (uint8_t)(validated_value & KDSE8_PAYLOAD_MASK);
    const uint8_t length = trusted_payload_length(payload);
    const double child_fraction = (1.0 - branch_loss) * 0.5;
    double terminal_value = input;
    double output = 0.0;
    uint8_t position = 0u;
    uint8_t width = 1u;

    for (;;) {
        uint8_t branches = 0u;
        uint8_t index;

        for (index = 0u; index < width; ++index) {
            const uint8_t shift = (uint8_t)((unsigned)length -
                (unsigned)position - (unsigned)index - 1u);
            const uint8_t branches_here =
                (uint8_t)((payload >> shift) & UINT8_C(1));

            branches = (uint8_t)(branches + branches_here);
            if (branches_here == 0u && terminal_value >= threshold) {
                output += terminal_value;
            }
        }
        position = (uint8_t)(position + width);
        terminal_value *= child_fraction;

        if (position == length) {
            if (terminal_value >= threshold) {
                output += (double)(2u * branches) * terminal_value;
            }
            return output;
        }
        width = (uint8_t)(2u * branches);
    }
}
