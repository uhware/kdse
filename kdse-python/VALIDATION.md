<!--
Copyright (C) 2026 Mark Karaman
SPDX-License-Identifier: CC-BY-SA-4.0
Source-code and command samples: AGPL-3.0-or-later
-->

# Python reference validation record

Validation date: 2026-08-24  
C reference: KDSE v1.0.2  
C reference archive SHA-256:
`0ee902c3fe1e9705bac695012969c5c20f46b51960390e6091695a2a2d42a3c2`

## Direct C parity

The test suite compiled the C v1.0.2 checked and compute sources as a temporary
shared library and compared their results with the Python implementation.

| Comparison | Coverage | Result |
| --- | ---: | --- |
| Validation status | All 256 KDSE-8 physical containers | Exact match |
| Validation status | All 65,536 KDSE-16 physical containers | Exact match |
| Decoded profile | All 38 valid KDSE-8 payloads | Exact match |
| Decoded profile | All 4,397 valid KDSE-16 payloads | Exact match |
| Ordered canonicalization | All 38 valid KDSE-8 payloads | Exact match |
| Ordered canonicalization | All 4,397 valid KDSE-16 payloads | Exact match |
| Trusted compute | 2,280 KDSE-8 parameter combinations | Match within `1e-15` relative tolerance |
| Trusted compute | 263,820 KDSE-16 parameter combinations | Match within `1e-15` relative tolerance |

The container comparisons include the ignored high bit: bit 7 for KDSE-8 and
bit 15 for KDSE-16.

## Structural and Mermaid validation

- Every one of the 4,397 valid KDSE-16 natural-width payloads completed
  KDSE → Mermaid → KDSE with identical structure and values.
- Non-Ordered payloads, including `110`, retained their upper/lower placement.
- The original 7,825 vector reconstructed as `1111010010001` with its exact BFS
  value order.
- Mermaid comments containing syntactically valid nodes or edges were confirmed
  not to participate in the parsed tree.
- Empty, duplicate, Unicode, multiline, entity-like, and punctuation-heavy
  values round-tripped exactly.
- Explicit final all-terminal levels, truncated levels, data after terminal
  levels, nonbinary strings, incorrect value counts, missing node values,
  multiple parents, cycles, and non-full trees were rejected.

## Reproduce

Run the Python-only suite:

```sh
PYTHONPATH=src python -m unittest discover -s tests -v
```

Run the same suite with direct compiled-C comparison:

```sh
KDSE_C_REFERENCE_ROOT=/path/to/kdse-v1.0.2 \
PYTHONPATH=src python -m unittest discover -s tests -v
```

The C parity test skips cleanly when `KDSE_C_REFERENCE_ROOT` is not set.

