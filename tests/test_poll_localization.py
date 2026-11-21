"""Regression checks for localized questions and choices, consistent option indices."""
import unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

class PollLocalizationTests(unittest.TestCase):
    def test_bundled_and_live_catalog_have_both_languages(self):
        gen=(ROOT/'tools/gen_data.py').read_text(encoding='utf8')
        parser=(ROOT/'src/catalog.c').read_text(encoding='utf8')
        live=(ROOT/'src/store.c').read_text(encoding='utf8')
        header=(ROOT/'src/catalog.h').read_text(encoding='utf8')
        for token in ('poll_question.en','poll_option.en'):
            self.assertIn(token,gen)
            self.assertIn(token,parser)
        for token in ('question_pt','question_en','options_pt','options_en'):
            self.assertIn(token,live)
        self.assertIn('poll_question_en',header)
        self.assertIn('poll_options_en',header)
        self.assertIn('p->poll_count++;',live)

    def test_success_is_paper_free_and_centered(self):
        source=(ROOT/'src/main.c').read_text(encoding='utf8')
        success=source.split('} else if(answered) {',1)[1].split('} else {',1)[0]
        self.assertIn('poll_receipt_page_y = y;',success)
        self.assertNotIn('poll_box_tex',success)
        self.assertNotIn('poll_paper_tex',success)
        self.assertIn('C_WHITE,1,3,1',success)
