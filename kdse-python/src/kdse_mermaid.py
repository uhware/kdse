#!/usr/bin/env python3
# Copyright (C) 2026 Mark Karaman
# SPDX-License-Identifier: AGPL-3.0-or-later

"""Compatibility module for the original standalone Mermaid adapter API."""

from kdse.mermaid import (  # noqa: F401
    kdse_to_mermaid,
    mermaid_to_kdse,
    parse_mermaid,
    tree_to_mermaid,
)
from kdse.structure import (  # noqa: F401
    int_to_kdse,
    is_valid_kdse,
    kdse_to_int,
    kdse_to_tree,
    tree_to_kdse_and_values,
    validate_kdse_bits,
)

