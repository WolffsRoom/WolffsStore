from pathlib import Path
import re
import unittest


ROOT = Path(__file__).resolve().parents[1]


class ReleaseInvariants(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")
        cls.main = (ROOT / "src/main.c").read_text(encoding="utf-8")
        cls.video = (ROOT / "src/video.c").read_text(encoding="utf-8")
        cls.updater = (ROOT / "updater/main.c").read_text(encoding="utf-8")

    def test_release_is_v08(self):
        self.assertIn('set(WS_SFO_VERSION "00.80")', self.cmake)
        self.assertIn('set(WS_FILE_VERSION "0.8")', self.cmake)

    def test_main_removes_the_closed_updater_after_update(self):
        install_h = (ROOT / "src/install.h").read_text(encoding="utf-8")
        install_c = (ROOT / "src/install.c").read_text(encoding="utf-8")
        self.assertIn("int inst_delete(const char *titleid);", install_h)
        self.assertIn("scePromoterUtilityDeletePkg(titleid)", install_c)
        self.assertIn('inst_delete("WOLF00002")', self.main)

    def test_settings_touch_does_not_scroll_page_behind_modal(self):
        self.assertIn("dragged && viewer < 0 && !vid_open && !settings_open", self.main)

    def test_settings_shows_store_version(self):
        settings = self.main[self.main.index("static void draw_settings") : self.main.index("/* ---------------------------------------------------------------- menu de áudio")]
        self.assertIn('"Wolff\'s Store v" WS_VERSION', settings)

    def test_paw_touch_goes_home_or_animates_when_already_home(self):
        touch = self.main[self.main.index("/* toque: arrastar rola") :]
        self.assertIn("if (view != V_HOME) go(V_HOME, 0);", touch)
        self.assertIn("else {\n                        bark_t0 = now_s;", touch)

    def test_header_uses_equal_columns(self):
        self.assertRegex(self.main, r"hdr_w\[i\]\s*=\s*HDR_COL_W")
        self.assertNotIn("lw > BADGE ? lw : BADGE", self.main)

    def test_both_update_stages_use_paw_and_wolffs_room(self):
        update = self.main[self.main.index("static void draw_update") :]
        self.assertIn("paw_draw(", update)
        self.assertIn('"WOLFF\'S ROOM"', update)
        self.assertIn('"WOLFF\'S ROOM"', self.updater)
        self.assertIn("draw_paw(", self.updater)
        self.assertIn("case 'M': return M", self.updater)

    def test_video_uses_proven_vita_buffer_contract(self):
        self.assertIn("SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_NC_RW", self.video)
        self.assertRegex(self.video, r"numOutputVideoFrameBuffers\s*=\s*5")
        self.assertIn("1024 * 1024", self.video)

    def test_video_has_file_callbacks_and_native_fallback(self):
        for member in ("open", "close", "readOffset", "size"):
            self.assertIn(f"in.fileReplacement.{member}", self.video)
        self.assertIn("memset(&in.fileReplacement, 0", self.video)
        self.assertGreaterEqual(self.video.count("sceAvPlayerAddSource"), 2)

    def test_video_waits_before_repointing_frame_texture(self):
        draw = self.video[self.video.index("void video_draw") :]
        wait = draw.index("vita2d_wait_rendering_done()")
        repoint = draw.index("sceGxmTextureInitLinear")
        self.assertLess(wait, repoint)


if __name__ == "__main__":
    unittest.main()
