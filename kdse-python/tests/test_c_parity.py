# Copyright (C) 2026 Mark Karaman
# SPDX-License-Identifier: AGPL-3.0-or-later

from __future__ import annotations

import ctypes
import math
import os
from pathlib import Path
import shutil
import subprocess
import tempfile
import unittest

from kdse import (
    KDSE_STATUS_OK,
    kdse8_canonicalize,
    kdse8_compute,
    kdse8_decode_profile,
    kdse8_validate,
    kdse16_canonicalize,
    kdse16_compute,
    kdse16_decode_profile,
    kdse16_validate,
)


class CProfile8(ctypes.Structure):
    _fields_ = [
        ("terminals", ctypes.c_uint8 * 5),
        ("payload_length", ctypes.c_uint8),
        ("max_depth", ctypes.c_uint8),
        ("branch_count", ctypes.c_uint8),
        ("terminal_count", ctypes.c_uint8),
    ]


class CProfile16(ctypes.Structure):
    _fields_ = [
        ("terminals", ctypes.c_uint8 * 9),
        ("payload_length", ctypes.c_uint8),
        ("max_depth", ctypes.c_uint8),
        ("branch_count", ctypes.c_uint8),
        ("terminal_count", ctypes.c_uint8),
    ]


class CReferenceParityTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        root_text = os.environ.get("KDSE_C_REFERENCE_ROOT")
        if not root_text:
            raise unittest.SkipTest("KDSE_C_REFERENCE_ROOT is not set")
        if shutil.which("cc") is None:
            raise unittest.SkipTest("C compiler 'cc' is unavailable")
        cls.root = Path(root_text).resolve()
        required = (
            cls.root / "src/kdse8_checked.c",
            cls.root / "src/kdse8_compute.c",
            cls.root / "src/kdse16_checked.c",
            cls.root / "src/kdse16_compute.c",
        )
        if any(not path.is_file() for path in required):
            raise unittest.SkipTest("KDSE_C_REFERENCE_ROOT is not a C source tree")

        cls.temp = tempfile.TemporaryDirectory(prefix="kdse-c-parity-")
        shared = Path(cls.temp.name) / "libkdse_reference.so"
        subprocess.run(
            [
                "cc",
                "-std=c11",
                "-Wall",
                "-Wextra",
                "-Wpedantic",
                "-Werror",
                "-shared",
                "-fPIC",
                f"-I{cls.root / 'include'}",
                *(str(path) for path in required),
                "-o",
                str(shared),
            ],
            check=True,
            capture_output=True,
            text=True,
        )
        cls.c = ctypes.CDLL(str(shared))

        cls.c.kdse8_validate.argtypes = [ctypes.c_uint8]
        cls.c.kdse8_validate.restype = ctypes.c_int
        cls.c.kdse16_validate.argtypes = [ctypes.c_uint16]
        cls.c.kdse16_validate.restype = ctypes.c_int

        cls.c.kdse8_decode_profile.argtypes = [
            ctypes.c_uint8,
            ctypes.POINTER(CProfile8),
        ]
        cls.c.kdse8_decode_profile.restype = ctypes.c_int
        cls.c.kdse16_decode_profile.argtypes = [
            ctypes.c_uint16,
            ctypes.POINTER(CProfile16),
        ]
        cls.c.kdse16_decode_profile.restype = ctypes.c_int

        cls.c.kdse8_canonicalize.argtypes = [
            ctypes.c_uint8,
            ctypes.POINTER(ctypes.c_uint8),
        ]
        cls.c.kdse8_canonicalize.restype = ctypes.c_int
        cls.c.kdse16_canonicalize.argtypes = [
            ctypes.c_uint16,
            ctypes.POINTER(ctypes.c_uint16),
        ]
        cls.c.kdse16_canonicalize.restype = ctypes.c_int

        for name, c_value in (("kdse8_compute", ctypes.c_uint8), ("kdse16_compute", ctypes.c_uint16)):
            function = getattr(cls.c, name)
            function.argtypes = [
                c_value,
                ctypes.c_double,
                ctypes.c_double,
                ctypes.c_double,
            ]
            function.restype = ctypes.c_double

    @classmethod
    def tearDownClass(cls):
        if hasattr(cls, "temp"):
            cls.temp.cleanup()

    def test_validation_statuses_all_physical_containers(self):
        for value in range(256):
            self.assertEqual(int(kdse8_validate(value)), self.c.kdse8_validate(value))
        for value in range(65536):
            self.assertEqual(int(kdse16_validate(value)), self.c.kdse16_validate(value))

    def test_profiles_and_canonicalization_all_valid_payloads(self):
        configurations = (
            (
                0x80,
                self.c.kdse8_validate,
                self.c.kdse8_decode_profile,
                self.c.kdse8_canonicalize,
                CProfile8,
                ctypes.c_uint8,
                kdse8_decode_profile,
                kdse8_canonicalize,
            ),
            (
                0x8000,
                self.c.kdse16_validate,
                self.c.kdse16_decode_profile,
                self.c.kdse16_canonicalize,
                CProfile16,
                ctypes.c_uint16,
                kdse16_decode_profile,
                kdse16_canonicalize,
            ),
        )
        for limit, validate, decode, canonicalize, Profile, CValue, py_decode, py_canonicalize in configurations:
            for payload in range(limit):
                if validate(payload) != int(KDSE_STATUS_OK):
                    continue
                c_profile = Profile()
                self.assertEqual(decode(payload, ctypes.byref(c_profile)), 0)
                py_profile = py_decode(payload)
                self.assertEqual(py_profile.terminals, tuple(c_profile.terminals))
                self.assertEqual(py_profile.payload_length, c_profile.payload_length)
                self.assertEqual(py_profile.max_depth, c_profile.max_depth)
                self.assertEqual(py_profile.branch_count, c_profile.branch_count)
                self.assertEqual(py_profile.terminal_count, c_profile.terminal_count)

                c_ordered = CValue()
                self.assertEqual(canonicalize(payload, ctypes.byref(c_ordered)), 0)
                self.assertEqual(py_canonicalize(payload), c_ordered.value)

    def test_compute_grid_all_valid_payloads(self):
        configurations = (
            (0x80, self.c.kdse8_validate, self.c.kdse8_compute, kdse8_compute),
            (0x8000, self.c.kdse16_validate, self.c.kdse16_compute, kdse16_compute),
        )
        inputs = (0.0, 0.3, 1.0, 5.0, 100.0)
        thresholds = (0.01, 0.3, 2.0)
        losses = (0.0, 0.17, 0.5, 0.99)
        for limit, validate, c_compute, py_compute in configurations:
            for payload in range(limit):
                if validate(payload) != 0:
                    continue
                for input_value in inputs:
                    for threshold in thresholds:
                        for loss in losses:
                            expected = c_compute(payload, input_value, threshold, loss)
                            actual = py_compute(payload, input_value, threshold, loss)
                            self.assertTrue(
                                math.isclose(actual, expected, rel_tol=1e-15, abs_tol=0.0)
                            )


if __name__ == "__main__":
    unittest.main()

