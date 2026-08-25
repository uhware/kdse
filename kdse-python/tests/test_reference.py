# Copyright (C) 2026 Mark Karaman
# SPDX-License-Identifier: AGPL-3.0-or-later

from __future__ import annotations

import math
import unittest

from kdse import (
    KDSE_STATUS_DATA_AFTER_TERMINAL_LEVEL,
    KDSE_STATUS_EXPLICIT_FINAL_TERMINAL_LEVEL,
    KDSE_STATUS_INVALID_PAYLOAD_LENGTH,
    KDSE_STATUS_OK,
    KDSE_STATUS_TRUNCATED_LEVEL,
    KdseValidationError,
    kdse8_canonicalize,
    kdse8_compute,
    kdse8_decode_profile,
    kdse8_is_ordered,
    kdse8_payload,
    kdse8_payload_length,
    kdse8_validate,
    kdse16_canonicalize,
    kdse16_compute,
    kdse16_decode_profile,
    kdse16_is_ordered,
    kdse16_payload,
    kdse16_payload_length,
    kdse16_validate,
    kdse_status_string,
)


VALID_8 = {
    0,
    1,
    5,
    6,
    7,
    21,
    22,
    23,
    25,
    26,
    27,
    85,
    86,
    87,
    89,
    90,
    91,
    101,
    102,
    103,
    105,
    106,
    107,
    113,
    114,
    115,
    116,
    117,
    118,
    119,
    120,
    121,
    122,
    123,
    124,
    125,
    126,
    127,
}

CANONICAL_8 = {
    0: (0,),
    1: (1,),
    5: (5, 6),
    7: (7,),
    21: (21, 22, 25, 26),
    23: (23, 27),
    85: (85, 86, 89, 90, 101, 102, 105, 106),
    87: (87, 91, 103, 107),
    113: (113, 114, 116, 120),
    115: (115, 124),
    117: (117, 118, 121, 122),
    119: (119, 123, 125, 126),
    127: (127,),
}


def profile_compute(profile, input_value, threshold, branch_loss):
    child_fraction = (1.0 - branch_loss) * 0.5
    terminal_value = input_value
    output = 0.0
    for depth in range(profile.max_depth + 1):
        if terminal_value >= threshold:
            output += profile.terminals[depth] * terminal_value
        terminal_value *= child_fraction
    return output


class Kdse8ReferenceTests(unittest.TestCase):
    def test_complete_container_space(self):
        for value in range(256):
            expected = (value & 0x7F) in VALID_8
            self.assertEqual(kdse8_validate(value) == KDSE_STATUS_OK, expected)

    def test_payload_and_length_ignore_bit_7(self):
        self.assertEqual(kdse8_payload(0x85), 5)
        self.assertEqual(kdse8_payload_length(0), 1)
        self.assertEqual(kdse8_payload_length(0x85), 3)

    def test_status_precedence_matches_c(self):
        self.assertEqual(kdse8_validate(2), KDSE_STATUS_INVALID_PAYLOAD_LENGTH)
        self.assertEqual(
            kdse8_validate(4), KDSE_STATUS_EXPLICIT_FINAL_TERMINAL_LEVEL
        )
        self.assertEqual(
            kdse8_validate(17), KDSE_STATUS_DATA_AFTER_TERMINAL_LEVEL
        )
        self.assertEqual(kdse8_validate(31), KDSE_STATUS_TRUNCATED_LEVEL)

    def test_authoritative_profiles(self):
        cases = {
            0: ((1, 0, 0, 0, 0), 1, 0, 0, 1),
            1: ((0, 2, 0, 0, 0), 1, 1, 1, 2),
            5: ((0, 1, 2, 0, 0), 3, 2, 2, 3),
            7: ((0, 0, 4, 0, 0), 3, 2, 3, 4),
            21: ((0, 1, 1, 2, 0), 5, 3, 3, 4),
            127: ((0, 0, 0, 8, 0), 7, 3, 7, 8),
        }
        for value, expected in cases.items():
            profile = kdse8_decode_profile(value)
            actual = (
                profile.terminals,
                profile.payload_length,
                profile.max_depth,
                profile.branch_count,
                profile.terminal_count,
            )
            self.assertEqual(actual, expected)

    def test_complete_ordered_classes(self):
        for ordered, members in CANONICAL_8.items():
            for member in members:
                self.assertEqual(kdse8_canonicalize(member), ordered)
                self.assertEqual(kdse8_canonicalize(member | 0x80), ordered)
                self.assertEqual(kdse8_is_ordered(member), member == ordered)

    def test_checked_operations_raise_status_error(self):
        with self.assertRaises(KdseValidationError) as caught:
            kdse8_decode_profile(4)
        self.assertEqual(
            caught.exception.status,
            KDSE_STATUS_EXPLICIT_FINAL_TERMINAL_LEVEL,
        )

    def test_compute_complete_space(self):
        inputs = (0.0, 0.01, 0.3, 1.0, 5.0, 100.0)
        thresholds = (0.01, 0.3, 2.0)
        losses = (0.0, 0.17, 0.5, 0.99)
        for payload in sorted(VALID_8):
            profile = kdse8_decode_profile(payload)
            for input_value in inputs:
                for threshold in thresholds:
                    for loss in losses:
                        expected = profile_compute(
                            profile, input_value, threshold, loss
                        )
                        actual = kdse8_compute(
                            payload, input_value, threshold, loss
                        )
                        with_reserved = kdse8_compute(
                            payload | 0x80, input_value, threshold, loss
                        )
                        self.assertTrue(
                            math.isclose(actual, expected, rel_tol=1e-12)
                        )
                        self.assertEqual(actual, with_reserved)

    def test_known_compute_value(self):
        self.assertTrue(
            math.isclose(
                kdse8_compute(119, 5.0, 0.3, 0.17),
                3.00532625,
                rel_tol=1e-12,
            )
        )


class Kdse16ReferenceTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.valid = [
            value
            for value in range(0x8000)
            if kdse16_validate(value) == KDSE_STATUS_OK
        ]

    def test_complete_payload_space(self):
        self.assertEqual(len(self.valid), 4397)
        for value in range(0x8000):
            self.assertEqual(
                kdse16_validate(value), kdse16_validate(value | 0x8000)
            )

    def test_payload_and_length_ignore_bit_15(self):
        self.assertEqual(kdse16_payload(0x8005), 5)
        self.assertEqual(kdse16_payload_length(0), 1)
        self.assertEqual(kdse16_payload_length(0x8005), 3)

    def test_profile_invariants_for_complete_space(self):
        for payload in self.valid:
            profile = kdse16_decode_profile(payload)
            self.assertEqual(profile.payload_length, payload.bit_length() or 1)
            self.assertEqual(profile.payload_length % 2, 1)
            self.assertLessEqual(profile.max_depth, 8)
            self.assertEqual(
                profile.terminal_count, profile.branch_count + 1
            )
            kraft = sum(
                profile.terminals[depth]
                << (profile.max_depth - depth)
                for depth in range(profile.max_depth + 1)
            )
            self.assertEqual(kraft, 1 << profile.max_depth)

    def test_authoritative_deep_and_full_profiles(self):
        chain = kdse16_decode_profile(0x5555)
        self.assertEqual(chain.terminals, (0, 1, 1, 1, 1, 1, 1, 1, 2))
        self.assertEqual((chain.max_depth, chain.branch_count), (8, 8))

        full = kdse16_decode_profile(0x7FFF)
        self.assertEqual(full.terminals, (0, 0, 0, 0, 16, 0, 0, 0, 0))
        self.assertEqual((full.max_depth, full.branch_count), (4, 15))

    def test_ordered_space_count_and_properties(self):
        ordered_count = 0
        for payload in self.valid:
            ordered = kdse16_canonicalize(payload)
            self.assertLessEqual(ordered, payload)
            self.assertEqual(kdse16_validate(ordered), KDSE_STATUS_OK)
            self.assertEqual(kdse16_canonicalize(ordered), ordered)
            self.assertEqual(kdse16_canonicalize(payload | 0x8000), ordered)
            is_ordered = kdse16_is_ordered(payload)
            self.assertEqual(is_ordered, ordered == payload)
            ordered_count += is_ordered
        self.assertEqual(ordered_count, 198)

    def test_compute_complete_space(self):
        inputs = (0.0, 0.3, 1.0, 5.0, 100.0)
        thresholds = (0.01, 0.3, 2.0)
        losses = (0.0, 0.17, 0.5, 0.99)
        for payload in self.valid:
            profile = kdse16_decode_profile(payload)
            for input_value in inputs:
                for threshold in thresholds:
                    for loss in losses:
                        expected = profile_compute(
                            profile, input_value, threshold, loss
                        )
                        actual = kdse16_compute(
                            payload, input_value, threshold, loss
                        )
                        with_reserved = kdse16_compute(
                            payload | 0x8000, input_value, threshold, loss
                        )
                        self.assertTrue(
                            math.isclose(actual, expected, rel_tol=1e-12)
                        )
                        self.assertEqual(actual, with_reserved)


class CommonApiTests(unittest.TestCase):
    def test_status_strings_match_c(self):
        self.assertEqual(kdse_status_string(KDSE_STATUS_OK), "valid KDSE payload")
        self.assertEqual(kdse_status_string(999), "unknown KDSE status")

    def test_python_rejects_values_outside_physical_container(self):
        with self.assertRaises(ValueError):
            kdse8_validate(256)
        with self.assertRaises(ValueError):
            kdse16_validate(65536)
        with self.assertRaises(TypeError):
            kdse8_validate(True)


if __name__ == "__main__":
    unittest.main()
