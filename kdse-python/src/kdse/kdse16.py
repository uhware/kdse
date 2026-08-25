# Copyright (C) 2026 Mark Karaman
# SPDX-License-Identifier: AGPL-3.0-or-later

"""KDSE-16 checked-boundary and trusted-compute reference API."""

from __future__ import annotations

from ._reference import (
    KDSE16_SPEC,
    KdseProfile,
    canonicalize,
    compute,
    decode_profile,
    is_ordered,
    payload,
    payload_length,
    validate,
)
from .status import KdseStatus

KDSE16_PAYLOAD_BITS = 15
KDSE16_PAYLOAD_MASK = 0x7FFF
KDSE16_MAX_DEPTH = 8
KDSE16_MAX_NODES = 31

Kdse16Profile = KdseProfile


def kdse16_payload(value: int) -> int:
    return payload(value, KDSE16_SPEC)


def kdse16_payload_length(value: int) -> int:
    return payload_length(value, KDSE16_SPEC)


def kdse16_validate(value: int) -> KdseStatus:
    return validate(value, KDSE16_SPEC)


def kdse16_decode_profile(value: int) -> Kdse16Profile:
    return decode_profile(value, KDSE16_SPEC)


def kdse16_canonicalize(value: int) -> int:
    return canonicalize(value, KDSE16_SPEC)


def kdse16_is_ordered(value: int) -> bool:
    return is_ordered(value, KDSE16_SPEC)


def kdse16_compute(
    validated_value: int,
    input_value: float,
    threshold: float,
    branch_loss: float,
) -> float:
    return compute(
        validated_value,
        input_value,
        threshold,
        branch_loss,
        KDSE16_SPEC,
    )

