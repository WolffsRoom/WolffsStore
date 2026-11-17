"""Regression checks for the rePatch replacement workflow."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]

class TranslationInstallTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.jobs = (ROOT / "src/jobs.c").read_text("utf-8")
        cls.inst = (ROOT / "src/install.c").read_text("utf-8")
        cls.main = (ROOT / "src/main.c").read_text("utf-8")
        cls.catalog = (ROOT / "src/catalog.h").read_text("utf-8")
        cls.store = (ROOT / "src/store.c").read_text("utf-8")

    def test_encrypted_zip_uses_explicit_passphrase(self):
        self.assertIn("archive_read_add_passphrase", self.inst)
        self.assertIn("inst_extract_password(path, stage", self.jobs)
        self.assertIn("j->password", self.jobs)

    def test_atomic_repatch_and_backup_guards(self):
        self.assertIn('"%s_bkp"', self.jobs)
        self.assertIn("fs_exists(dst) && fs_exists(backup)", self.jobs)
        self.assertIn('app_payload_valid(stage, 0, &files)', self.jobs)
        self.assertIn('snprintf(marker, sizeof(marker), "@%s/"', self.jobs)
        self.assertIn("sceIoRename(dst, backup)", self.jobs)
        self.assertIn("sceIoRename(stage, dst)", self.jobs)
        self.assertIn("sceIoRename(backup, dst)", self.jobs)
        self.assertIn("!j->confirm_replace", self.jobs)
        self.assertLess(self.jobs.index("inst_extract_password(path, stage"),
                        self.jobs.index("sceIoRename(dst, backup)"))

    def test_single_download_and_optional_mirrors(self):
        self.assertIn("MAX_MIRRORS", self.catalog)
        self.assertIn('json_object_get(d, "alternativas")', self.store)
        self.assertIn('x->password = S(d, "senha")', self.store)
        self.assertIn("resolve_mediafire(", self.jobs)
        self.assertIn("for (int attempt = 0; attempt <= j->nmirrors; attempt++)", self.jobs)

    def test_repatch_active_config_and_confirmation(self):
        self.assertIn('repatch_from_config("ur0:tai/config.txt")', self.main)
        self.assertIn("if (!*line || *line == '#' ", self.main)
        self.assertIn("fs_exists(line)", self.main)
        self.assertIn("trans_confirm_backup", self.main)
        self.assertIn("close_translation_confirm(1)", self.main)

    def test_installed_translation_can_open_base_game(self):
        self.assertIn('snprintf(game, sizeof(game), "ux0:app/%s/sce_sys/param.sfo"', self.main)
        self.assertIn('snprintf(s->tid, sizeof(s->tid), "%s", d->tids[i])', self.main)
        self.assertIn("sceAppMgrLaunchAppByUri(0xFFFFF, uri)", self.main)

if __name__ == "__main__":
    unittest.main()
