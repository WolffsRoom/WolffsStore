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

    def test_release_is_v093(self):
        self.assertIn('set(WS_SFO_VERSION "00.93")', self.cmake)
        self.assertIn('set(WS_FILE_VERSION "0.93")', self.cmake)

    def test_third_party_author_badge_is_optional(self):
        catalog=(ROOT / "src/catalog.h").read_text(encoding="utf-8")
        store=(ROOT / "src/store.c").read_text(encoding="utf-8")
        self.assertIn("char *author;", catalog)
        self.assertIn('p->author = S(o, "autor")', store)
        self.assertIn('snprintf(by, sizeof(by), "by: %s", p->author)', self.main)
        self.assertIn("p->author && p->author[0]", self.main)
        self.assertIn('snprintf(by, sizeof(by), "by: %s", g_posts[pi].author)', self.main)

    def test_author_is_only_in_cards_and_details_not_post_header(self):
        area = self.main[self.main.index("static void build_post("):self.main.index("/* barra de rolagem", self.main.index("static void build_post("))]
        header = area[:area.index("rect(24, y, 912, 4, C_LINE);")]
        self.assertNotIn('snprintf(by, sizeof(by), "by: %s"', header)
        card = self.main[self.main.index("static void card("):self.main.index("static float header(")]
        self.assertIn('snprintf(by, sizeof(by), "by: %s", p->author)', card)
        self.assertIn("if (t->ninfo)", area)

    def test_vita_github_lite_redirect_preserves_direct_download_assets(self):
        nav = self.main[self.main.index("static const char *vita_github_url("):self.main.index("/* ---------------------------------------------------------------- avisos (toasts) */")]
        self.assertIn("https://wolffsroom.github.io/WolffsStore/", nav)
        self.assertIn('strstr(path, "/releases/download/")', nav)
        self.assertIn('const char *target = vita_github_url(url, mapped, sizeof(mapped));', nav)

    def test_status_is_inside_hint_footer_not_separate_row(self):
        self.assertIn("static float draw_hints(void)", self.main)
        self.assertIn("static void draw_strip(float hint_right)", self.main)
        self.assertIn("float nh = 36; /* notifications reuse unused right side of hints */", self.main)
        self.assertIn("draw_strip(hint_right)", self.main)
        self.assertIn("int offset = ((int)(now_s * 9.0)) % len;", self.main)
        self.assertNotIn("float nh = 36 + (strip_visible() ? 32 : 0);", self.main)

    def test_launches_updater_and_store_directly(self):
        self.assertIn('sceAppMgrLaunchAppByUri(0x20000, "psgm:play?titleid=WOLF00002")', self.main)
        self.assertIn('sceAppMgrLaunchAppByUri(0x20000, "psgm:play?titleid=" MAIN_TITLE)', self.updater)
        self.assertIn('sceAppMgrLaunchAppByName(0x60000, "WOLF00002", NULL)', self.main)
        self.assertIn('sceAppMgrLaunchAppByName(0x60000, MAIN_TITLE, NULL)', self.updater)
        self.assertNotIn('sceAppMgrLaunchAppByUri(0xFFFFF', self.updater)

    def test_defer_updater_cleanup_on_upgrade_first_launch(self):
        self.assertIn('last_started_version.txt', self.main)
        self.assertIn('if (upgraded_this_boot)', self.main)
        self.assertIn('updater cleanup deferred: first boot of', self.main)
        self.assertIn('cleanup updater on next launch', self.main)
        self.assertIn('updater: relaunch Store ->', self.updater)

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

    def test_updater_uses_the_same_brand_assets_as_the_store(self):
        update = self.main[self.main.index("static void draw_update") :]
        self.assertNotIn("paw_draw(", update)
        self.assertIn('"WOLFF\'S ROOM"', update)
        self.assertIn('"WOLFF\'S ROOM"', self.updater)
        self.assertIn('app0:data/tex/paw4.wrt', self.updater)
        self.assertIn('app0:data/tex/font_title.wrt', self.updater)
        self.assertNotIn("glyph5(", self.updater)
        self.assertIn('updater/data/tex/paw4.wrt', self.cmake)
        self.assertIn('updater/data/tex/font_title.wrt', self.cmake)

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
