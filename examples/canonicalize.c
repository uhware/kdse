/*
 * Copyright (C) 2026 Mark Karaman
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This file is part of KDSE.
 */

#include "kdse/kdse8_checked.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    const kdse8_t kdse = UINT8_C(6); /* payload 110 */
    kdse8_t ordered;
    const kdse_status_t status = kdse8_canonicalize(kdse, &ordered);

    if (status != KDSE_STATUS_OK) {
        (void)fprintf(stderr, "invalid KDSE-8: %s\n",
                      kdse_status_string(status));
        return 1;
    }

    (void)printf("ordered decimal payload = %" PRIu8 "\n", ordered);
    return 0;
}
