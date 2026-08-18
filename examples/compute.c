/*
 * Copyright (C) 2026 Mark Karaman
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This file is part of KDSE.
 */

#include "kdse/kdse8_checked.h"
#include "kdse/kdse8_compute.h"

#include <stdint.h>
#include <stdio.h>

int main(void)
{
    const kdse8_t kdse = UINT8_C(0x77); /* payload 1110111 */
    const double input = 5.0;
    const double threshold = 0.30;
    const double branch_loss = 0.17;
    const kdse_status_t status = kdse8_validate(kdse);

    if (status != KDSE_STATUS_OK) {
        (void)fprintf(stderr, "invalid KDSE-8: %s\n",
                      kdse_status_string(status));
        return 1;
    }

    (void)printf("output = %.9f\n",
                 kdse8_compute(kdse, input, threshold, branch_loss));
    return 0;
}
