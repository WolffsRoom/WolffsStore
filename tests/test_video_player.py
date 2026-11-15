from pathlib import Path
import unittest


ROOT = Path(__file__).resolve().parents[1]


class VideoPlayerInvariants(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.source = (ROOT / "src/video.c").read_text(encoding="utf-8")
        cls.catalog = (ROOT / "data/catalog.txt").read_bytes().decode("latin-1")

    def test_vitadb_compatible_native_path(self):
        source = self.source
        self.assertIn("init.basePriority = 0xA0", source)
        self.assertIn("init.numOutputVideoFrameBuffers = VIDEO_BUFFERS", source)
        self.assertIn("init.autoStart = SCE_TRUE", source)
        self.assertLess(source.index("open_decoder(0)"), source.index("open_decoder(1)"))
        self.assertIn("SCE_GXM_TEXTURE_FORMAT_YVU420P2_CSC1", source)

    def test_no_port_collision_with_store_audio(self):
        self.assertNotIn("sceAudioOutOpenPort", self.source)
        self.assertNotIn("SCE_AUDIO_OUT_PORT_TYPE_MAIN", self.source)

    def test_cache_download_handoff(self):
        self.assertIn("VID_DOWNLOAD_READY", self.source)
        self.assertIn("sceKernelWaitThreadEnd(dl_thread", self.source)
        self.assertIn("video: invalid cache, downloading a fresh copy", self.source)
        self.assertIn("sceSysmoduleIsLoaded(SCE_SYSMODULE_AVPLAYER)", self.source)

    def test_vita_optimized_trailers_in_offline_catalog(self):
        for slug in ("pizza", "delta"):
            self.assertIn(f"/posts/{slug}/trailer-vita-v2.mp4", self.catalog)


if __name__ == "__main__":
    unittest.main()
