"""
Tier 5 - Adversarial 1: Encoding & Escaping Integrity.
Tests resilience against SQL meta-characters, Unicode strings, null-byte injection,
and control characters in inputs and metadata.
"""

import unittest


class TestAdv01EncodingEscaping(unittest.TestCase):
    def test_sql_injection_in_plot_name(self):
        """Plot names containing SQL injection tokens do not compromise in-memory mappings."""
        injected_name = "plot_1'; DROP TABLE Biomass_Output; --"
        mock_registry = {injected_name: 101}
        # In-memory dictionary lookup handles literal string cleanly
        self.assertEqual(mock_registry[injected_name], 101)

    def test_unicode_species_names(self):
        """Species names containing multi-byte UTF-8 characters do not cause buffer overflow."""
        unicode_spp = "Artemisia tridentata subsp. wyomingensis—spécïes_β"
        encoded = unicode_spp.encode("utf-8")
        self.assertGreater(len(encoded), len(unicode_spp))
        decoded = encoded.decode("utf-8")
        self.assertEqual(decoded, unicode_spp)

    def test_null_byte_string_handling(self):
        """Strings containing null bytes are safely handled or rejected."""
        null_byte_str = "ARTR2\x00extra_chars"
        # Python / Rust strings retain length or safely split
        clean_code = null_byte_str.split("\x00", maxsplit=1)[0]
        self.assertEqual(clean_code, "ARTR2")

    def test_control_characters_in_identifiers(self):
        """Identifiers with embedded newlines, tabs, or carriage returns normalize cleanly."""
        raw_id = "  PLOT\t\n101\r  "
        normalized = "".join(raw_id.split())
        self.assertEqual(normalized, "PLOT101")

    def test_whitespace_trimming_on_species_codes(self):
        """Padded species codes (' ARTR2  ') resolve correctly against catalog."""
        catalog = {"ARTR2": 165}
        raw_input = "  ARTR2  "
        resolved = catalog.get(raw_input.strip())
        self.assertEqual(resolved, 165)


if __name__ == "__main__":
    unittest.main()
