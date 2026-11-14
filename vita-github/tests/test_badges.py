"""Regression tests for PS Vita badge readability (no network needed)."""
import sys
import unittest
from pathlib import Path
from unittest.mock import patch

from bs4 import BeautifulSoup

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import generate


class BadgeTests(unittest.TestCase):
    def test_static_badges_keep_text_color_and_center_alignment(self):
        content = """
<div align="center">
 <a href="https://github.com/WolffsRoom/FileManager/releases/tag/v0.30-beta"><img src="https://img.shields.io/badge/RELEASE-v0.30--beta-377bb5?style=for-the-badge" alt="File Manager v0.30 Beta"></a>
 <img src="https://img.shields.io/badge/STATUS-BETA-4c9aff?style=for-the-badge" alt="Beta status">
 <img src="https://img.shields.io/badge/PLATFORM-PS%20VITA-6f42c1?style=for-the-badge&amp;logo=playstation&amp;logoColor=white" alt="PlayStation Vita">
</div>
"""
        output = generate.markdown_html(
            content, "WolffsRoom", "FileManager", "master", "",
            generate.repo_page("WolffsRoom", "FileManager"), {}
        )
        soup = BeautifulSoup(output, "html.parser")
        badges = soup.select(".vita-badge")
        self.assertEqual(len(badges), 3)
        self.assertEqual(["RELEASE", "STATUS", "PLATFORM"],
                         [x.select_one(".vita-badge-label").text for x in badges])
        self.assertEqual(["V0.30-BETA", "BETA", "PS VITA"],
                         [x.select_one(".vita-badge-value").text for x in badges])
        self.assertEqual(["#377bb5", "#4c9aff", "#6f42c1"],
                         [x.select_one(".vita-badge-value")["style"].split(":")[-1] for x in badges])
        self.assertIn("align-center", soup.find("div")["class"])
        self.assertIsNotNone(soup.select_one("a .vita-badge"))
        self.assertIsNone(soup.select_one('img[src*="shields.io"]'))

    def test_dynamic_license_badge_fallback_is_readable(self):
        url = "https://img.shields.io/github/license/WolffsRoom/FileManager?style=for-the-badge&color=2ea44f"
        from requests import RequestException
        with patch.object(generate.SESSION, "get", side_effect=RequestException("offline")):
            label, value, color = generate.shields_badge_data(url, "GPL-3.0 license")
        self.assertEqual((label.upper(), value.upper(), color), ("LICENSE", "GPL-3.0", "#2ea44f"))

    def test_dynamic_svg_title_text(self):
        class Response:
            content = b'<svg xmlns="http://www.w3.org/2000/svg" aria-label="downloads: 9.7k"><text>downloads</text><text>downloads</text><text>9.7k</text><text>9.7k</text></svg>'
            def raise_for_status(self):
                pass

        with patch.object(generate.SESSION, "get", return_value=Response()):
            label, value, color = generate.shields_badge_data(
                "https://img.shields.io/github/downloads/WolffsRoom/FileManager/total",
                "Downloads"
            )
        self.assertEqual((label, value), ("downloads", "9.7k"))

    def test_color_sanitization(self):
        self.assertEqual(generate.badge_color("brightgreen"), "#4c1")
        self.assertEqual(generate.badge_color("red; position:absolute"), "#4c9aff")


if __name__ == "__main__":
    unittest.main()
