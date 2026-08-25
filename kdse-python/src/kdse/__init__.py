# Copyright (C) 2026 Mark Karaman
# SPDX-License-Identifier: AGPL-3.0-or-later

"""Python reference implementation of K — Dendritic Structural Encoding."""

from ._reference import KdseProfile
from .kdse8 import (
    KDSE8_MAX_DEPTH,
    KDSE8_MAX_NODES,
    KDSE8_PAYLOAD_BITS,
    KDSE8_PAYLOAD_MASK,
    Kdse8Profile,
    kdse8_canonicalize,
    kdse8_compute,
    kdse8_decode_profile,
    kdse8_is_ordered,
    kdse8_payload,
    kdse8_payload_length,
    kdse8_validate,
)
from .kdse16 import (
    KDSE16_MAX_DEPTH,
    KDSE16_MAX_NODES,
    KDSE16_PAYLOAD_BITS,
    KDSE16_PAYLOAD_MASK,
    Kdse16Profile,
    kdse16_canonicalize,
    kdse16_compute,
    kdse16_decode_profile,
    kdse16_is_ordered,
    kdse16_payload,
    kdse16_payload_length,
    kdse16_validate,
)
from .mermaid import (
    kdse_to_mermaid,
    mermaid_to_kdse,
    parse_mermaid,
    tree_to_mermaid,
)
from .status import (
    KDSE_STATUS_DATA_AFTER_TERMINAL_LEVEL,
    KDSE_STATUS_EXPLICIT_FINAL_TERMINAL_LEVEL,
    KDSE_STATUS_INVALID_PAYLOAD_LENGTH,
    KDSE_STATUS_NULL_ARGUMENT,
    KDSE_STATUS_OK,
    KDSE_STATUS_TRUNCATED_LEVEL,
    KdseStatus,
    KdseValidationError,
    kdse_status_string,
)
from .structure import (
    int_to_kdse,
    is_valid_kdse,
    kdse_to_int,
    kdse_to_tree,
    tree_to_kdse_and_values,
    validate_kdse_bits,
)

__all__ = [name for name in globals() if not name.startswith("_")]

