"""Regression checks for AES ZIP extraction, rePatch modal and status labels."""
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]


class TranslationZipModalTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.jobs = (ROOT / "src/jobs.c").read_text(encoding="utf-8")
        cls.install = (ROOT / "src/install.c").read_text(encoding="utf-8")
        cls.main = (ROOT / "src/main.c").read_text(encoding="utf-8")
        cls.cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf-8")

    def test_translation_password_forced_to_wolffsroom(self):
        self.assertIn('protected_translation ? "wolffsroom"', self.jobs)
        self.assertIn('d->type == DL_TRANS || d->type == DL_APP_FILES', self.jobs)
        self.assertIn('(!strcasecmp(ext, ".zip") || !strcasecmp(ext, ".rar"))', self.jobs)
        self.assertIn('password && *password) ? password : "wolffsroom"', self.install)

    def test_zip_aes_uses_libzip_and_not_libarchive(self):
        self.assertIn("#include <zip.h>", self.install)
        self.assertIn("zip_set_default_password(za, pass)", self.install)
        self.assertIn("zip_fopen_index_encrypted(za, i, 0, pass)", self.install)
        self.assertIn("ZIP_STAT_ENCRYPTION_METHOD", self.install)
        self.assertIn("zip_fread(zf, buf, sizeof(buf))", self.install)
        self.assertIn("zip_stat_index(za, i, 0, &stat)", self.install)
        self.assertIn("return extract_zip_libzip(zip, dst, strip, nstrip, pass, pr)", self.install)
        self.assertIn("  zip\n", self.cmake)
        self.assertNotIn("zip_file_get_encryption_method", self.install)

    def test_zip_prevents_traversal_and_checks_data(self):
        self.assertIn('strstr(name, "..")', self.install)
        self.assertIn("ZIP_OPSYS_UNIX", self.install)
        self.assertIn("written != (zip_int64_t)stat.size", self.install)
        self.assertIn("zip_fclose(zf)", self.install)
        self.assertIn("sceIoRemove(full)", self.install)
        self.assertIn("pr->cancel", self.install)

    def test_modal_texts_are_translated_and_accents_safe(self):
        self.assertIn('"INSTALAR TRADUÇÃO"', self.main)
        self.assertIn('"Essa tradução exige o plugin rePatch instalado!"', self.main)
        self.assertIn('"Plugin detectado no taiHEN."', self.main)
        self.assertIn('"Plugin ausente no taiHen."', self.main)
        self.assertIn("TXT(S_TRANS_DIALOG_TITLE)", self.main)
        self.assertIn("TXT(S_TRANS_REQUIRE)", self.main)
        self.assertIn("TXT(S_TRANS_DETECTED)", self.main)
        self.assertIn("TXT(S_TRANS_MISSING)", self.main)
        self.assertIn("utf8_to_latin1(note_utf8, note", self.main)
        self.assertIn("trans_confirm_plugin ? C_GOLD : C_PROGRESS_LOW", self.main)

    def test_modal_uses_standard_buttons_and_respects_plugin(self):
        self.assertIn("static void draw_translation_action(", self.main)
        self.assertIn("primary ? C_WHITE : C_DARK", self.main)
        self.assertIn("frame_rect(x, y, w, h, 4, border)", self.main)
        self.assertIn("TXT(S_TRANS_CONFIRM)", self.main)
        self.assertIn("TXT(S_TRANS_CANCEL)", self.main)
        self.assertIn("!trans_confirm_app && !trans_confirm_plugin", self.main)
        self.assertIn("t_y0 >= 380 && t_y0 < 428", self.main)

    def test_sync_exclamation_is_at_end(self):
        self.assertNotIn('snprintf(msg, sizeof(msg), "! %s", t->text)', self.main)
        self.assertIn('? "%s" : "%s!"', self.main)
        self.assertIn('"Sincronizando", "Catálogo atualizado"', self.main)
        self.assertIn('toast(TXT(S_SYNCING))', self.main)
        self.assertIn('toast(TXT(S_SYNC_OK))', self.main)


if __name__ == "__main__":
    unittest.main()
