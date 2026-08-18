#!/bin/sh
# Copyright (C) 2026 Mark Karaman
# SPDX-License-Identifier: AGPL-3.0-or-later
# This file is part of KDSE.

set -eu

cli=${1:?path to kdse CLI is required}

"$cli" validate 0x85 | grep -F "valid: yes" >/dev/null
"$cli" validate 0x85 | grep -F "payload-bits: 101" >/dev/null
if "$cli" validate 4 >/dev/null 2>&1; then
    echo "invalid payload unexpectedly passed validation" >&2
    exit 1
fi
"$cli" canonicalize 0b110 | grep -F "ordered-payload: 101" >/dev/null
"$cli" profile 119 | grep -F "terminal-depth-profile: [0, 0, 1, 6]" >/dev/null
"$cli" compute 119 --input 5 --threshold 0.3 --loss-percent 17 |
    grep -F "output: 3.00532625" >/dev/null

echo "CLI tests: pass"
