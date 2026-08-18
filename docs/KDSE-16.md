<!--
Copyright (C) 2026 Mark Karaman
SPDX-License-Identifier: CC-BY-SA-4.0
Source-code and command samples: AGPL-3.0-or-later
-->

# KDSE-16 representation and computation

## Container

KDSE-16 is the binary container class `KDSE[2,15;16,2]`:

- structural arity `q = 2`;
- at most 15 explicit branch/terminal symbols;
- one 16-bit physical container;
- payload right-aligned in bits 0 through 14; and
- bit 15 unassigned and ignored.

For example, `0x0005` and `0x8005` both contain the natural-width payload
`101`. Valid minimal-form payload lengths are 1, 3, 5, 7, 9, 11, 13, and 15.
Leading container zeros are padding and are not payload symbols.

## Encoding and validity

KDSE-16 uses the same binary breadth-first structure as KDSE-8. Each explicit
node contributes one bit: `1` for a branch with two ordered children and `0`
for a terminal. A level containing `b` branches creates exactly `2b` positions
in the next level. The deterministic final all-terminal level is omitted.

A payload is valid when every derived level is complete, the payload ends on a
level boundary, no data follows a zero-branch level, and—except for the
isolated terminal payload `0`—the final encoded level contains at least one
branch.

The 15-bit payload space contains **4,397 valid minimal-form payloads**. Under
lowest-numerical sibling canonicalization, they collapse to **198 Ordered
forms**. These counts are verified exhaustively by `tests/test_checked16.c`.

## Structural limits

| Attribute | KDSE-16 limit |
|---|---:|
| Payload bits | 15 |
| Reconstructed nodes | 31 |
| Branches | 15 |
| Terminals | 16 |
| Maximum terminal depth | 8 |

The maximum-depth payload is the chain `101010101010101`, whose terminal-depth
profile is `[0,1,1,1,1,1,1,1,2]`. The payload `111111111111111` reconstructs
a complete binary tree with 16 implicit terminals at depth 4.

## Ordered form

Ordered KDSE-16 is the lowest-numerical valid breadth-first serialization among
all independent sibling subtree permutations. Ordered form supplies canonical
identity; it is not required for validity or completion.

`kdse16_canonicalize` canonicalizes structurally. It does not require a
stored table of the 198 Ordered values.

## Threshold and branch-loss operator

`kdse16_compute` uses the same operator and numeric contract as the
KDSE-8 compute function. For input `n`, threshold `T`, and branch-loss
fraction `loss`, a terminal at depth `d` receives:

```text
n * ((1 - loss) / 2)^d
```

It contributes that value when the value is at least `T`. All active terminal
contributions are summed.

The function is a trusted-input path: the KDSE-16 value must already be valid.
It performs no validation, validity assertion, canonicalization, or malformed-
input recovery. Use `kdse16_validate` at the admission boundary.

---

*Copyright © 2026 Mark Karaman · Documentation: CC BY-SA 4.0 · Source samples: AGPL-3.0-or-later*  
*U.S. Patent Pending — Application No. 64/131,240*  
*Commercial licensing: licensing@uhware.com · Security: security@uhware.com*  
*Companion papers: see [docs/papers/](papers/).*
