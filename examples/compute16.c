/*
 * Copyright (C) 2026 Mark Karaman
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This file is part of KDSE.
 */

#include "kdse/kdse16_checked.h"
#include "kdse/kdse16_compute.h"

#include <stdint.h>
#include <stdio.h>

int main(void)
{
    const kdse16_t kdse = UINT16_C(0x5555); /* 101010101010101 */
    const double input = 100.0;
    const double threshold = 0.30;
    const double branch_loss = 0.17;
    const kdse_status_t status = kdse16_validate(kdse);

    if (status != KDSE_STATUS_OK) {
        (void)fprintf(stderr, "invalid KDSE-16: %s\n",
                      kdse_status_string(status));
        return 1;
    }

    (void)printf("output = %.9f\n",
                 kdse16_compute(kdse, input, threshold, branch_loss));
    return 0;
}
