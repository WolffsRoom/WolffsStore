"""Verify safe installer flows in the native Vita implementation."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]

class VitaPackageInstallerTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.jobs = (ROOT / "src/jobs.c").read_text("utf-8")
        cls.inst = (ROOT / "src/install.c").read_text("utf-8")
        cls.main = (ROOT / "src/main.c").read_text("utf-8")
        cls.catalog = (ROOT / "src/catalog.h").read_text("utf-8")
        cls.store = (ROOT / "src/store.c").read_text("utf-8")

    def test_nested_vpk_uses_existing_promoter(self):
        self.assertIn("find_nested_vpk(", self.jobs)
        self.assertIn('strcasecmp(ext, ".zip")', self.jobs)
        self.assertIn('inst_extract_password(path, stage, NULL, 0, j->password', self.jobs)
        self.assertIn("inst_install_vpk(to_install, &j->pr, &j->phase)", self.jobs)
        self.assertIn("sceShellUtilLock", self.jobs)
        self.assertIn("inst_promote(WS_PKG_TMP)", self.inst)

    def test_app_files_is_distinct_from_repatch(self):
        self.assertIn("DL_APP_FILES", self.catalog)
        self.assertIn("JK_APP_FILES", (ROOT / "src/jobs.h").read_text("utf-8"))
        self.assertIn('!strcmp(tp, "app_arquivos") ? DL_APP_FILES', self.store)
        self.assertIn('const char *dst = "ux0:app/PCSF00007"', self.jobs)
        self.assertIn('fs_is_dir(dst)', self.jobs)
        self.assertIn('strcmp(j->tids[0], "PCSF00007")', self.jobs)
        self.assertIn("app_apply_files(payload, dst, backup", self.jobs)

    def test_full_package_validation_precedes_app_mutation(self):
        self.assertIn('archive_read_support_format_rar5(a)', self.inst)
        self.assertIn('archive_read_add_passphrase(a, password)', self.inst)
        self.assertIn('strcasecmp(ext, ".torrent")', self.jobs)
        self.assertIn("!count", self.jobs)
        self.assertIn("Arquivo só tem torrent", self.jobs)
        self.assertIn("app_preflight(payload, dst, backup", self.jobs)
        self.assertLess(self.jobs.index("app_payload_valid(payload, 0, &count)"),
                        self.jobs.index("app_apply_files(payload, dst, backup"))
        self.assertIn('const char *backup = "ux0:data/WolffsStore/backups/PCSF00007"', self.jobs)
        self.assertIn("fs_copy(b, c)", self.jobs)

    def test_livearea_ui_and_open_game(self):
        self.assertIn("atualize a LiveArea pelo FMVita ou File Manager", self.main)
        self.assertIn("refresh LiveArea with FMVita or File Manager", self.main)
        self.assertIn("body_draw(TXT(S_APP_LIVEAREA)", self.main)
        self.assertIn('trans_confirm_app = dl->type == DL_APP_FILES', self.main)
        self.assertIn("trans_confirm_base_missing", self.main)
        self.assertIn("j->kind == JK_APP_FILES ? DL_APP_FILES", self.main)
        self.assertIn('sceAppMgrLaunchAppByUri(0xFFFFF, uri)', self.main)

if __name__ == "__main__":
    unittest.main()
