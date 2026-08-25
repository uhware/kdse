# Copyright (C) 2026 Mark Karaman
# SPDX-License-Identifier: AGPL-3.0-or-later

"""Natural-width structural KDSE bit-string helpers."""

from __future__ import annotations

import re
from collections import deque
from collections.abc import Mapping, Sequence


def _node_id(index: int) -> str:
    """Return sequential IDs A, B, ..., Z, AA, AB, ... ."""

    result = ""
    value = index
    while True:
        result = chr(ord("A") + (value % 26)) + result
        value = value // 26 - 1
        if value < 0:
            return result


def _require_lexical_bits(kdse_bits: str) -> str:
    if not isinstance(kdse_bits, str) or not kdse_bits:
        raise ValueError("KDSE bit-string must be a non-empty string")
    if re.fullmatch(r"[01]+", kdse_bits) is None:
        raise ValueError(
            f"KDSE bits must contain only '0' and '1', got {kdse_bits!r}"
        )
    return kdse_bits


def validate_kdse_bits(kdse_bits: str) -> None:
    """Raise ``ValueError`` unless *kdse_bits* is complete minimal-form KDSE."""

    bits = _require_lexical_bits(kdse_bits)
    length = len(bits)
    if length % 2 == 0:
        raise ValueError(
            f"KDSE length {length} is even; valid payloads have odd length"
        )

    position = 0
    width = 1
    while True:
        if position + width > length:
            raise ValueError(
                f"Incomplete level: need {width} bits, "
                f"have only {length - position}"
            )
        level = bits[position : position + width]
        branches = level.count("1")
        position += width

        if position == length:
            if bits == "0" or branches:
                return
            raise ValueError(
                "Encoded final all-terminal level is illegal; "
                "the final level must be omitted"
            )
        if branches == 0:
            raise ValueError(
                f"Superfluous data starts at position {position}; "
                "no symbols may follow an all-terminal level"
            )
        width = 2 * branches


def is_valid_kdse(kdse_bits: object) -> bool:
    """Return whether *kdse_bits* is a structurally legal minimal payload."""

    try:
        validate_kdse_bits(kdse_bits)  # type: ignore[arg-type]
    except (TypeError, ValueError):
        return False
    return True


def kdse_to_int(kdse_bits: str) -> int:
    """Validate a natural-width KDSE bit-string and return its integer value."""

    validate_kdse_bits(kdse_bits)
    return int(kdse_bits, 2)


def int_to_kdse(value: int) -> str:
    """Convert a non-negative payload integer to validated natural-width KDSE."""

    if isinstance(value, bool) or not isinstance(value, int) or value < 0:
        raise ValueError(f"Expected non-negative integer, got {value!r}")
    bits = bin(value)[2:]
    try:
        validate_kdse_bits(bits)
    except ValueError as exc:
        raise ValueError(
            f"Integer {value} (bits {bits!r}) is not a valid KDSE payload"
        ) from exc
    return bits


def _require_values(values: Sequence[str], expected: int) -> list[str]:
    if isinstance(values, (str, bytes)) or not isinstance(values, Sequence):
        raise TypeError("values must be a sequence of strings")
    normalized = list(values)
    if any(not isinstance(value, str) for value in normalized):
        raise TypeError("every node value must be a string")
    if len(normalized) != expected:
        relation = "Not enough" if len(normalized) < expected else "Too many"
        raise ValueError(
            f"{relation} values: structure uses {expected} nodes, "
            f"but {len(normalized)} values were supplied"
        )
    return normalized


def kdse_to_tree(
    kdse_bits: str, values: Sequence[str]
) -> tuple[dict[str, list[str]], dict[str, str], str]:
    """Reconstruct a full ordered binary tree from KDSE and exact BFS values."""

    validate_kdse_bits(kdse_bits)
    expected_nodes = 2 * kdse_bits.count("1") + 1
    normalized_values = _require_values(values, expected_nodes)

    root = _node_id(0)
    nodes = [root]
    children: dict[str, list[str]] = {}
    current_level = [root]
    position = 0

    while position < len(kdse_bits):
        level_bits = kdse_bits[position : position + len(current_level)]
        next_level: list[str] = []
        for node, bit in zip(current_level, level_bits, strict=True):
            if bit == "1":
                first = _node_id(len(nodes))
                second = _node_id(len(nodes) + 1)
                nodes.extend((first, second))
                children[node] = [first, second]
                next_level.extend((first, second))
        position += len(current_level)
        current_level = next_level

    values_by_id = dict(zip(nodes, normalized_values, strict=True))
    return children, values_by_id, root


def _validate_tree(
    children: Mapping[str, Sequence[str]],
    values: Mapping[str, str],
    root: str,
) -> tuple[dict[str, list[str]], dict[str, str]]:
    if not isinstance(root, str) or not root:
        raise ValueError("root must be a non-empty node ID")
    if root not in values:
        raise ValueError(f"Missing value for root node {root!r}")

    normalized_values = dict(values)
    if any(not isinstance(node, str) or not node for node in normalized_values):
        raise TypeError("node IDs must be non-empty strings")
    if any(not isinstance(value, str) for value in normalized_values.values()):
        raise TypeError("every node value must be a string")

    normalized_children: dict[str, list[str]] = {}
    parents: dict[str, str] = {}
    referenced = {root}

    for parent, raw_kids in children.items():
        if not isinstance(parent, str) or not parent:
            raise TypeError("node IDs must be non-empty strings")
        if isinstance(raw_kids, (str, bytes)) or not isinstance(
            raw_kids, Sequence
        ):
            raise TypeError(f"children of {parent!r} must be a sequence")
        kids = list(raw_kids)
        if len(kids) not in (0, 2):
            raise ValueError(
                f"Node {parent} has {len(kids)} children; "
                "only full binary trees are supported"
            )
        if any(not isinstance(child, str) or not child for child in kids):
            raise TypeError("node IDs must be non-empty strings")
        if len(set(kids)) != len(kids):
            raise ValueError(f"Node {parent} repeats a child")
        normalized_children[parent] = kids
        referenced.add(parent)
        referenced.update(kids)
        for child in kids:
            if child in parents:
                raise ValueError(f"Node {child} has multiple parents")
            parents[child] = parent

    missing_values = sorted(referenced - normalized_values.keys())
    if missing_values:
        raise ValueError(f"Missing values for nodes: {missing_values}")
    extra_values = sorted(normalized_values.keys() - referenced)
    if extra_values:
        raise ValueError(f"Values supplied for unrelated nodes: {extra_values}")
    if root in parents:
        raise ValueError(f"Root node {root} has a parent")

    reachable: set[str] = set()
    queue = deque([root])
    while queue:
        node = queue.popleft()
        if node in reachable:
            raise ValueError(f"Cycle or repeated reachability at node {node}")
        reachable.add(node)
        queue.extend(normalized_children.get(node, ()))
    unreachable = sorted(referenced - reachable)
    if unreachable:
        raise ValueError(f"Unreachable nodes: {unreachable}")

    return normalized_children, normalized_values


def tree_to_kdse_and_values(
    children: Mapping[str, Sequence[str]],
    values: Mapping[str, str],
    root: str,
) -> tuple[str, list[str]]:
    """Encode a validated full binary tree in BFS order without reordering it."""

    normalized_children, normalized_values = _validate_tree(
        children, values, root
    )
    bits: list[str] = []
    value_list: list[str] = []
    queue = deque([root])

    while queue:
        level_size = len(queue)
        level_bits: list[str] = []
        next_level: list[str] = []
        for _ in range(level_size):
            node = queue.popleft()
            value_list.append(normalized_values[node])
            kids = normalized_children.get(node, [])
            level_bits.append("1" if kids else "0")
            next_level.extend(kids)

        if "1" not in level_bits:
            if not bits:
                bits.append("0")
            break
        bits.extend(level_bits)
        queue.extend(next_level)

    return "".join(bits), value_list

