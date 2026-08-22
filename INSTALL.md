<!--
Copyright (C) 2026 Mark Karaman
SPDX-License-Identifier: CC-BY-SA-4.0
Command and source-code samples: AGPL-3.0-or-later
-->

# Building and installing KDSE

**Repository status:** Read-only (v1.0). This document remains the build reference for the frozen 1.0 release.

## Requirements

- An ISO C11 compiler.
- A POSIX-style `make` and shell.
- A standard archiver compatible with `ar rcs`.

No third-party runtime libraries are required.

## Build and test

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
