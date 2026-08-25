# Copyright (C) 2026 Mark Karaman
# SPDX-License-Identifier: AGPL-3.0-or-later

from __future__ import annotations

import unittest

from kdse import (
    int_to_kdse,
    is_valid_kdse,
    kdse_to_int,
    kdse_to_mermaid,
    mermaid_to_kdse,
    parse_mermaid,
    tree_to_mermaid,
    validate_kdse_bits,
)


class StructuralHelpersTests(unittest.TestCase):
    def test_integer_helpers_are_checked_both_directions(self):
        self.assertEqual(int_to_kdse(7825), "1111010010001")
        self.assertEqual(kdse_to_int("1111010010001"), 7825)
        for invalid in ("100", "001", "1100", "1x1", ""):
            self.assertFalse(is_valid_kdse(invalid))
            with self.assertRaises(ValueError):
                validate_kdse_bits(invalid)
        with self.assertRaises(ValueError):
            kdse_to_int("100")
        with self.assertRaises(ValueError):
            int_to_kdse(4)
        with self.assertRaises(ValueError):
            int_to_kdse(True)

    def test_single_node(self):
        bits, values = mermaid_to_kdse('flowchart LR\nA("42")')
        self.assertEqual((bits, values), ("0", ["42"]))
        self.assertEqual(
            kdse_to_mermaid("0", ["42"]),
            'flowchart LR\n    A("42")',
        )

    def test_nonordered_tree_is_preserved(self):
        source = """
        flowchart LR
            A("42") --> B("91")
            A --> C("7")
            B --> D("13")
            B --> E("68")
        """
        bits, values = mermaid_to_kdse(source)
        self.assertEqual(bits, "110")
        self.assertEqual(values, ["42", "91", "7", "13", "68"])
        self.assertEqual(mermaid_to_kdse(kdse_to_mermaid(bits, values)), (bits, values))

    def test_original_7825_vector(self):
        source = """
        flowchart LR
            A["42"] --> B["91"]
            A --> C["7"]
            B --> D["13"]
            B --> E["68"]
            D --> F["55"]
            D --> G["3"]
            G --> H["77"]
            G --> I["29"]
            I --> J["84"]
            I --> K["11"]
            C --> L["63"]
            C --> M["18"]
            L --> N["37"]
            L --> O["96"]
        """
        bits, values = mermaid_to_kdse(source)
        self.assertEqual(bits, "1111010010001")
        self.assertEqual(kdse_to_int(bits), 7825)
        self.assertEqual(
            values,
            [
                "42",
                "91",
                "7",
                "13",
                "68",
                "63",
                "18",
                "55",
                "3",
                "37",
                "96",
                "77",
                "29",
                "84",
                "11",
            ],
        )

    def test_comments_containing_mermaid_are_ignored(self):
        source = """
        %% X["not a node"] --> Y["not a node"]
        flowchart LR
            A("1") --> B("2") & C("3") %% D("x") --> E("y")
        """
        self.assertEqual(mermaid_to_kdse(source), ("1", ["1", "2", "3"]))

    def test_chrome_is_ignored_but_subgraph_nodes_participate(self):
        source = """
        %%{init: {'theme': 'neutral'}}%%
        flowchart LR
            subgraph demo
                A("1") --> B("2")
                A --> C("3")
            end
            classDef foo fill:#f9f
            class A foo
            style A fill:#bbf
            linkStyle 0 stroke:#f00
            click A "https://example.com"
        """
        self.assertEqual(mermaid_to_kdse(source), ("1", ["1", "2", "3"]))

    def test_arrow_chains_and_groups_follow_source_order(self):
        source = """
        flowchart LR
            A("a") --> B("b") & C("c")
            B --> D("d") & E("e")
        """
        self.assertEqual(
            mermaid_to_kdse(source),
            ("110", ["a", "b", "c", "d", "e"]),
        )

    def test_empty_duplicate_and_special_values_round_trip(self):
        values = [
            "",
            "same",
            "same",
            'hello "world" [x] (y)',
            "line 1\nline 2\r#quot; <tag> #35;",
        ]
        mermaid = kdse_to_mermaid("110", values)
        self.assertEqual(mermaid_to_kdse(mermaid), ("110", values))

    def test_comment_and_arrow_tokens_inside_values_are_data(self):
        source = """
        flowchart LR
            A["root %% --> & ; ( raw"] --> B["upper"]
            A --> C["lower"] %% actual comment
        """
        self.assertEqual(
            mermaid_to_kdse(source),
            ("1", ["root %% --> & ; ( raw", "upper", "lower"]),
        )

    def test_exact_value_count_and_types(self):
        with self.assertRaises(ValueError):
            kdse_to_mermaid("1", ["a"])
        with self.assertRaises(ValueError):
            kdse_to_mermaid("1", ["a", "b", "c", "extra"])
        with self.assertRaises(TypeError):
            kdse_to_mermaid("0", [42])

    def test_invalid_mermaid_is_rejected(self):
        cases = (
            'flowchart TD\nA("1")',
            'A("1")',
            'flowchart LR\nA("1") --> B\nA --> C("3")',
            'flowchart LR\nA("1") --> B("2")',
            'flowchart LR\nA("1") --> B("2")\nC("3") --> B',
            'flowchart LR\nA("1") -->|edge label| B("2")',
            'flowchart LR\nA("1")\nA("2")',
        )
        for source in cases:
            with self.subTest(source=source), self.assertRaises(ValueError):
                parse_mermaid(source)

    def test_tree_output_uses_canonical_bfs_ids(self):
        mermaid = tree_to_mermaid(
            {"root": ["upper", "lower"]},
            {"root": "r", "upper": "u", "lower": "l"},
            "root",
        )
        self.assertEqual(
            mermaid,
            "\n".join(
                (
                    "flowchart LR",
                    '    A("r")',
                    '    B("u")',
                    '    C("l")',
                    "    A --> B",
                    "    A --> C",
                )
            ),
        )

    def test_all_kdse16_payloads_round_trip(self):
        valid_count = 0
        for payload in range(0x8000):
            bits = bin(payload)[2:]
            if not is_valid_kdse(bits):
                continue
            valid_count += 1
            values = [f"v{i}" for i in range(2 * bits.count("1") + 1)]
            self.assertEqual(
                mermaid_to_kdse(kdse_to_mermaid(bits, values)),
                (bits, values),
            )
        self.assertEqual(valid_count, 4397)


if __name__ == "__main__":
    unittest.main()
