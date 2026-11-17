"""Contracts for dates in Wolff's Store, compatible with old catalogs."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]

class PostDateContracts(unittest.TestCase):
    def test_optional_update_field_in_both_catalog_formats(self):
        h = (ROOT / "src/catalog.h").read_text("utf-8")
        js = (ROOT / "src/store.c").read_text("utf-8")
        offline = (ROOT / "src/catalog.c").read_text("utf-8")
        gen = (ROOT / "tools/gen_data.py").read_text("utf-8")
        self.assertIn("char updated[16]", h)
        self.assertIn('json_object_get(o, "atualizado_em")', js)
        self.assertIn('if (!strcmp(k, "updated"))', offline)
        self.assertIn("if p.get('updated'):", gen)

    def test_post_progress_uses_colored_vita_palette(self):
        gfx = (ROOT / "src/gfx.h").read_text("utf-8")
        main = (ROOT / "src/main.c").read_text("utf-8")
        self.assertIn("C_PROGRESS_LOW", gfx)
        self.assertIn("C_PROGRESS_MID", gfx)
        self.assertIn("C_PROGRESS_HIGH", gfx)
        self.assertIn("pct < 40 ? C_PROGRESS_LOW : pct < 80 ? C_PROGRESS_MID", main)
        self.assertIn("sw, 16, progress_color(pct)", main)

    def test_details_renders_both_dates_only_when_available(self):
        main = (ROOT / "src/main.c").read_text("utf-8")
        self.assertIn('"Published on", "Last update"', main)
        self.assertIn('"Publicado em", "Atualizado em"', main)
        self.assertIn("post_date_display(p->date", main)
        self.assertIn("if (post_date_display(p->updated", main)
        self.assertIn("snprintf(line + used", main)

if __name__ == "__main__":
    unittest.main()
