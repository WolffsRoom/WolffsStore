"""Invariant checks for choosing and restoring the active taiHEN config."""
import unittest
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]

class ActiveTaihenConfigTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.code=(ROOT/"src/touch_plugin.c").read_text("utf-8")

    def test_active_config_preference_and_restoration(self):
        c=self.code
        self.assertIn('fs_exists("ux0:tai/config.txt")',c)
        self.assertIn('? "ux0:tai/config.txt" : DD_CONFIG',c)
        self.assertIn('fs_read(active_cfg,&oldsize)',c)
        self.assertIn('fs_copy(active_cfg,DD_BACKUP "/config.txt.before")',c)
        self.assertIn('fs_write(next_cfg,cfg,(int)strlen(cfg))',c)
        self.assertIn('sceIoRename(next_cfg,active_cfg)',c)
        self.assertIn('fs_copy(next_cfg,active_cfg)',c)
        self.assertIn('fs_read(active_cfg,&result_size)',c)
        self.assertIn('fs_copy(DD_BACKUP "/config.txt.before",active_cfg)',c)
        self.assertIn('sceIoRemove(next_cfg)',c)

    def test_plugin_path_does_not_change_with_active_config(self):
        c=self.code
        self.assertIn('#define DD_PLUGIN "ur0:tai/DDTouchUI.suprx"',c)
        self.assertIn('const char *line="ur0:tai/DDTouchUI.suprx"',c)
        self.assertIn('if(!approved_zip(archive))',c)
        self.assertIn('size_is(path,sizes[i])',c)

if __name__=="__main__":
    unittest.main()
