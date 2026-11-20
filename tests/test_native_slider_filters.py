"""Regression checks for the native PS Vita carousel + category section filters."""
import re
import unittest
from pathlib import Path

ROOT=Path(__file__).resolve().parents[1]

class NativeCarouselAndFiltersTests(unittest.TestCase):
 @classmethod
 def setUpClass(cls):
  cls.source=(ROOT/"src/main.c").read_text("utf-8")

 def test_five_card_visual_order_and_twelve_browseable_news(self):
  s=self.source
  self.assertIn("#define DECK_LIMIT 12",s)
  for line in ("deck_order[0] = fresh[3]","deck_order[1] = fresh[2]",
               "deck_order[2] = fresh[0]","deck_order[3] = fresh[1]"):
   self.assertIn(line,s)
  self.assertIn("const int visual[5] = {-2, 2, -1, 1, 0}",s)
  order=[4,3,1,2,5]+list(range(6,13))
  center=2
  self.assertEqual([order[(center+k)%len(order)] for k in (-2,-1,0,1,2)],
                   [4,3,1,2,5])
  center=(center+1)%len(order)
  self.assertEqual(order[center],2)
  center=(center-1)%len(order)
  self.assertEqual(order[center],1)

 def test_deck_uses_native_rendering_and_input(self):
  s=self.source
  self.assertIn("OP_DECK_CARD",s)
  self.assertIn("draw_deck_card(o, x, y, al, f)",s)
  self.assertIn("int it = item(bx, by, bw, bh, A_DECK_CARD, di, NULL)",s)
  self.assertIn('button("<<", 340, y, 80, 0, A_DECK_ARROW, -1',s)
  self.assertIn('button(">>", 540, y, 80, 0, A_DECK_ARROW, 1',s)
  self.assertIn("case A_DECK_CARD:",s)
  self.assertIn("case A_DECK_ARROW:",s)
  self.assertIn("items[focus].act == A_DECK_CARD || items[focus].act == A_DECK_ARROW",s)
  self.assertIn("deck_direction * 20.f * (1.f - moving / .24f)",s)
  self.assertIn("focus = find_item(A_DECK_CARD, deck_pos)",s)
  self.assertIn("col_alpha(C_WHITE, a)",s)  # dimmed edge artwork
  self.assertIn("int slot = o->bold - 2",s)
  self.assertIn("slot == 0 && f",s)
  self.assertIn("slot == 0 ? 10.f : (abs(slot) == 1 ? 8.f : 5.f)",s)

 def test_slides_fit_960x544_without_side_clipping(self):
  slots={-2:(62,188),-1:(208,248),0:(332,296),1:(504,248),2:(710,188)}
  for slot,(x,width) in slots.items():
   with self.subTest(slot=slot):
    # 20 px horizontal transition + 6 px drop shadow
    self.assertGreaterEqual(x-20,24)
    self.assertLessEqual(x+width+20+6,936)
  s=self.source
  for value in ("bx = 62; by = y + 56; bw = 188; bh = 222",
                "bx = 710; by = y + 56; bw = 188; bh = 222",
                "bx = 332; by = y + 10; bw = 296; bh = 306"):
   self.assertIn(value,s)

 def test_all_categories_get_all_and_individual_filters(self):
  s=self.source
  self.assertIn("static int cat_filter[CAT_COUNT] = {-1, -1, -1, -1};",s)
  for value in ("GRP_DONE, GRP_DOING, GRP_NEXT, GRP_OTHERS",
                "SUB_APP, SUB_TOOL, SUB_MOD, SUB_PLUGIN",
                "SUB_NEWS, SUB_REVIEW, SUB_POLL"):
   self.assertIn(value,s)
  self.assertIn('lang == LANG_PT ? "Todos" : "All"',s)
  self.assertIn('lang == LANG_PT ? "VER SOMENTE:" : "SHOW ONLY:"',s)
  self.assertIn("cat_filter[c] >= 0 && cat_filter[c] != section",s)
  self.assertIn("case A_CAT_FILTER:",s)
  self.assertIn("cat_filter[view_cat] = selected",s)
  self.assertIn("focus = find_item(A_CAT_FILTER, selected)",s)
  self.assertIn("button(TXT(S_BACK_HOME)",s)
  self.assertIn("if (v == V_CAT) {",s)

 def test_translated_heading_and_caption_are_gated_until_intro(self):
  s=self.source
  for text in ("Check out the latest updates...", "Confira as últimas novidades...",
               "Explore other completed and in-development projects...",
               "Explore outros trabalhos finalizados ou em desenvolvimento..."):
   self.assertIn(text,s)
  self.assertNotIn("Check out the latest three updates...",s)
  self.assertNotIn("Confira as últimas três novidades...",s)
  self.assertIn("home_pop(T_NAV + .30, .90)",s)

 def test_updater_cleanup_still_runs_before_home(self):
  s=self.source
  self.assertIn("updater_cleanup_at_startup();",s)
  self.assertLess(s.index("updater_cleanup_at_startup();"),s.index("build_home();",s.index("int main(void) {")))

if __name__=="__main__":
 unittest.main()
