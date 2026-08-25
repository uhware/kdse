<!--
Copyright (C) 2026 Mark Karaman
SPDX-License-Identifier: CC-BY-SA-4.0
Command and source-code samples: AGPL-3.0-or-later
-->

# Building and installing KDSE

**Repository status:** Read-only (v1.0.3).

## C requirements

- An ISO C11 compiler.
- A POSIX-style `make` and shell.
- A standard archiver compatible with `ar rcs`.

No third-party runtime libraries are required.

## Build and test C

```sh
make
make test
```

Build products are written beneath `build/`:

- `build/lib/libkdse8_checked.a`
- `build/lib/libkdse8_compute.a`
- `build/lib/libkdse16_checked.a`
- `build/lib/libkdse16_compute.a`
- `build/bin/kdse`
- `build/bin/kdse16`
- `build/bin/example_compute`
- `build/bin/example_canonicalize`
- `build/bin/example_compute16`
- `build/bin/example_canonicalize16`

Select another compiler or build directory through ordinary make variables:

```sh
make clean
make CC=clang BUILD_DIR=out
make CC=clang BUILD_DIR=out test
```

## Install

The default prefix is `/usr/local`:

```sh
make install
```

For packaging or an unprivileged staging directory:

```sh
make DESTDIR="$PWD/stage" PREFIX=/usr install
```

This installs the public headers, all four static libraries, and the `kdse`
and `kdse16` command-line programs.

## Install and test Python

The Python reference implementation requires Python 3.10 or later and has no
third-party runtime dependencies.

From the repository root:

```sh
python -m pip install ./kdse-python
KDSE_C_REFERENCE_ROOT=. \
  python -m unittest discover -s kdse-python/tests -v
```

The environment variable enables direct parity tests against the C source in
this repository. Without it, the Python-only suite runs and the C-parity class
skips cleanly.

The installed commands are:

- `kdse-py` — KDSE-8 validation, profiles, canonicalization, and compute.
- `kdse16-py` — the corresponding KDSE-16 command.

For editable development from `kdse-python/`:

```sh
python -m pip install -e .
KDSE_C_REFERENCE_ROOT=.. python -m unittest discover -s tests -v
```
