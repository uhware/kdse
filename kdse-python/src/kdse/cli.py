# Copyright (C) 2026 Mark Karaman
# SPDX-License-Identifier: AGPL-3.0-or-later

"""Command-line interfaces for the KDSE-8 and KDSE-16 Python APIs."""

from __future__ import annotations

import argparse
from collections.abc import Callable, Sequence
from dataclasses import dataclass
import math
import sys

from ._reference import KdseProfile
from .kdse8 import (
    kdse8_canonicalize,
    kdse8_compute,
    kdse8_decode_profile,
    kdse8_payload,
    kdse8_validate,
)
from .kdse16 import (
    kdse16_canonicalize,
    kdse16_compute,
    kdse16_decode_profile,
    kdse16_payload,
    kdse16_validate,
)
from .status import KdseStatus, KdseValidationError, kdse_status_string


@dataclass(frozen=True, slots=True)
class _CliApi:
    width: int
    validate: Callable[[int], KdseStatus]
    payload: Callable[[int], int]
    profile: Callable[[int], KdseProfile]
    canonicalize: Callable[[int], int]
    compute: Callable[[int, float, float, float], float]

    @property
    def container_max(self) -> int:
        return (1 << self.width) - 1


_API8 = _CliApi(
    8,
    kdse8_validate,
    kdse8_payload,
    kdse8_decode_profile,
    kdse8_canonicalize,
    kdse8_compute,
)
_API16 = _CliApi(
    16,
    kdse16_validate,
    kdse16_payload,
    kdse16_decode_profile,
    kdse16_canonicalize,
    kdse16_compute,
)


def _container_value(text: str, api: _CliApi) -> int:
    try:
        value = int(text, 0)
    except ValueError as exc:
        raise argparse.ArgumentTypeError(
            "VALUE accepts decimal, 0x hexadecimal, or 0b binary notation"
        ) from exc
    if not 0 <= value <= api.container_max:
        raise argparse.ArgumentTypeError(
            f"VALUE must be in 0..{api.container_max}"
        )
    return value


def _finite_float(text: str) -> float:
    try:
        value = float(text)
    except ValueError as exc:
        raise argparse.ArgumentTypeError("expected a number") from exc
    if not math.isfinite(value):
        raise argparse.ArgumentTypeError("expected a finite number")
    return value


def _payload_bits(value: int) -> str:
    return bin(value)[2:]


def _print_identity(value: int, api: _CliApi) -> None:
    payload = api.payload(value)
    print(f"container: {value}")
    print(f"unassigned-bit: {(value >> (api.width - 1)) & 1} (ignored)")
    print(f"payload-decimal: {payload}")
    print(f"payload-bits: {_payload_bits(payload)}")


def _parser(api: _CliApi, program: str) -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(
        prog=program,
        description=f"KDSE-{api.width} Python reference CLI",
    )
    subparsers = parser.add_subparsers(dest="command", required=True)

    for command in ("validate", "canonicalize", "profile"):
        subparser = subparsers.add_parser(command)
        subparser.add_argument("value", type=lambda text: _container_value(text, api))

    compute_parser = subparsers.add_parser("compute")
    compute_parser.add_argument("value", type=lambda text: _container_value(text, api))
    compute_parser.add_argument("--input", required=True, type=_finite_float)
    compute_parser.add_argument("--threshold", required=True, type=_finite_float)
    compute_parser.add_argument(
        "--loss-percent", required=True, type=_finite_float
    )
    return parser


def _run(api: _CliApi, program: str, argv: Sequence[str] | None) -> int:
    parser = _parser(api, program)
    arguments = parser.parse_args(argv)
    value = arguments.value

    if arguments.command == "validate":
        status = api.validate(value)
        print(f"valid: {'yes' if status is KdseStatus.OK else 'no'}")
        _print_identity(value, api)
        print(f"status: {kdse_status_string(status)}")
        return 0 if status is KdseStatus.OK else 1

    if arguments.command == "canonicalize":
        try:
            ordered = api.canonicalize(value)
        except KdseValidationError as exc:
            print(
                f"invalid KDSE-{api.width}: {kdse_status_string(exc.status)}",
                file=sys.stderr,
            )
            return 1
        print(f"input-payload: {_payload_bits(api.payload(value))}")
        print(f"ordered-payload: {_payload_bits(ordered)}")
        print(f"ordered-decimal: {ordered}")
        return 0

    if arguments.command == "profile":
        try:
            profile = api.profile(value)
        except KdseValidationError as exc:
            print(
                f"invalid KDSE-{api.width}: {kdse_status_string(exc.status)}",
                file=sys.stderr,
            )
            return 1
        _print_identity(value, api)
        print(f"payload-length: {profile.payload_length}")
        print(f"branches: {profile.branch_count}")
        print(f"terminals: {profile.terminal_count}")
        print(f"max-depth: {profile.max_depth}")
        occupied = profile.terminals[: profile.max_depth + 1]
        print(f"terminal-depth-profile: {list(occupied)}")
        return 0

    if (
        arguments.input < 0.0
        or arguments.threshold <= 0.0
        or arguments.loss_percent < 0.0
        or arguments.loss_percent >= 100.0
    ):
        parser.error(
            "required ranges: input >= 0, threshold > 0, "
            "0 <= loss-percent < 100"
        )

    status = api.validate(value)
    if status is not KdseStatus.OK:
        print(
            f"invalid KDSE-{api.width}: {kdse_status_string(status)}",
            file=sys.stderr,
        )
        return 1
    output = api.compute(
        value,
        arguments.input,
        arguments.threshold,
        arguments.loss_percent / 100.0,
    )
    print(f"payload: {_payload_bits(api.payload(value))}")
    print(f"input: {arguments.input:.15g}")
    print(f"threshold: {arguments.threshold:.15g}")
    print(f"branch-loss-percent: {arguments.loss_percent:.15g}")
    print(f"output: {output:.15g}")
    return 0


def main8(argv: Sequence[str] | None = None) -> int:
    """Run the KDSE-8 CLI."""

    return _run(_API8, "kdse-py", argv)


def main16(argv: Sequence[str] | None = None) -> int:
    """Run the KDSE-16 CLI."""

    return _run(_API16, "kdse16-py", argv)


def main(argv: Sequence[str] | None = None) -> int:
    """Run ``python -m kdse`` with an explicit container width."""

    parser = argparse.ArgumentParser(prog="python -m kdse", add_help=False)
    parser.add_argument("--width", type=int, choices=(8, 16), required=True)
    arguments, remaining = parser.parse_known_args(argv)
    return main8(remaining) if arguments.width == 8 else main16(remaining)

