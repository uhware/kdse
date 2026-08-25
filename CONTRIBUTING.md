<!--
Copyright (C) 2026 Mark Karaman
SPDX-License-Identifier: CC-BY-SA-4.0
Command samples: AGPL-3.0-or-later
-->

# Contributing

**This repository is currently read-only (v1.0.3).**
Issues and pull requests are not accepted at this time.

When the repository is re-opened for contributions, the following rules will apply:

Contributions should preserve the separation between checked admission and trusted computation for both container widths. In particular, do not add validation, canonicalization, assertions of KDSE validity, or malformed-input recovery to either streamlined compute path.

Before submitting a change:

```sh
make clean
make test

cd kdse-python
python -m pip install -e .
KDSE_C_REFERENCE_ROOT=.. python -m unittest discover -s tests -v
```

Public C API changes must include complete header documentation and
corresponding updates to `docs/API.md`. Python API or adapter changes must
update `kdse-python/README.md` when their public behavior changes. Behavioral
changes must include tests, and the Python implementation must retain direct
parity with the C reference for their shared contracts. New source files must
carry an `AGPL-3.0-or-later` SPDX notice; new Markdown files must carry a
`CC-BY-SA-4.0` notice and separately identify any AGPL-covered source samples.
