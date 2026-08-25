# Copyright (C) 2026 Mark Karaman
# SPDX-License-Identifier: AGPL-3.0-or-later

"""Status values shared by the checked KDSE Python APIs."""

from __future__ import annotations

from enum import IntEnum


class KdseStatus(IntEnum):
    """Python equivalent of ``kdse_status_t`` from the C reference."""

    OK = 0
    NULL_ARGUMENT = 1
    INVALID_PAYLOAD_LENGTH = 2
    TRUNCATED_LEVEL = 3
    DATA_AFTER_TERMINAL_LEVEL = 4
    EXPLICIT_FINAL_TERMINAL_LEVEL = 5


KDSE_STATUS_OK = KdseStatus.OK
KDSE_STATUS_NULL_ARGUMENT = KdseStatus.NULL_ARGUMENT
KDSE_STATUS_INVALID_PAYLOAD_LENGTH = KdseStatus.INVALID_PAYLOAD_LENGTH
KDSE_STATUS_TRUNCATED_LEVEL = KdseStatus.TRUNCATED_LEVEL
KDSE_STATUS_DATA_AFTER_TERMINAL_LEVEL = KdseStatus.DATA_AFTER_TERMINAL_LEVEL
KDSE_STATUS_EXPLICIT_FINAL_TERMINAL_LEVEL = (
    KdseStatus.EXPLICIT_FINAL_TERMINAL_LEVEL
)


_STATUS_MESSAGES = {
    KdseStatus.OK: "valid KDSE payload",
    KdseStatus.NULL_ARGUMENT: "null output argument",
    KdseStatus.INVALID_PAYLOAD_LENGTH: "invalid natural-width payload length",
    KdseStatus.TRUNCATED_LEVEL: "truncated breadth-first level",
    KdseStatus.DATA_AFTER_TERMINAL_LEVEL: (
        "data follows a level with no branches"
    ),
    KdseStatus.EXPLICIT_FINAL_TERMINAL_LEVEL: (
        "final all-terminal level must be omitted"
    ),
}


def kdse_status_string(status: int | KdseStatus) -> str:
    """Return the stable C-reference description for *status*."""

    try:
        normalized = KdseStatus(status)
    except (TypeError, ValueError):
        return "unknown KDSE status"
    return _STATUS_MESSAGES[normalized]


class KdseValidationError(ValueError):
    """Raised by checked Python operations when a payload is invalid."""

    def __init__(self, status: KdseStatus):
        self.status = KdseStatus(status)
        super().__init__(kdse_status_string(self.status))

