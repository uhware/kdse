<!--
Copyright (C) 2026 Mark Karaman
SPDX-License-Identifier: CC-BY-SA-4.0
Source-code samples in this file: AGPL-3.0-or-later
-->

# KDSE-8 and KDSE-16 C API

Every public function is declared and fully documented in `include/kdse/`.
The API allocates no memory, retains no pointers, and transfers no ownership.

## Common status API

Include `kdse/status.h` for `kdse_status_t` and `kdse_status_string`. The status
codes are shared by both checked libraries:

- `KDSE_STATUS_OK`
- `KDSE_STATUS_NULL_ARGUMENT`
- `KDSE_STATUS_INVALID_PAYLOAD_LENGTH`
- `KDSE_STATUS_TRUNCATED_LEVEL`
- `KDSE_STATUS_DATA_AFTER_TERMINAL_LEVEL`
- `KDSE_STATUS_EXPLICIT_FINAL_TERMINAL_LEVEL`

`kdse_status_string` returns immutable static storage; callers must not free it.

## KDSE-8 common representation

Include `kdse/kdse8.h` for:

- `kdse8_t` — an eight-bit physical KDSE-8 container;
- `KDSE8_PAYLOAD_MASK` — mask for bits 0 through 6;
- `KDSE8_MAX_DEPTH` — deepest possible terminal depth; and
- `KDSE8_MAX_NODES` — largest reconstructed full tree.

Bit 7 is unassigned and ignored throughout the API.

## KDSE-8 checked boundary library

Header: `kdse/kdse8_checked.h`  
Library: `libkdse8_checked.a`

| Function | Purpose | Success result | Failure behavior |
|---|---|---|---|
| `kdse8_payload` | Extract the lower seven payload bits | Payload `0..127` | Cannot fail |
| `kdse8_payload_length` | Recover natural-width length; `0` has length 1 | Length `1..7` | Does not validate |
| `kdse8_validate` | Enforce complete, minimal-form KDSE-8 structure | `KDSE_STATUS_OK` | Specific status code |
| `kdse8_decode_profile` | Validate and reconstruct terminal counts by depth | Populates caller-owned profile | Leaves output unchanged |
| `kdse8_canonicalize` | Validate and find lowest-numerical sibling-permutation representative | Writes Ordered payload with bit 7 clear | Leaves output unchanged |
| `kdse8_is_ordered` | Validate and test canonical identity | Writes `true` or `false` | Leaves output unchanged |

## KDSE-8 streamlined compute library

Header: `kdse/kdse8_compute.h`  
Library: `libkdse8_compute.a`

```c
double kdse8_compute(kdse8_t validated_value,
                          double input,
                          double threshold,
                          double branch_loss);
```

Parameters:

- `validated_value` — KDSE-8 that the caller has already validated; bit 7 is ignored.
- `input` — finite root magnitude, `input >= 0`.
- `threshold` — finite terminal threshold, `threshold > 0`.
- `branch_loss` — finite loss fraction, `0 <= branch_loss < 1`.

The return value is the sum of all active terminal contributions. The function
performs no validation of either the KDSE payload or numeric preconditions.
Violating a precondition produces an unspecified result.

A standard checked-to-trusted call sequence is:

```c
kdse_status_t status = kdse8_validate(candidate);
if (status == KDSE_STATUS_OK) {
    double result = kdse8_compute(candidate, n, threshold, loss);
}
```

## KDSE-16 common representation

Include `kdse/kdse16.h` for:

- `kdse16_t` — a 16-bit physical KDSE-16 container;
- `KDSE16_PAYLOAD_MASK` — mask for bits 0 through 14;
- `KDSE16_MAX_DEPTH` — deepest possible terminal depth; and
- `KDSE16_MAX_NODES` — largest reconstructed full tree.

Bit 15 is unassigned and ignored throughout the KDSE-16 API.

## KDSE-16 checked boundary library

Header: `kdse/kdse16_checked.h`  
Library: `libkdse16_checked.a`

| Function | Purpose | Success result | Failure behavior |
|---|---|---|---|
| `kdse16_payload` | Extract the lower 15 payload bits | Payload `0..32767` | Cannot fail |
| `kdse16_payload_length` | Recover natural-width length; `0` has length 1 | Length `1..15` | Does not validate |
| `kdse16_validate` | Enforce complete, minimal-form KDSE-16 structure | `KDSE_STATUS_OK` | Specific status code |
| `kdse16_decode_profile` | Validate and reconstruct terminal counts by depth | Populates caller-owned profile | Leaves output unchanged |
| `kdse16_canonicalize` | Validate and find the lowest-numerical sibling-permutation representative | Writes Ordered payload with bit 15 clear | Leaves output unchanged |
| `kdse16_is_ordered` | Validate and test canonical identity | Writes `true` or `false` | Leaves output unchanged |

## KDSE-16 streamlined compute library

Header: `kdse/kdse16_compute.h`  
Library: `libkdse16_compute.a`

```c
double kdse16_compute(kdse16_t validated_value,
                           double input,
                           double threshold,
                           double branch_loss);
```

Parameters and operator behavior match `kdse8_compute`, except that
`validated_value` is a previously validated KDSE-16 container and bit 15 is
ignored. The function performs no validation of the payload or numeric
preconditions.

```c
kdse_status_t status = kdse16_validate(candidate);
if (status == KDSE_STATUS_OK) {
    double result = kdse16_compute(candidate, n, threshold, loss);
}
```

---

*Copyright © 2026 Mark Karaman · Documentation: CC BY-SA 4.0 · Source samples: AGPL-3.0-or-later*  
*U.S. Patent Pending — Application No. 64/131,240*  
*Commercial licensing: licensing@uhware.com · Security: security@uhware.com*  
*Companion papers: see [docs/papers/](papers/).*
