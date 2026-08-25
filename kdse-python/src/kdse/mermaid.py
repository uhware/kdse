# Copyright (C) 2026 Mark Karaman
# SPDX-License-Identifier: AGPL-3.0-or-later

"""Bidirectional Structural KDSE and Mermaid ``flowchart LR`` adapter."""

from __future__ import annotations

import re
from collections import defaultdict, deque
from collections.abc import Sequence

from .structure import (
    _node_id,
    _validate_tree,
    kdse_to_tree,
    tree_to_kdse_and_values,
)


_ID_RE = re.compile(r"[A-Za-z_][A-Za-z0-9_]*")
_HEADER_RE = re.compile(r"flowchart\s+([A-Za-z]+)\s*", re.IGNORECASE)
_IGNORED_PREFIXES = (
    "classdef ",
    "class ",
    "style ",
    "linkstyle ",
    "click ",
    "subgraph ",
    "direction ",
    "acctitle",
    "accdescr",
    "title ",
)


_ESCAPE_MAP = (
    ("#", "#35;"),
    ('"', "#quot;"),
    ("[", "#91;"),
    ("]", "#93;"),
    ("(", "#40;"),
    (")", "#41;"),
    ("<", "#lt;"),
    (">", "#gt;"),
    ("\n", "#10;"),
    ("\r", "#13;"),
)
_UNESCAPE_MAP = {
    "#quot;": '"',
    "#91;": "[",
    "#93;": "]",
    "#40;": "(",
    "#41;": ")",
    "#lt;": "<",
    "#gt;": ">",
    "#10;": "\n",
    "#13;": "\r",
    "#nbsp;": "\n",  # compatibility with the original adapter
    "#35;": "#",
}
_UNESCAPE_RE = re.compile(
    r"#(?:quot|91|93|40|41|lt|gt|10|13|nbsp|35);"
)


def _escape_label(value: str) -> str:
    for old, new in _ESCAPE_MAP:
        value = value.replace(old, new)
    return value


def _unescape_label(value: str) -> str:
    return _UNESCAPE_RE.sub(
        lambda match: _UNESCAPE_MAP[match.group(0)], value
    )


def _outside_positions(text: str):
    """Yield indexes that are outside node shapes and quoted label text."""

    square_depth = 0
    round_depth = 0
    quote: str | None = None
    index = 0
    while index < len(text):
        char = text[index]
        if quote is not None:
            if char == quote and (index == 0 or text[index - 1] != "\\"):
                quote = None
        else:
            if char in "\"'" and (square_depth or round_depth):
                quote = char
            elif char == "[" and round_depth == 0:
                square_depth += 1
            elif char == "]" and square_depth:
                square_depth -= 1
            elif char == "(" and square_depth == 0:
                round_depth += 1
            elif char == ")" and round_depth:
                round_depth -= 1
            elif square_depth == 0 and round_depth == 0:
                yield index
        index += 1


def _strip_comment(line: str) -> str:
    outside = set(_outside_positions(line))
    for index in range(len(line) - 1):
        if index in outside and line[index : index + 2] == "%%":
            return line[:index]
    return line


def _split_outside(text: str, delimiter: str) -> list[str]:
    outside = set(_outside_positions(text))
    parts: list[str] = []
    start = 0
    for index, char in enumerate(text):
        if index in outside and char == delimiter:
            parts.append(text[start:index])
            start = index + 1
    parts.append(text[start:])
    return parts


def _split_arrows(statement: str) -> list[str]:
    outside = set(_outside_positions(statement))
    parts: list[str] = []
    start = 0
    index = 0
    while index <= len(statement) - 3:
        if index in outside and statement[index : index + 3] == "-->":
            parts.append(statement[start:index])
            start = index + 3
            index += 3
        else:
            index += 1
    parts.append(statement[start:])
    return parts


def _parse_node_ref(text: str) -> tuple[str, str | None]:
    source = text.strip()
    match = _ID_RE.match(source)
    if match is None:
        raise ValueError(f"Expected a Mermaid node reference, got {text!r}")
    node_id = match.group(0)
    remainder = source[match.end() :].strip()
    if not remainder:
        return node_id, None

    opener = remainder[0]
    closer = {"[": "]", "(": ")"}.get(opener)
    if closer is None:
        raise ValueError(
            f"Unsupported Mermaid syntax after node {node_id}: {remainder!r}"
        )

    depth = 0
    quote: str | None = None
    end = None
    for index, char in enumerate(remainder):
        if quote is not None:
            if char == quote and remainder[index - 1] != "\\":
                quote = None
            continue
        if char in "\"'":
            quote = char
        elif char == opener:
            depth += 1
        elif char == closer:
            depth -= 1
            if depth == 0:
                end = index
                break
    if end is None or remainder[end + 1 :].strip():
        raise ValueError(f"Malformed Mermaid declaration for node {node_id}")

    raw = remainder[1:end].strip()
    if len(raw) >= 2 and raw[0] == raw[-1] and raw[0] in "\"'":
        raw = raw[1:-1]
    elif raw[:1] in "\"'" or raw[-1:] in "\"'":
        raise ValueError(f"Unbalanced label quotes for node {node_id}")
    return node_id, _unescape_label(raw)


def _parse_node_group(text: str) -> list[tuple[str, str | None]]:
    parts = _split_outside(text, "&")
    if any(not part.strip() for part in parts):
        raise ValueError(f"Malformed Mermaid node group: {text!r}")
    return [_parse_node_ref(part) for part in parts]


def _record_value(
    values: dict[str, str], node_id: str, value: str | None
) -> None:
    if value is None:
        return
    if node_id in values and values[node_id] != value:
        raise ValueError(
            f"Conflicting values for node {node_id}: "
            f"{values[node_id]!r} vs {value!r}"
        )
    values[node_id] = value


def parse_mermaid(
    mermaid: str,
) -> tuple[dict[str, list[str]], dict[str, str], str]:
    """Parse the supported Mermaid subset and validate one full binary tree."""

    if not isinstance(mermaid, str):
        raise TypeError("mermaid must be a string")

    statements: list[str] = []
    for line in mermaid.splitlines():
        uncommented = _strip_comment(line)
        statements.extend(
            part.strip()
            for part in _split_outside(uncommented, ";")
            if part.strip()
        )

    header_seen = False
    body: list[str] = []
    for statement in statements:
        header = _HEADER_RE.fullmatch(statement)
        if header is not None:
            if header_seen:
                raise ValueError("Expected exactly one Mermaid flowchart header")
            if header.group(1).upper() != "LR":
                raise ValueError("Expected Mermaid 'flowchart LR'")
            header_seen = True
        else:
            body.append(statement)
    if not header_seen:
        raise ValueError("Expected Mermaid 'flowchart LR' header")

    values: dict[str, str] = {}
    all_ids: set[str] = set()
    edges: list[tuple[str, str]] = []

    for statement in body:
        lowered = statement.lower()
        if lowered == "end" or lowered.startswith(_IGNORED_PREFIXES):
            continue

        arrow_parts = _split_arrows(statement)
        if len(arrow_parts) == 1:
            node_id, value = _parse_node_ref(statement)
            all_ids.add(node_id)
            _record_value(values, node_id, value)
            continue

        groups = [_parse_node_group(part) for part in arrow_parts]
        for group in groups:
            for node_id, value in group:
                all_ids.add(node_id)
                _record_value(values, node_id, value)
        for sources, targets in zip(groups, groups[1:]):
            for source, _ in sources:
                for target, _ in targets:
                    edges.append((source, target))

    children: dict[str, list[str]] = defaultdict(list)
    parents: dict[str, str] = {}
    for parent, child in edges:
        children[parent].append(child)
        if child in parents:
            raise ValueError(f"Node {child} has multiple parents - not a tree")
        parents[child] = parent

    roots = sorted(node for node in all_ids if node not in parents)
    if len(roots) != 1:
        raise ValueError(f"Expected exactly one root, found {roots}")
    root = roots[0]

    for node, kids in children.items():
        if len(kids) not in (0, 2):
            raise ValueError(
                f"Node {node} has {len(kids)} children; "
                "only full binary trees are supported"
            )

    reachable: set[str] = set()
    queue = deque([root])
    while queue:
        node = queue.popleft()
        if node in reachable:
            raise ValueError(f"Cycle or repeated reachability at node {node}")
        reachable.add(node)
        queue.extend(children.get(node, ()))
    unreachable = sorted(all_ids - reachable)
    if unreachable:
        raise ValueError(f"Unreachable nodes: {unreachable}")

    missing = sorted(node for node in all_ids if node not in values)
    if missing:
        raise ValueError(f"Missing values for nodes: {missing}")

    return dict(children), values, root


def mermaid_to_kdse(mermaid: str) -> tuple[str, list[str]]:
    """Convert Mermaid ``flowchart LR`` to structural KDSE and BFS values."""

    children, values, root = parse_mermaid(mermaid)
    return tree_to_kdse_and_values(children, values, root)


def tree_to_mermaid(
    children: dict[str, list[str]],
    values: dict[str, str],
    root: str,
    direction: str = "LR",
) -> str:
    """Emit deterministic Mermaid with sequential BFS IDs and rounded nodes."""

    normalized_direction = direction.upper()
    if normalized_direction not in {"LR", "RL", "TB", "TD", "BT"}:
        raise ValueError(f"Unsupported Mermaid direction: {direction!r}")

    normalized_children, normalized_values = _validate_tree(
        children, values, root
    )
    order: list[str] = []
    queue = deque([root])
    visited: set[str] = set()
    while queue:
        node = queue.popleft()
        if node in visited:
            raise ValueError(f"Cycle or repeated reachability at node {node}")
        visited.add(node)
        order.append(node)
        queue.extend(normalized_children.get(node, ()))

    canonical_ids = {
        node: _node_id(index) for index, node in enumerate(order)
    }

    lines = [f"flowchart {normalized_direction}"]
    for node in order:
        node_id = canonical_ids[node]
        lines.append(
            f'    {node_id}("{_escape_label(normalized_values[node])}")'
        )
    for node in order:
        for child in normalized_children.get(node, ()):
            lines.append(
                f"    {canonical_ids[node]} --> {canonical_ids[child]}"
            )
    return "\n".join(lines)


def kdse_to_mermaid(
    kdse_bits: str,
    values: Sequence[str],
    direction: str = "LR",
) -> str:
    """Convert validated structural KDSE and exact BFS values to Mermaid."""

    children, values_by_id, root = kdse_to_tree(kdse_bits, values)
    return tree_to_mermaid(children, values_by_id, root, direction)
