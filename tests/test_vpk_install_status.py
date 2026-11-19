"""Regression checks for stale SFO APP_VER after a successful VPK install."""
import shutil
import subprocess
import tempfile
import unittest
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]

class VpkInstallStatusTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.main=(ROOT/"src/main.c").read_text(encoding="utf-8")
        cls.policy=(ROOT/"src/vpk_version.h").read_text(encoding="utf-8")

    def test_store_records_completed_install(self):
        self.assertIn("reg_set(j->slug, j->dl, kind, j->version, j->name, path);",self.main)
        self.assertIn("ws_vpk_installed_version(",self.main)
        self.assertIn("r && r->kind == DL_VPK ? r->path : NULL",self.main)
        self.assertIn("version_cmp(s->inst_ver, d->version) < 0",self.main)

    def test_policy_validates_titleid_and_sfo_fallback(self):
        self.assertIn("strlen(installed_titleid) != 9",self.policy)
        self.assertIn('"ux0:app/%s"',self.policy)
        self.assertIn("strcmp(recorded_path, expected) != 0",self.policy)
        self.assertIn("version_cmp(recorded_version, sfo) <= 0",self.policy)
        self.assertIn("return recorded_version;",self.policy)

    def test_host_compiled_version_policy(self):
        cc=shutil.which("cc") or shutil.which("gcc") or shutil.which("clang")
        if not cc:
            self.skipTest("Host C compiler unavailable; VitaSDK builds the same header")
        with tempfile.TemporaryDirectory() as tmp:
            exe=Path(tmp)/("policy.exe" if __import__("sys").platform == "win32" else "policy")
            subprocess.run([cc,"-std=c99","-Wall","-Wextra","-Werror",
                            "-I",str(ROOT/"src"),
                            str(ROOT/"tests/test_vpk_version_policy.c"),
                            "-o",str(exe)],check=True)
            proc=subprocess.run([str(exe)],capture_output=True,text=True,check=True)
            self.assertIn("9 VPK install-status assertions passed",proc.stdout)

if __name__=="__main__":
    unittest.main()
