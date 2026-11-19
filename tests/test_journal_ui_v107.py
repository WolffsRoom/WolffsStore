import unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]
class JournalV107Tests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.main=(ROOT/"src/main.c").read_text("utf-8")
  cls.audio=(ROOT/"src/audio.c").read_text("utf-8")
  cls.catalog=(ROOT/"src/catalog.h").read_text("utf-8")
  cls.store=(ROOT/"src/store.c").read_text("utf-8")
  cls.poll=(ROOT/"src/polls.c").read_text("utf-8")
  cls.net=(ROOT/"src/net.c").read_text("utf-8")
  cls.updater=(ROOT/"updater/main.c").read_text("utf-8")
  cls.build=(ROOT/"CMakeLists.txt").read_text("utf-8")
 def test_categorization_is_mapped_in_both_parsers(self):
  self.assertIn("SUB_APP, SUB_TOOL, SUB_MOD, SUB_PLUGIN",self.catalog)
  self.assertIn("SUB_NEWS, SUB_REVIEW, SUB_POLL",self.catalog)
  self.assertIn('!strcmp(c, "journal")',self.store)
  self.assertIn('json_object_get(o, "subcategoria")',self.store)
  for word in ['"Apps/Mods"','"Journal"','"Enquetes"']:
   self.assertIn(word,self.main)
  self.assertIn("static const int ports_groups[4] = {GRP_DONE, GRP_DOING",self.main)
  self.assertIn("static void build_news(",self.main)
 def test_sfx_and_animation(self):
  for tag in ["download","cancel","complete","notice","updates"]:
   self.assertIn('!strcmp(k, "'+tag+'")',self.audio)
  self.assertIn('sfx("complete")',self.main)
  self.assertIn('sfx("download")',self.main)
  self.assertIn('sfx("cancel")',self.main)
  self.assertIn('sfx("notice")',self.main)
  self.assertIn("now_s * 105.f",self.main)
  self.assertIn("float stagger",self.main)
  self.assertIn("sinf(f * .07f)",self.updater)
  sync=self.main[self.main.index("static void draw_first_sync(void)"):
                 self.main.index("static void draw_title",self.main.index("static void draw_first_sync(void)"))] if "static void draw_title" in self.main[self.main.index("static void draw_first_sync(void)")+1:] else self.main[self.main.index("static void draw_first_sync(void)"):]
  self.assertNotIn("paw_draw(",sync.split("}\n",1)[0])
 def test_release_notes_only_after_intro_and_once(self):
  self.assertIn("release_notes_seen.txt",self.main)
  self.assertIn("update_notes_pending",self.main)
  self.assertIn("update_notes_visible",self.main)
  self.assertIn("now_s - intro_t0 >= T_DONE",self.main)
  self.assertIn("draw_update_notes();",self.main)
  self.assertIn("static void close_update_notes",self.main)
 def test_polls_fail_closed_and_device_pseudonym(self):
  self.assertIn("SceKernelOpenPsId",self.poll)
  self.assertIn("SHA256(",self.poll)
  self.assertIn('if (!poll_api_ready() || running) return 0;',self.poll)
  self.assertIn("poll_local_answered",self.poll)
  self.assertIn("poll_vote(poll_selected)",self.main)
  self.assertIn("build_poll(",self.main)
  self.assertIn("CURLOPT_SSL_VERIFYPEER, 1L",self.net)
  self.assertIn("CURLOPT_SSL_VERIFYHOST, 2L",self.net)
  self.assertIn("CURLOPT_FOLLOWLOCATION, 0L",self.net)
  self.assertIn("  src/polls.c",self.build)
if __name__=="__main__":
 unittest.main()
