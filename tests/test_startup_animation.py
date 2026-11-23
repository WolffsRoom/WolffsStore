"""Regression checks for the real layered Wolff's Room cold-start animation."""
import struct
import unittest
import zipfile
from pathlib import Path

from PIL import Image, ImageChops

ROOT = Path(__file__).resolve().parents[1]
SOURCE = ROOT / 'assets' / 'Animação'
TEX = ROOT / 'data' / 'tex'
LIVE = ROOT / 'assets' / 'livearea' / 'startup.png'


class StartupAnimationTests(unittest.TestCase):
    def test_original_layers_exactly_reconstruct_wordmark(self):
        final = Image.open(SOURCE / "Logo Wolff's Room.png").convert("RGBA")
        self.assertEqual(final.size, (786, 108))
        composed = Image.new('RGBA', final.size, (0, 0, 0, 0))
        for number in (3, 2, 1):
            source = Image.open(SOURCE / f'{number} Camada.png').convert('RGBA')
            self.assertEqual(source.size, (786, 108))
            composed.alpha_composite(source)
        self.assertIsNone(ImageChops.difference(composed, final).getbbox())

    def test_embedded_layers_are_pixel_exact_half_scale(self):
        for number in (3, 2, 1):
            original = Image.open(SOURCE / f'{number} Camada.png').convert('RGBA')
            half = original.resize((393, 54), Image.Resampling.NEAREST)
            data = (TEX / f'startup_layer{number}.wrt').read_bytes()
            self.assertEqual(data[:4], b'WRT1')
            self.assertEqual(struct.unpack('<HH', data[4:8]), (393, 54))
            self.assertEqual(data[8:], half.tobytes())

    def test_livearea_png_is_280x158_indexed_with_transparency(self):
        with Image.open(LIVE) as img:
            self.assertEqual(img.size, (280, 158))
            self.assertEqual(img.mode, 'P')
            self.assertEqual(img.convert('RGBA').getchannel('A').getextrema(), (0, 255))

    def test_real_animation_integrated_and_regenerable(self):
        main = (ROOT / 'src' / 'main.c').read_text(encoding='utf-8')
        generator = (ROOT / 'tools' / 'gen_data.py').read_text(encoding='utf-8')
        audio = (ROOT / 'src' / 'audio.c').read_text(encoding='utf-8')
        for sound in ('logo_fall', 'logo_hit3', 'logo_hit2', 'logo_hit1'):
            self.assertIn(sound, main)
            self.assertIn(sound, audio)
        self.assertIn('now_s - first_sync_t0 >= STARTUP_FINISH + STARTUP_NOTICE_MIN', main)
        self.assertIn('static void draw_first_sync(void)', main)
        self.assertIn('if (elapsed < 0.0) return;', main)
        self.assertIn('if (st == SYNC_RUNNING && elapsed >= 3.4)', main)
        self.assertIn('title_draw(ch, cx, y - bounce, scale, C_WHITE);', main)
        self.assertIn('subtitles[2][5]', main)
        self.assertIn('startup_load_layers();', main)
        self.assertIn('startup_tick_sounds();', main)
        self.assertIn("f'startup_layer{number}.wrt'", generator)

    def test_compiled_vpk_contains_new_assets_and_current_titleid(self):
        package = ROOT / 'artifacts' / 'current' / 'WolffsStore.vpk'
        if not package.exists():
            self.skipTest('Build VPK before packaging test')
        with zipfile.ZipFile(package) as archive:
            self.assertIn(b'WFFS00042', archive.read('sce_sys/param.sfo'))
            for number in (1, 2, 3):
                self.assertEqual(
                    archive.read(f'data/tex/startup_layer{number}.wrt'),
                    (TEX / f'startup_layer{number}.wrt').read_bytes())
            self.assertEqual(
                archive.read('sce_sys/livearea/contents/startup.png'),
                LIVE.read_bytes())
            self.assertIn('updater_v2/eboot.bin', archive.namelist())


if __name__ == '__main__':
    unittest.main()
