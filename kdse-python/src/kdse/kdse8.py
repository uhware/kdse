# Copyright (C) 2026 Mark Karaman
# SPDX-License-Identifier: AGPL-3.0-or-later

"""KDSE-8 checked-boundary and trusted-compute reference API."""

from __future__ import annotations

from ._reference import (
    KDSE8_SPEC,
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

KDSE8_PAYLOAD_BITS = 7
KDSE8_PAYLOAD_MASK = 0x7F
KDSE8_MAX_DEPTH = 4
KDSE8_MAX_NODES = 15

Kdse8Profile = KdseProfile


def kdse8_payload(value: int) -> int:
    return payload(value, KDSE8_SPEC)


def kdse8_payload_length(value: int) -> int:
    return payload_length(value, KDSE8_SPEC)


def kdse8_validate(value: int) -> KdseStatus:
    return validate(value, KDSE8_SPEC)


def kdse8_decode_profile(value: int) -> Kdse8Profile:
    return decode_profile(value, KDSE8_SPEC)


def kdse8_canonicalize(value: int) -> int:
    return canonicalize(value, KDSE8_SPEC)


def kdse8_is_ordered(value: int) -> bool:
    return is_ordered(value, KDSE8_SPEC)


def kdse8_compute(
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
        KDSE8_SPEC,
    )

