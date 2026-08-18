<!--
Copyright (C) 2026 Mark Karaman
SPDX-License-Identifier: CC-BY-SA-4.0
Source-code and command samples: AGPL-3.0-or-later
-->

# KDSE-8 representation and computation

## Container

KDSE-8 is the binary container class `KDSE[2,7;8,2]`:

- structural arity `q = 2`;
- at most seven explicit branch/terminal symbols;
- one eight-bit physical container;
- payload right-aligned in bits 0 through 6; and
- bit 7 unassigned and ignored.

The same KDSE payload is therefore obtained from `0x05` and `0x85`: both
contain the natural-width payload `101`.

## Encoding

Each explicit node contributes one bit:

- `1` — the node branches into two ordered children;
- `0` — the node is terminal.

Bits are concatenated breadth-first, level by level. If a level contains
`b` branches, the next level contains exactly `2b` explicit positions. The
final level of a finite full binary tree contains only terminals and is
deterministic, so minimal form omits it. The terminal root is represented by
the one-symbol payload `0`.

Examples:

| Payload | Derived levels | Meaning |
|---|---|---|
| `0` | `0` | isolated terminal root |
| `1` | `1` | root branch with two implicit terminals |
| `101` | levels `1`, `01` | terminal at depth 1 and two implicit terminals at depth 2 |
| `10101` | levels `1`, `01`, `01` | terminals at depths 1, 2, and 3 |

The vertical bars illustrate derived level boundaries; they are not stored.
Valid KDSE-8 payload lengths are 1, 3, 5, and 7.

## Minimal-form validity

A KDSE-8 payload is valid when:

1. every required breadth-first level is complete;
2. the payload ends exactly on a level boundary;
3. no symbols follow a level containing zero branches; and
4. except for `0`, the final encoded level contains at least one branch.

The fourth rule ensures that a deterministic final all-terminal level is not
stored explicitly.

## Ordered form

When sibling position is not semantically independent, sibling subtree
permutations can serialize the same unordered rooted topology differently.
Ordered KDSE-8 is the lowest-numerical valid payload among all such
permutations. Ordered form is canonical identity, not a validity requirement.

The complete KDSE-8 space contains 38 valid payloads and 13 Ordered forms. See
[catalog-8bit.md](catalog-8bit.md).

## Threshold and branch-loss operator

`kdse8_compute` applies one operator to an already-valid KDSE-8 value.
For root input `n`, positive threshold `T`, and branch-loss fraction `loss`,
define the retained branch fraction `a = 1 - loss`. Every branch retains `a`
of its incoming value, divides the retained value equally, and sends half to
each child.

A terminal at depth `d` receives:

```text
n * ((1 - loss) / 2)^d
```

It contributes that received value to the output when the value is at least
`T`. Contributions from all active terminals are summed. The operator accepts
any valid KDSE-8 payload; it does not require Ordered form.

### Trusted-input contract

The compute library performs no payload validation, validity assertion,
canonicalization, or malformed-input recovery. Callers handling untrusted
bytes must admit them through `kdse8_validate` before computation.

---

*Copyright © 2026 Mark Karaman · Documentation: CC BY-SA 4.0 · Source samples: AGPL-3.0-or-later*  
*U.S. Patent Pending — Application No. 64/131,240*  
*Commercial licensing: licensing@uhware.com · Security: security@uhware.com*  
*Companion papers: see [docs/papers/](papers/).*
