#!/bin/sh
# Copyright (C) 2026 Mark Karaman
# SPDX-License-Identifier: AGPL-3.0-or-later
# This file is part of KDSE.

set -eu

cli=${1:?path to kdse16 CLI is required}

"$cli" validate 0xD555 | grep -F "valid: yes" >/dev/null
"$cli" validate 0xD555 | grep -F \
    "payload-bits: 101010101010101" >/dev/null
if "$cli" validate 4 >/dev/null 2>&1; then
    echo "invalid KDSE-16 payload unexpectedly passed validation" >&2
    exit 1
fi
"$cli" canonicalize 0b110 | grep -F "ordered-payload: 101" >/dev/null
"$cli" profile 0x5555 | grep -F \
    "terminal-depth-profile: [0, 1, 1, 1, 1, 1, 1, 1, 2]" >/dev/null
"$cli" compute 0x7fff --input 100 --threshold 1 --loss-percent 0 |
    grep -F "output: 100" >/dev/null

echo "KDSE-16 CLI tests: pass"
