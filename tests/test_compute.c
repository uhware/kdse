/*
 * Copyright (C) 2026 Mark Karaman
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This file is part of KDSE.
 */

#include "kdse/kdse8_checked.h"
#include "kdse/kdse8_compute.h"

#include <math.h>
#include <stdint.h>
#include <stdio.h>

#define ARRAY_LENGTH(a) (sizeof(a) / sizeof((a)[0]))

static int failures = 0;

#define CHECK(condition) do { \
    if (!(condition)) { \
        (void)fprintf(stderr, "%s:%d: check failed: %s\n", \
                      __FILE__, __LINE__, #condition); \
        ++failures; \
    } \
} while (0)

static double reference_compute(uint8_t payload,
                                double input,
                                double threshold,
                                double branch_loss)
{
    kdse8_profile_t profile;
    const double child_fraction = (1.0 - branch_loss) * 0.5;
    double terminal_value = input;
    double output = 0.0;
    uint8_t depth;

    if (kdse8_decode_profile(payload, &profile) != KDSE_STATUS_OK) {
        return -1.0;
    }
    for (depth = 0u; depth <= profile.max_depth; ++depth) {
        if (terminal_value >= threshold) {
            output += (double)profile.terminals[depth] * terminal_value;
        }
        terminal_value *= child_fraction;
    }
    return output;
}

static bool close_enough(double actual, double expected)
{
    const double scale = fabs(expected) > 1.0 ? fabs(expected) : 1.0;
    return fabs(actual - expected) <= 1.0e-12 * scale;
}

static void test_complete_space(void)
{
    static const double inputs[] = {0.0, 0.01, 0.3, 1.0, 5.0, 100.0};
    static const double thresholds[] = {0.01, 0.3, 2.0};
    static const double losses[] = {0.0, 0.17, 0.5, 0.99};
    unsigned payload;

    for (payload = 0u; payload <= KDSE8_PAYLOAD_MASK; ++payload) {
        size_t input_index;

        if (kdse8_validate((uint8_t)payload) != KDSE_STATUS_OK) {
            continue;
        }
        for (input_index = 0u;
             input_index < ARRAY_LENGTH(inputs);
             ++input_index) {
            size_t threshold_index;

            for (threshold_index = 0u;
                 threshold_index < ARRAY_LENGTH(thresholds);
                 ++threshold_index) {
                size_t loss_index;

                for (loss_index = 0u;
                     loss_index < ARRAY_LENGTH(losses);
                     ++loss_index) {
                    const double expected = reference_compute(
                        (uint8_t)payload,
                        inputs[input_index],
                        thresholds[threshold_index],
                        losses[loss_index]);
                    const double actual = kdse8_compute(
                        (uint8_t)payload,
                        inputs[input_index],
                        thresholds[threshold_index],
                        losses[loss_index]);
                    const double with_bit7 = kdse8_compute(
                        (uint8_t)(payload | 0x80u),
                        inputs[input_index],
                        thresholds[threshold_index],
                        losses[loss_index]);

                    CHECK(close_enough(actual, expected));
                    CHECK(close_enough(with_bit7, expected));
                }
            }
        }
    }
}

static void test_simple_responses(void)
{
    CHECK(kdse8_compute(0u, 1.0, 2.0, 0.75) == 0.0);
    CHECK(kdse8_compute(0u, 2.0, 2.0, 0.75) == 2.0);
    CHECK(kdse8_compute(1u, 4.0, 2.0, 0.0) == 4.0);
    CHECK(kdse8_compute(1u, 3.0, 2.0, 0.0) == 0.0);
    CHECK(close_enough(kdse8_compute(119u, 5.0, 0.3, 0.17),
                       3.00532625));
}

int main(void)
{
    test_complete_space();
    test_simple_responses();

    if (failures != 0) {
        (void)fprintf(stderr, "compute tests: %d failure(s)\n", failures);
        return 1;
    }
    (void)puts("compute tests: pass");
    return 0;
}
