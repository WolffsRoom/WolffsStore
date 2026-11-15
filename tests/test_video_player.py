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
        self.assertIn("init.numOutputVideoFrameBuffers = mode == 2 ? 8 : VIDEO_BUFFERS", source)
        self.assertIn("init.autoStart = SCE_TRUE", source)
        self.assertIn("for (int mode = first; mode < 3; ++mode)",source)
        self.assertIn("SCE_GXM_TEXTURE_FORMAT_YVU420P2_CSC1", source)

    def test_video_audio_uses_own_main_output_port(self):
        self.assertIn("SCE_AUDIO_OUT_PORT_TYPE_MAIN", self.source)
        self.assertIn("stop_video_audio()", self.source)
        music=(ROOT/"src/audio.c").read_text(encoding="utf-8")
        self.assertIn("SCE_AUDIO_OUT_PORT_TYPE_BGM", music)

    def test_cache_download_handoff(self):
        self.assertIn("VID_DOWNLOAD_READY", self.source)
        self.assertIn("sceKernelWaitThreadEnd(dl_thread", self.source)
        self.assertIn("video: invalid cache, downloading a fresh copy", self.source)
        self.assertIn("sceSysmoduleIsLoaded(SCE_SYSMODULE_AVPLAYER)", self.source)

    def test_vita_optimized_trailers_in_offline_catalog(self):
        for slug in ("pizza", "delta"):
            self.assertIn(f"/posts/{slug}/trailer-vita-v2.mp4", self.catalog)


    def test_diagnostic_fallback_and_cpu_mode(self):
        self.assertIn('video_diagnostic.txt', self.source)
        self.assertIn('SCE_KERNEL_MEMBLOCK_TYPE_USER_CDRAM_RW',self.source)
        self.assertIn('nv12_to_rgba',self.source)
        self.assertIn('try_fallback_if_stalled',self.source)
        self.assertIn('decoder_errors[3]',self.source)


if __name__ == "__main__":
    unittest.main()
