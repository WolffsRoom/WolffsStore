"""Dark-only GitHub README theme tests for the original PS Vita browser."""
import sys
import unittest
from pathlib import Path
from unittest.mock import patch

from bs4 import BeautifulSoup
sys.path.insert(0, str(Path(__file__).resolve().parents[1]))
import generate


class ThemeAndReadabilityTests(unittest.TestCase):
    def render(self, sample):
        return generate.markdown_html(
            sample, "WolffsRoom", "FileManager", "master", "",
            generate.repo_page("WolffsRoom", "FileManager"), {}
        )

    def test_filemanager_light_logo_is_dropped_and_dark_logo_remains(self):
        sample = """<p align="center">
          <img src="Release/Logo%20preta.png#gh-light-mode-only"
               alt="File Manager for PlayStation Vita" width="700">
          <img src="Release/Logo%20branca.png#gh-dark-mode-only"
               alt="File Manager for PlayStation Vita" width="700">
        </p>"""
        with patch.object(generate, "mirror_image", side_effect=lambda src,*args: "LOCAL-" + src) as mock:
            rendered = self.render(sample)
        soup = BeautifulSoup(rendered, "html.parser")
        images = soup.select("p img")
        self.assertEqual(len(images), 1, rendered)
        self.assertEqual(images[0]["src"], "LOCAL-Release/Logo%20branca.png#gh-dark-mode-only")
        self.assertEqual(images[0]["width"], "700")
        self.assertEqual(mock.call_count, 1)
        self.assertIn("align-center", soup.select_one("p")["class"])

    def test_picture_uses_dark_source_without_modern_css(self):
        sample = """<picture>
          <source media="(prefers-color-scheme: dark)" srcset="assets/dark.png">
          <source media="(prefers-color-scheme: light)" srcset="assets/light.png">
          <img src="assets/light.png" alt="test" width="240">
        </picture>"""
        with patch.object(generate, "mirror_image", side_effect=lambda src,*args: src) as mock:
            rendered = self.render(sample)
        soup = BeautifulSoup(rendered, "html.parser")
        self.assertEqual(len(soup.select("img")), 1)
        self.assertEqual(soup.select_one("img")["src"], "assets/dark.png")
        self.assertEqual(mock.call_count, 1)
        self.assertEqual(len(soup.select("source,picture")), 0)

    def test_filemanager_badges_are_native_readable_html(self):
        sample = """<div align="center">
          <a href="https://github.com/WolffsRoom/FileManager/releases/tag/v0.30-beta">
          <img src="https://img.shields.io/badge/RELEASE-v0.30--beta-377bb5?style=for-the-badge"
          alt="File Manager v0.30 Beta"></a>
          <img src="https://img.shields.io/badge/STATUS-BETA-4c9aff?style=for-the-badge" alt="Beta status">
          <img src="https://img.shields.io/badge/PLATFORM-PS%20VITA-6f42c1?style=for-the-badge"
               alt="PlayStation Vita"></div>"""
        rendered = self.render(sample)
        soup = BeautifulSoup(rendered, "html.parser")
        badges = soup.select("span.vita-badge")
        self.assertEqual(len(badges), 3)
        self.assertEqual([x.select_one(".vita-badge-value").text for x in badges],
                         ["V0.30-BETA", "BETA", "PS VITA"])
        for badge in badges:
            self.assertIn("font-size:15px", badge.get("style", ""))
            self.assertIn("line-height:30px", badge.get("style", ""))
            self.assertIn("background-color:", badge.select_one(".vita-badge-value")["style"])
        self.assertEqual(len(soup.select('img[src*="shields.io"]')), 0)


if __name__ == "__main__":
    unittest.main()
