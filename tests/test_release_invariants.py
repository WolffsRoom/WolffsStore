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

    def test_release_is_v0996(self):
        self.assertIn('set(WS_SFO_VERSION "00.99")', self.cmake)
        self.assertIn('set(WS_FILE_VERSION "0.99.6")', self.cmake)

    def test_featured_update_suffix_follows_app_title(self):
        hero=self.main[self.main.index("static void build_home("):
                       self.main.index("static void build_cat(")]
        self.assertIn('kind == 1 ? " - Release" : kind == 2 ? " - Update"',self.main)
        self.assertIn("feature_title_rows(t->title",hero)
        self.assertIn("feature_title(t->title, ix, iy, iw, tsc",hero)
        self.assertIn("p->news_kind, ip + .08",hero)
        self.assertIn("C_GOLD",self.main)
        self.assertIn("sx + title_width(suffix, 2) > x + width",self.main)
        self.assertNotIn("news_word(",hero)
        self.assertNotIn("badge_x",hero)

    def test_updater_paw_is_smaller_and_higher(self):
        self.assertIn("const float scale = 1.5f",self.updater)
        self.assertIn("draw_paw((960.f - w * scale) / 2.f, 40.f, scale)",self.updater)
        self.assertNotIn("draw_paw(412, 62, 2)",self.updater)

    def test_title_id_migration_keeps_old_bubble_safe(self):
        self.assertIn('set(WS_TITLEID "WFFS00042")', self.cmake)
        self.assertIn('set(WS_UPDATER_TITLEID "WFFS00043")', self.cmake)
        self.assertIn('set(WS_FILE_VERSION "0.99.6")', self.cmake)
        self.assertIn('set(WS_SFO_VERSION "00.99")', self.cmake)
        self.assertIn('#define WS_VERSION "0.99.6"', self.main)
        self.assertIn('#define MAIN_TITLE "WFFS00042"', self.updater)
        self.assertNotIn('set(WS_TITLEID "WOLF00001")',self.cmake)
        self.assertNotIn('#define MAIN_TITLE "WOLF00001"',self.updater)
        self.assertNotIn('inst_delete("WOLF00002")', self.main)
        self.assertNotIn('"psgm:play?titleid=WOLF00002"', self.main)
        self.assertIn('inst_delete("WFFS00043")', self.main)
        self.assertNotIn('inst_delete("WOLF00001")', self.main)
        self.assertIn('strcmp(title, "Wolff\'s Store Updater") == 0', self.main)
        readme=(ROOT/"README.md").read_text(encoding="utf-8")
        self.assertIn("só exclui WOLF00001",readme)
        self.assertIn("APP_VER 00.xx",readme)
        self.assertIn("ux0:data/WolffsStore/",readme)

    def test_music_only_after_fully_drawn_home(self):
        self.assertIn("if (!music_started_after_frame && !first_sync && started &&",self.main)
        self.assertIn("view == V_HOME && now_s - intro_t0 >= T_DONE &&",self.main)
        self.assertIn("upd_job < 0",self.main)
        frame=self.main[self.main.index("vita2d_swap_buffers();",self.main.index("int main(void)")):]
        self.assertLess(frame.index("vita2d_swap_buffers();"),
                        frame.index("audio_start_music_after_first_frame();"))
        self.assertIn("music_screen_ready", (ROOT/"src/audio.c").read_text(encoding="utf-8"))

    def test_legacy_migration_requires_exact_name_id_and_pre1_version(self):
        self.assertIn('#define LEGACY_APP "WOLF00001"',self.updater)
        self.assertIn('legacy_id_migration_v0992.done',self.updater)
        self.assertIn('valid_store_sfo(old_sfo, LEGACY_APP, 1)',self.updater)
        self.assertIn('valid_store_sfo(new_sfo, "WFFS00042", 0)',self.updater)
        self.assertIn('strcmp(title, "Wolff\'s Store")',self.updater)
        self.assertIn('strcmp(id, expected_id)',self.updater)
        self.assertIn('installed_sfo_value(path, "APP_VER"',self.updater)
        self.assertIn('major != 0 || minor > 99',self.updater)
        self.assertIn("scePromoterUtilityDeletePkg(LEGACY_APP)",self.updater)
        self.assertLess(self.updater.index("if (legacy_valid)"),
                        self.updater.index("int r = delete_legacy_store();"))
        post=self.updater[self.updater.index("int main(void) {"):]
        self.assertLess(post.index("if (r < 0) {"),
                        post.index("migrate_legacy_store_after_install();"))
        self.assertLess(post.index("migrate_legacy_store_after_install();"),
                        post.index('updater_cleanup.pending'))
        self.assertNotIn('inst_delete("WOLF00001")',self.main)
        self.assertIn('f = fopen(LEGACY_MIGRATION_MARKER, "wb")',self.updater)

    def test_legacy_updater_ownership_detection_is_retained_without_delete(self):
        self.assertIn('sfo_get("ux0:app/WOLF00002/sce_sys/param.sfo"',self.main)
        self.assertIn('strcmp(title, "Wolff\'s Store Updater") == 0',self.main)
        tick=self.main[self.main.index("static void tick_store("):
                       self.main.index("/* ================================================================ main */")]
        self.assertNotIn('inst_delete("WOLF00002")',tick)
        self.assertNotIn('inst_delete("WFFS00043")',tick)
        self.assertNotIn('inst_delete("WOLF00001")',tick)

    def test_legacy_updater_migration_bridge(self):
        self.assertIn("set(WS_LEGACY_UPDATER_SFO",self.cmake)
        self.assertIn("TITLE_ID=WOLF00002",self.cmake)
        self.assertIn("updater/sce_sys/param.sfo",self.cmake)
        self.assertIn("updater_v2/sce_sys/param.sfo",self.cmake)
        self.assertIn("updater_v2/sce_sys/icon0.png",self.cmake)
        self.assertIn("ws_legacy_updater_sfo",self.cmake)
        self.assertIn('WS_PKG_SELF "/updater_v2/%s"',self.main)
        self.assertIn('WS_PKG_SELF "/updater_v2/sce_sys/%s"',self.main)
        self.assertIn('#define MAIN_TITLE "WFFS00042"',self.updater)

    def test_third_party_author_badge_is_optional(self):
        catalog=(ROOT / "src/catalog.h").read_text(encoding="utf-8")
        store=(ROOT / "src/store.c").read_text(encoding="utf-8")
        self.assertIn("char *author;", catalog)
        self.assertIn('p->author = S(o, "autor")', store)
        self.assertIn('snprintf(by, sizeof(by), "by: %s", p->author)', self.main)
        self.assertIn("p->author && p->author[0]", self.main)
        self.assertIn('snprintf(by, sizeof(by), "by: %s", g_posts[pi].author)', self.main)

    def test_updater_uses_dedicated_livearea(self):
        import struct
        for name, dimensions in (("icon0.png", (128,128)), ("bg0.png", (840,500)),
                                 ("startup.png", (280,158))):
            data=(ROOT / "assets" / "livearea_autoupdater" / name).read_bytes()
            self.assertEqual(data[:8], bytes.fromhex("89504e470d0a1a0a"))
            self.assertEqual(struct.unpack(">II",data[16:24]), dimensions)
            self.assertEqual(data[25],3)
            self.assertIn(f"assets/livearea_autoupdater/{name}",self.cmake)
            partial="updater/sce_sys/icon0.png" if name=="icon0.png" else "updater/sce_sys/livearea/contents/"+name
            self.assertIn(partial,self.cmake)
            self.assertIn("copy_new_updater_file(",self.main)
            self.assertNotIn("app0:"+partial,self.main)
        self.assertIn("updater/sce_sys/livearea/contents/template.xml",self.cmake)
        self.assertIn('fs_mkdir_p(WS_PKG_UPD "/sce_sys/livearea/contents")',self.main)

    def test_updater_livearea_template_refresh_and_binary_integrity(self):
        import xml.etree.ElementTree as ET
        tmpl = ROOT / "assets/livearea_autoupdater/template.xml"
        xml = ET.parse(tmpl).getroot()
        self.assertEqual(xml.attrib.get("content-rev"), "99")
        self.assertEqual(xml.findtext("livearea-background/image"), "bg0.png")
        self.assertEqual(xml.findtext("gate/startup-image"), "startup.png")
        self.assertIn("assets/livearea_autoupdater/template.xml",self.cmake)
        self.assertIn("set(WS_UPDATER_SFO",self.cmake)
        self.assertIn("updater_param_",self.cmake)
        self.assertIn("DEPENDS",self.cmake)
        self.assertIn("static int updater_asset_matches(", self.main)
        self.assertIn("memcmp(a, b, (size_t)a_len)",self.main)
        self.assertIn('updater_asset_matches("icon0.png")',self.main)
        self.assertIn('updater_asset_matches("livearea/contents/bg0.png")',self.main)
        self.assertIn('updater_asset_matches("livearea/contents/startup.png")',self.main)
        self.assertIn('updater_asset_matches("livearea/contents/template.xml")',self.main)

    def test_news_kind_is_optional_and_rendered_as_yellow_pixel_word(self):
        header=(ROOT / "src/catalog.h").read_text(encoding="utf-8")
        store=(ROOT / "src/store.c").read_text(encoding="utf-8")
        self.assertIn("int news_kind;",header)
        self.assertIn('json_object_get(o, "novidade")',store)
        self.assertIn('!strcmp(kind, "Release")',store)
        self.assertIn('!strcmp(kind, "Update")',store)
        self.assertIn('static const char *feature_suffix(int kind)',self.main)
        self.assertIn("C_GOLD",self.main)
        self.assertIn("feature_title_rows",self.main)
        self.assertIn("p->news_kind",self.main)

    def test_self_update_installs_livearea_from_downloaded_new_vpk(self):
        self.assertIn('WS_PKG_SELF "/updater_v2/%s"', self.main)
        self.assertIn('WS_PKG_SELF "/updater_v2/sce_sys/%s"', self.main)
        self.assertIn("static int copy_new_updater_file(", self.main)
        launch=self.main[self.main.index("static void launch_updater("):
                         self.main.index("static void maybe_self_update(")]
        for name in ("icon0.png", "bg0.png", "startup.png", "template.xml"):
            self.assertIn('copy_new_updater_file("sce_sys/',launch)
            self.assertIn(name,launch)
        self.assertIn("updater_asset_matches(", launch)
        self.assertNotIn('fs_copy("app0:updater/',launch)
        self.assertLess(launch.index('copy_new_updater_file("eboot.bin"'),
                        launch.index("inst_promote(WS_PKG_UPD)"))
        jobs=(ROOT/"src/jobs.c").read_text(encoding="utf-8")
        self.assertIn("inst_extract(path, WS_PKG_SELF, NULL, 0, &j->pr)",jobs)

    def test_update_label_only_featured_on_vita_not_cards_or_post_details(self):
        hero=self.main[self.main.index("static void build_home("):
                       self.main.index("static void build_cat(")]
        self.assertIn("feature_title(t->title, ix, iy, iw, tsc",hero)
        card=self.main[self.main.index("static void card("):
                       self.main.index("static float header(")]
        post=self.main[self.main.index("static void build_post("):
                       self.main.index("/* barra de rolagem",self.main.index("static void build_post("))]
        self.assertNotIn("feature_suffix(",card)
        self.assertNotIn("feature_suffix(",post)
        self.assertNotIn("news_word(",self.main)

    def test_home_featured_github_prefers_source_and_mirrors_lite(self):
        home=self.main[self.main.index("static void build_home("):self.main.index("static void build_cat(")]
        self.assertIn("t->srcs[i].b",home)
        self.assertIn('!strstr(url, "/releases/download/")',home)
        self.assertIn("vita_github_url(gh, mapped, sizeof(mapped))",home)
        self.assertIn("A_URL, 0, keep(url)",home)
        self.assertNotIn('A_URL, 0, "https://github.com/WolffsRoom?tab=repositories"',self.main)
        self.assertIn('A_URL, 0, "https://wolffsroom.github.io/WolffsStore/"',self.main)

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

    def test_handoff_queues_only_one_shell_launch_then_exits(self):
        launch = self.main[self.main.index("static void launch_updater("):
                           self.main.index("static void maybe_self_update(")]
        self.assertIn('sceAppMgrLaunchAppByUri(0x20000, "psgm:play?titleid=WFFS00043")',launch)
        self.assertIn("sceKernelExitProcess(0);", launch)
        self.assertNotIn("sceAppMgrLaunchAppByName(",launch)
        self.assertNotIn("sceAppMgrLaunchAppByUri(0xFFFFF",launch)
        updater_launch = self.updater[self.updater.index('/* Wait for SceShell registration.'):]
        self.assertIn('sceAppMgrLaunchAppByUri(0x20000, "psgm:play?titleid=" MAIN_TITLE)',updater_launch)
        self.assertIn("sceKernelExitProcess(0);",updater_launch)
        self.assertNotIn("sceAppMgrLaunchAppByName(",updater_launch)
        self.assertNotIn("sceAppMgrLaunchAppByUri(0xFFFFF",updater_launch)

    def test_updater_livearea_is_freshly_installed(self):
        launch = self.main[self.main.index("static void launch_updater("):
                           self.main.index("static void maybe_self_update(")]
        self.assertLess(launch.index('inst_delete("WFFS00043")'),
                        launch.index("inst_promote(WS_PKG_UPD)"))
        for name in ("icon0.png","bg0.png","startup.png","template.xml"):
            self.assertIn('fs_exists("ux0:app/WFFS00043/sce_sys/',launch)
            self.assertIn(name,launch)
        self.assertIn('updater: installed bubble missing custom LiveArea files',launch)

    def test_cleanup_is_disabled_while_updated_store_is_running(self):
        tick=self.main[self.main.index("static void tick_store("):
                       self.main.index("/* ================================================================ main */")]
        self.assertIn("helper deletion DISABLED while Store runs",tick)
        self.assertNotIn('inst_delete("WFFS00043")',tick)
        self.assertNotIn('inst_delete("WOLF00002")',tick)
        self.assertNotIn("fs_rm_rf(WS_DATA \"/updater_cleanup.pending\")",tick)
        self.assertIn('updater_cleanup.pending',self.updater)
        # The INSTALLER may still replace an outdated helper before leaving.
        launch=self.main[self.main.index("static void launch_updater("):
                         self.main.index("static void maybe_self_update(")]
        self.assertIn('inst_delete("WFFS00043")',launch)

    def test_settings_has_compact_nested_pages(self):
        self.assertIn("enum { SETTINGS_ROOT, SETTINGS_AUDIO, SETTINGS_VISUAL, SETTINGS_GENERAL }",self.main)
        self.assertIn("148 + settings_rows() * *rh",self.main)
        self.assertIn("settings_page = SETTINGS_ROOT",self.main)
        self.assertIn("settings_page = settings_row == 0 ? SETTINGS_AUDIO",self.main)
        self.assertIn("if (down & SCE_CTRL_CIRCLE) settings_back()",self.main)
        self.assertIn("settings_geometry(&x, &y, &w, &h, &rh)",self.main)
        self.assertIn("row < settings_rows()",self.main)
        self.assertIn("SET_OST",self.main)
        self.assertIn("S_SET_CAT_AUDIO",self.main)
        self.assertIn("S_SET_CAT_VISUAL",self.main)
        self.assertIn("S_SET_CAT_GENERAL",self.main)
        self.assertNotIn("96 + SET_COUNT * rh + 100",self.main)

    def test_video_root_diagnostics_and_last_resort_player(self):
        self.assertIn('WS_DATA "/video_diagnostic.txt"',self.video)
        self.assertIn("static void video_trace_new_session",self.video)
        self.assertIn("try_decoder_from(0)",self.video)
        self.assertIn("mode == 2 ? cpu_frame_alloc : gpu_alloc",self.video)
        self.assertIn("SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW",self.video)
        self.assertIn("nv12_to_rgba",self.video)
        self.assertIn("sceAudioOutOpenPort",self.video)
        self.assertIn("try_fallback_if_stalled",self.video)
        self.assertIn("decoder_errors[3]",self.video)

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

    def test_vitadb_native_video_buffer_and_decoder_contract(self):
        self.assertIn("SCE_KERNEL_MEMBLOCK_TYPE_USER_MAIN_PHYCONT_NC_RW",self.video)
        self.assertIn("VIDEO_BUFFERS 5",self.video)
        self.assertIn("init.basePriority = 0xA0",self.video)
        self.assertIn("init.numOutputVideoFrameBuffers = mode == 2 ? 8 : VIDEO_BUFFERS",self.video)
        self.assertIn("SCE_GXM_TEXTURE_FORMAT_YVU420P2_CSC1",self.video)
        self.assertIn("SCE_AUDIO_OUT_PORT_TYPE_MAIN",self.video)

    def test_video_has_file_callbacks_and_native_fallback(self):
        for member in ("open", "close", "readOffset", "size"):
            self.assertIn(f"init.fileReplacement.{member}", self.video)
        self.assertIn("if (mode != 0)", self.video)
        self.assertIn("for (int mode = first; mode < 3; ++mode)", self.video)

    def test_native_video_frame_ownership_and_cache_recovery(self):
        self.assertIn("sceGxmTextureInitLinear",self.video)
        self.assertIn("vita2d_wait_rendering_done()",self.video)
        self.assertIn("video: invalid cache, downloading a fresh copy",self.video)
        self.assertIn("AddSource=0x%08X",self.video)
        self.assertIn("video: GPU memblock allocation failed",self.video)
        self.assertIn('init.basePriority = 0xA0',self.video)
        self.assertIn("First GXM frame",self.video)
        catalog=(ROOT/"data/catalog.txt").read_bytes().decode("latin-1")
        for slug in ("pizza","delta"):
            self.assertIn(f"/posts/{slug}/trailer-vita-v2.mp4",catalog)

    def test_audio_daypart_and_fixed_ost_options(self):
        aud=(ROOT/"src/audio.c").read_text(encoding="utf-8")
        header=(ROOT/"src/audio.h").read_text(encoding="utf-8")
        self.assertIn("audio_ost_refresh_clock()", self.main)
        self.assertIn("audio_ost_mode()",self.main)
        self.assertIn("ost=%d",self.main)
        self.assertIn('if (!strncmp(l, "ost=", 4))',self.main)
        self.assertIn("SET_OST",self.main)
        self.assertIn("S_AUTO_OST",self.main)
        self.assertIn("Background OST",self.main)
        self.assertIn("Morning Pixels",self.main)
        self.assertIn("Afternoon Arcade",self.main)
        self.assertIn("Night Shift",self.main)
        self.assertIn("static void switch_track(int next)",aud)
        self.assertIn("OST_FADE_SAMPLES",aud)
        self.assertIn("music_mix * ost_gain_step()",aud)
        self.assertIn("ost_hour >= 6 && ost_hour < 12",aud)
        self.assertIn("ost_hour >= 12 && ost_hour < 18",aud)
        for name,frames in (
            ("morning", 921600),
            ("afternoon", 768000),
            ("night", 822857),
        ):
            pcm=ROOT/"data/music"/(name+".pcm")
            self.assertTrue(pcm.exists(),name)
            self.assertEqual(pcm.stat().st_size,frames*2)
            self.assertIn("data/music/"+name+".pcm",self.cmake)
            self.assertIn("data/music/"+name+".pcm",aud)
        self.assertIn("void audio_set_ost_mode(int mode)",header)
        self.assertIn("int audio_ost_mode(void)",header)
        self.assertIn("void audio_ost_refresh_clock(void)",header)



if __name__ == "__main__":
    unittest.main()
