"""CapUnlocker notice must be optional and only appear for flagged installs."""
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

class PluginAdvisoryTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.main = (ROOT / "src/main.c").read_text("utf-8")
        cls.catalog = (ROOT / "src/catalog.h").read_text("utf-8")
        cls.parse = (ROOT / "src/store.c").read_text("utf-8")
        cls.sync = (ROOT / "src/sync.h").read_text("utf-8")

    def test_advisory_metadata_round_trip(self):
        self.assertIn("*advisory_plugin;", self.catalog)
        self.assertIn('S(d, "aviso_plugin")', self.parse)
        self.assertIn("store-v103.json", self.sync)
        self.assertIn("store-v103.json", self.sync)

    def test_installer_shows_optional_warning_on_vpk_only(self):
        self.assertIn("static void open_plugin_advice(", self.main)
        self.assertIn("d->type != DL_VPK", self.main)
        self.assertIn("static void draw_plugin_advice(", self.main)
        self.assertIn("CapUnlocker", self.main)
        self.assertIn("Optional. The app can be installed without this plugin.", self.main)
        self.assertIn("Opcional. A instalação também funciona sem esse plugin.", self.main)
        self.assertIn("TXT(S_CAP_TITLE)", self.main)
        self.assertIn("TXT(S_CAP_CPU)", self.main)

    def test_click_controls_and_background_protection(self):
        self.assertIn("close_plugin_advice(0)", self.main)
        self.assertIn("close_plugin_advice(1)", self.main)
        self.assertIn("draw_plugin_advice();", self.main)
        self.assertIn("&& plugin_advice_post < 0)", self.main)
        self.assertIn("jobs_add(p, di", self.main)
        self.assertIn("advisory_plugin[0]", self.main)

if __name__ == "__main__":
    unittest.main()
