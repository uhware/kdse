<!--
Copyright (C) 2026 Mark Karaman
SPDX-License-Identifier: CC-BY-SA-4.0
-->

# Changelog

## [1.0.3] — 2026-08-24

- Added the Python reference implementation of the KDSE-8 and KDSE-16
  contracts: validation, terminal-depth profiles, Ordered canonicalization,
  trusted compute paths, shared status codes, and width-specific CLIs.
- Added the bidirectional Mermaid `flowchart LR` structural adapter with BFS
  value ordering, preserved upper/lower orientation, deterministic output, and
  strict full-binary-tree validation.
- Added exhaustive Python-space tests and direct compiled-C parity tests for
  validation status, profiles, canonicalization, and compute results.
- Added Python packaging, compatibility import `kdse_mermaid`, validation
  records, installation guidance, and CI coverage for Python 3.10 and 3.12.

## [1.0.2] - 2026-08-22

Documentation-only alignment with project state.

## [1.0.1] — 2026-08-19

Documentation-only alignment with the project description:

- Describe the C11 code in this repository as a portable reference implementation, not as the definition of KDSE.
- README title and lead paragraph updated; no code or API changes.

## [1.0] — 2026-08-18

Initial public release.

- KDSE-8 and KDSE-16 ISO C11 reference implementation (checked admission + trusted compute paths).
- Companion papers under CC BY-SA 4.0:
  - Introductory paper (structure, attributes, operator separation)
  - Instantaneous Jump Magnitude paper (threshold-and-loss operator, \(J_d = T\,c_d\))
- Dual licensing: AGPL-3.0-or-later (code), CC BY-SA 4.0 (documentation and papers).
- U.S. Patent Pending — Application No. 64/131,240.
- Repository published in **Archived / read-only** state.

Commercial licensing: licensing@uhware.com  
Security reports: security@uhware.com
