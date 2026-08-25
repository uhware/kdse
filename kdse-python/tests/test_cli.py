# Copyright (C) 2026 Mark Karaman
# SPDX-License-Identifier: AGPL-3.0-or-later

from __future__ import annotations

from contextlib import redirect_stderr, redirect_stdout
from io import StringIO
import unittest

from kdse.cli import main8, main16


class CliTests(unittest.TestCase):
    def run_cli(self, function, arguments):
        stdout = StringIO()
        stderr = StringIO()
        with redirect_stdout(stdout), redirect_stderr(stderr):
            status = function(arguments)
        return status, stdout.getvalue(), stderr.getvalue()

    def test_kdse8_validate_and_reserved_bit(self):
        status, stdout, stderr = self.run_cli(main8, ["validate", "0x85"])
        self.assertEqual(status, 0)
        self.assertEqual(stderr, "")
        self.assertIn("valid: yes", stdout)
        self.assertIn("unassigned-bit: 1 (ignored)", stdout)
        self.assertIn("payload-bits: 101", stdout)

    def test_kdse8_invalid(self):
        status, stdout, stderr = self.run_cli(main8, ["validate", "0b100"])
        self.assertEqual(status, 1)
        self.assertEqual(stderr, "")
        self.assertIn("valid: no", stdout)
        self.assertIn("final all-terminal level must be omitted", stdout)

    def test_kdse8_canonicalize(self):
        status, stdout, stderr = self.run_cli(
            main8, ["canonicalize", "0b110"]
        )
        self.assertEqual((status, stderr), (0, ""))
        self.assertIn("input-payload: 110", stdout)
        self.assertIn("ordered-payload: 101", stdout)

    def test_kdse16_profile(self):
        status, stdout, stderr = self.run_cli(main16, ["profile", "0x5555"])
        self.assertEqual((status, stderr), (0, ""))
        self.assertIn("branches: 8", stdout)
        self.assertIn("terminals: 9", stdout)
        self.assertIn("max-depth: 8", stdout)

    def test_kdse16_compute(self):
        status, stdout, stderr = self.run_cli(
            main16,
            [
                "compute",
                "0x7fff",
                "--input",
                "100",
                "--threshold",
                "1",
                "--loss-percent",
                "0",
            ],
        )
        self.assertEqual((status, stderr), (0, ""))
        self.assertIn("output: 100", stdout)


if __name__ == "__main__":
    unittest.main()

