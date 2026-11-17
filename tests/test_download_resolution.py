"""Client download resolver behavior on slow CDN pages and broken mirrors."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]

class DownloadResolutionTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.jobs=(ROOT/"src/jobs.c").read_text("utf-8")
        cls.net=(ROOT/"src/net.c").read_text("utf-8")
        cls.neth=(ROOT/"src/net.h").read_text("utf-8")
        cls.jobs_h=(ROOT/"src/jobs.h").read_text("utf-8")
        cls.main=(ROOT/"src/main.c").read_text("utf-8")

    def test_mediafire_page_timeout_does_not_block_other_mirrors(self):
        self.assertIn("net_fetch_timeout(page_url, &size, 15L)",self.jobs)
        self.assertIn("char *net_fetch_timeout(",self.net)
        self.assertIn("curl_easy_setopt(c, CURLOPT_TIMEOUT, seconds > 0 ? seconds : 60L)",self.net)
        self.assertIn("for (int attempt = 0; attempt <= j->nmirrors; attempt++)",self.jobs)
        self.assertIn("set_state(j, J_RESOLVE);",self.jobs)
        self.assertIn("set_state(j, J_DOWNLOAD);",self.jobs)
        self.assertIn("curl_easy_setopt(c, CURLOPT_LOW_SPEED_TIME, 20L)",self.net)

    def test_connecting_resolving_status_instead_of_zero_kb(self):
        self.assertIn("J_QUEUED, J_RESOLVE, J_DOWNLOAD",self.jobs_h)
        self.assertIn('"Resolvendo link..."',self.main)
        self.assertIn('"Conectando..."',self.main)
        self.assertIn("case J_RESOLVE: return TXT(S_RESOLVING_LINK);",self.main)
        self.assertIn("j->pr.done > 0 ? TXT(S_DOWNLOADING) : TXT(S_CONNECTING)",self.main)
        self.assertIn("if (j->state == J_DOWNLOAD && j->pr.done > 0)",self.main)
        self.assertIn("j->state == J_RESOLVE || (j->state == J_DOWNLOAD && j->pr.done <= 0)",self.main)

if __name__ == "__main__":
    unittest.main()
