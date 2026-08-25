# Copyright (C) 2026 Mark Karaman
# SPDX-License-Identifier: AGPL-3.0-or-later

"""Shared implementation for the KDSE-8 and KDSE-16 reference APIs."""

from __future__ import annotations

from dataclasses import dataclass
from functools import lru_cache

from .status import KdseStatus, KdseValidationError


@dataclass(frozen=True, slots=True)
class KdseProfile:
    """Terminal-depth profile reconstructed from a valid KDSE payload."""

    terminals: tuple[int, ...]
    payload_length: int
    max_depth: int
    branch_count: int
    terminal_count: int


@dataclass(frozen=True, slots=True)
class _ContainerSpec:
    payload_bits: int
    max_depth: int
    max_nodes: int

    @property
    def payload_mask(self) -> int:
        return (1 << self.payload_bits) - 1

    @property
    def container_mask(self) -> int:
        return (1 << (self.payload_bits + 1)) - 1


KDSE8_SPEC = _ContainerSpec(payload_bits=7, max_depth=4, max_nodes=15)
KDSE16_SPEC = _ContainerSpec(payload_bits=15, max_depth=8, max_nodes=31)


def _require_container(value: int, spec: _ContainerSpec) -> int:
    if isinstance(value, bool) or not isinstance(value, int):
        raise TypeError("KDSE container value must be an integer")
    if not 0 <= value <= spec.container_mask:
        raise ValueError(
            f"KDSE-{spec.payload_bits + 1} container must be in "
            f"0..{spec.container_mask}, got {value}"
        )
    return value


def payload(value: int, spec: _ContainerSpec) -> int:
    """Extract the right-aligned payload; the high container bit is ignored."""

    return _require_container(value, spec) & spec.payload_mask


def _bit_length(payload_value: int) -> int:
    return payload_value.bit_length() or 1


def payload_length(value: int, spec: _ContainerSpec) -> int:
    """Return natural-width payload length without validating the structure."""

    return _bit_length(payload(value, spec))


def _payload_bit(payload_value: int, length: int, position: int) -> int:
    shift = length - position - 1
    return (payload_value >> shift) & 1


def _validate_payload(payload_value: int) -> KdseStatus:
    length = _bit_length(payload_value)
    position = 0
    width = 1

    if length % 2 == 0:
        return KdseStatus.INVALID_PAYLOAD_LENGTH

    while True:
        if position + width > length:
            return KdseStatus.TRUNCATED_LEVEL

        branches = sum(
            _payload_bit(payload_value, length, position + index)
            for index in range(width)
        )
        position += width

        if position == length:
            if payload_value == 0 or branches != 0:
                return KdseStatus.OK
            return KdseStatus.EXPLICIT_FINAL_TERMINAL_LEVEL
        if branches == 0:
            return KdseStatus.DATA_AFTER_TERMINAL_LEVEL
        width = 2 * branches


def validate(value: int, spec: _ContainerSpec) -> KdseStatus:
    """Validate the masked, natural-width payload in a physical container."""

    return _validate_payload(payload(value, spec))


def decode_profile(value: int, spec: _ContainerSpec) -> KdseProfile:
    """Validate and reconstruct a fixed-width terminal-depth profile."""

    payload_value = payload(value, spec)
    status = _validate_payload(payload_value)
    if status is not KdseStatus.OK:
        raise KdseValidationError(status)

    length = _bit_length(payload_value)
    terminals = [0] * (spec.max_depth + 1)
    position = 0
    width = 1
    depth = 0
    branch_count = 0

    while True:
        branches = sum(
            _payload_bit(payload_value, length, position + index)
            for index in range(width)
        )
        terminals[depth] += width - branches
        branch_count += branches
        position += width

        if position == length:
            if branches:
                terminals[depth + 1] = 2 * branches
                max_depth = depth + 1
            else:
                max_depth = depth
            break

        width = 2 * branches
        depth += 1

    return KdseProfile(
        terminals=tuple(terminals),
        payload_length=length,
        max_depth=max_depth,
        branch_count=branch_count,
        terminal_count=sum(terminals),
    )


def _build_branch_tree(payload_value: int) -> tuple[tuple[int, int] | None, ...]:
    """Build the full tree used to compute the C-compatible unordered key."""

    length = _bit_length(payload_value)
    nodes: list[tuple[int, int] | None] = [None]
    queue = [0]
    queue_head = 0

    for position in range(length):
        node_index = queue[queue_head]
        queue_head += 1
        if _payload_bit(payload_value, length, position):
            first = len(nodes)
            nodes.extend((None, None))
            nodes[node_index] = (first, first + 1)
            queue.extend((first, first + 1))

    return tuple(nodes)


def _canonical_key_node(
    tree: tuple[tuple[int, int] | None, ...], node_index: int
) -> str:
    children = tree[node_index]
    if children is None:
        return "0"
    left = _canonical_key_node(tree, children[0])
    right = _canonical_key_node(tree, children[1])
    first, second = sorted((left, right))
    return "1" + first + second


def _unordered_key(payload_value: int) -> str:
    return _canonical_key_node(_build_branch_tree(payload_value), 0)


@lru_cache(maxsize=2)
def _ordered_representatives(payload_bits: int) -> dict[str, int]:
    representatives: dict[str, int] = {}
    for candidate in range(1 << payload_bits):
        if _validate_payload(candidate) is not KdseStatus.OK:
            continue
        representatives.setdefault(_unordered_key(candidate), candidate)
    return representatives


def canonicalize(value: int, spec: _ContainerSpec) -> int:
    """Return the lowest-numerical sibling-permutation representative."""

    payload_value = payload(value, spec)
    status = _validate_payload(payload_value)
    if status is not KdseStatus.OK:
        raise KdseValidationError(status)
    return _ordered_representatives(spec.payload_bits)[
        _unordered_key(payload_value)
    ]


def is_ordered(value: int, spec: _ContainerSpec) -> bool:
    """Validate and determine whether the payload is already Ordered."""

    payload_value = payload(value, spec)
    return payload_value == canonicalize(value, spec)


def compute(
    validated_value: int,
    input_value: float,
    threshold: float,
    branch_loss: float,
    spec: _ContainerSpec,
) -> float:
    """Run the trusted-input threshold-and-loss operator without validation."""

    payload_value = payload(validated_value, spec)
    length = _bit_length(payload_value)
    child_fraction = (1.0 - float(branch_loss)) * 0.5
    terminal_value = float(input_value)
    threshold_value = float(threshold)
    output = 0.0
    position = 0
    width = 1

    while True:
        branches = 0
        for index in range(width):
            branches_here = _payload_bit(
                payload_value, length, position + index
            )
            branches += branches_here
            if branches_here == 0 and terminal_value >= threshold_value:
                output += terminal_value

        position += width
        terminal_value *= child_fraction

        if position == length:
            if terminal_value >= threshold_value:
                output += (2 * branches) * terminal_value
            return output
        width = 2 * branches

