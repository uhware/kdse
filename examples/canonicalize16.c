/*
 * Copyright (C) 2026 Mark Karaman
 * SPDX-License-Identifier: AGPL-3.0-or-later
 *
 * This file is part of KDSE.
 */

#include "kdse/kdse16_checked.h"

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

int main(void)
{
    const kdse16_t kdse = UINT16_C(6); /* payload 110 */
    kdse16_t ordered;
    const kdse_status_t status = kdse16_canonicalize(kdse, &ordered);

    if (status != KDSE_STATUS_OK) {
        (void)fprintf(stderr, "invalid KDSE-16: %s\n",
                      kdse_status_string(status));
        return 1;
    }

    (void)printf("ordered decimal payload = %" PRIu16 "\n", ordered);
    return 0;
}
