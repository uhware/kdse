<!--
Copyright (C) 2026 Mark Karaman
SPDX-License-Identifier: CC-BY-SA-4.0
Source-code and command samples: AGPL-3.0-or-later
-->

# KDSE — Python reference implementation and Mermaid adapter

Python implementation of the KDSE-8 and KDSE-16 contracts from the portable
ISO C11 reference implementation v1.0.2, plus a bidirectional Structural KDSE
adapter for Mermaid `flowchart LR` trees.

**K — Dendritic Structural Encoding (KDSE)** encodes finite full binary trees
in breadth-first order. `1` means branch, `0` means terminal, and the final
all-terminal level is deterministic and omitted. Ordered form is a canonical
identity among sibling permutations; it is not a validity requirement.

## Scope

| Component | Behavior |
| --- | --- |
| `kdse.kdse8` | KDSE-8 payload extraction, validation, profiles, Ordered canonicalization, and trusted compute |
| `kdse.kdse16` | KDSE-16 equivalent of the KDSE-8 API |
| `kdse.structure` | Checked natural-width structural bit-string and tree conversion helpers |
| `kdse.mermaid` | Mermaid `flowchart LR` ↔ structural KDSE + BFS value list |
| `kdse_mermaid` | Compatibility import surface for the original adapter |
| `kdse-py`, `kdse16-py` | Width-specific validation, profile, canonicalization, and compute CLIs |

The Mermaid adapter preserves upper/lower orientation. It does not reorder a
valid structural payload into Ordered form. “Canonical Mermaid” refers only to
deterministic formatting and sequential BFS node IDs.

## Install and test

Python 3.10 or later is required.

```sh
python -m pip install -e .
python -m unittest discover -s tests -v
```

## KDSE-8 and KDSE-16 checked APIs

```python
from kdse import (
    KDSE_STATUS_OK,
    kdse8_canonicalize,
    kdse8_compute,
    kdse8_decode_profile,
    kdse8_validate,
)

candidate = 0b110
if kdse8_validate(candidate) == KDSE_STATUS_OK:
    profile = kdse8_decode_profile(candidate)
    ordered = kdse8_canonicalize(candidate)  # 0b101
    output = kdse8_compute(candidate, 5.0, 0.30, 0.17)
```

The checked APIs mask and ignore the unassigned container bit exactly as the C
reference does: bit 7 for KDSE-8 and bit 15 for KDSE-16. Canonicalization
returns a payload with that bit clear.

`kdse8_compute` and `kdse16_compute` are trusted-input paths. They deliberately
do not validate structure or numeric preconditions. Validate untrusted
containers before calling them.

### Command line

The installed package provides width-specific commands analogous to the C
reference CLIs:

```sh
kdse-py validate 0b101
kdse-py canonicalize 0b110
kdse-py profile 0b1110111
kdse-py compute 0b1110111 \
  --input 5 --threshold 0.3 --loss-percent 17

kdse16-py validate 0x5555
kdse16-py profile 0b101010101010101
```

The equivalent module entry point is `python -m kdse --width 8 ...` or
`python -m kdse --width 16 ...`.

## Mermaid conversion

```python
from kdse import kdse_to_int, kdse_to_mermaid, mermaid_to_kdse

mermaid = """
flowchart LR
    A("42") --> B("91")
    A --> C("7")
    B --> D("13")
    B --> E("68")
"""

bits, values = mermaid_to_kdse(mermaid)
assert bits == "110"
assert values == ["42", "91", "7", "13", "68"]
assert kdse_to_int(bits) == 6

generated = kdse_to_mermaid(bits, values)
bits2, values2 = mermaid_to_kdse(generated)
assert (bits2, values2) == (bits, values)
```

`110` is valid Structural KDSE. Its Ordered sibling-permutation representative
is `101`; the adapter correctly retains `110` because upper/lower placement is
part of its input contract.

### Mermaid input contract

- Exactly one `flowchart LR` header is required.
- Source order of outgoing edges defines upper then lower child.
- Nodes must have zero or exactly two children.
- Every participating node needs an explicit value; `A("")` is an explicit
  empty value.
- Square and rounded node declarations are accepted.
- Simple arrow chains and `&` node groups are accepted.
- Mermaid `%%` comments are removed before parsing, including comments that
  contain node or edge syntax.
- Styling and documentation chrome such as `classDef`, `class`, `style`,
  `linkStyle`, `click`, and `subgraph` declarations is ignored.
- Unsupported arrow or node syntax raises rather than being silently skipped.

Values are independent of structure. Empty strings, duplicates, Unicode,
quotes, brackets, parentheses, angle brackets, newlines, carriage returns, and
literal Mermaid entity text round-trip through generated Mermaid.

## Natural-width integer helpers

```python
from kdse import int_to_kdse, is_valid_kdse, kdse_to_int

bits = int_to_kdse(7825)
assert bits == "1111010010001"
assert kdse_to_int(bits) == 7825
assert is_valid_kdse(bits)
assert not is_valid_kdse("100")
```

These helpers operate on natural-width payload integers, not fixed-width
KDSE-8 or KDSE-16 physical containers. Both conversion directions perform full
structural validation. Use `kdse8_payload` or `kdse16_payload` when extracting
payloads from physical containers with an unassigned high bit.

## Reference alignment

The authoritative C-facing contracts are:

- `docs/KDSE-8.md`
- `docs/KDSE-16.md`
- `docs/API.md`
- `include/kdse/`

The test suite exhaustively checks all KDSE-8 and KDSE-16 payloads. When the C
v1.0.2 source tree is available, `tests/test_c_parity.py` can additionally
compile it and compare validation, decoded profiles, canonicalization, and
compute results directly. See [VALIDATION.md](VALIDATION.md) for the recorded
coverage and results against C v1.0.2.

## License

| Material | License |
| --- | --- |
| Python source, tests, and source-code samples | AGPL-3.0-or-later |
| Markdown documentation | CC BY-SA 4.0 |

U.S. Patent Pending — Application No. 64/131,240

Commercial licensing: **licensing@uhware.com**  
Security reports: **security@uhware.com**
