"""Closed Poll uses server-confirmed results and renders fully native pixel bars."""
import json
import re
import unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

class ClosedPollResultsTests(unittest.TestCase):
    def test_native_closed_status_has_priority_over_vita_clock(self):
        main=(ROOT/'src/main.c').read_text(encoding='utf-8')
        self.assertIn('p->poll_closed || poll_local_ended(p->poll_ends_at)', main)
        self.assertIn('"Votação encerrada!"', main)
        self.assertIn('p->poll_has_counts', main)
        self.assertIn('const int *votes=use_live ? ps.votes : p->poll_votes;', main)
        self.assertIn('news_poll_result_chart(t,y)', main)
        self.assertIn('rgba', main.lower())  # native graphics, no HTML
    def test_closed_results_exported_to_bundled_catalog(self):
        text=(ROOT/'data/catalog.txt').read_text(encoding='latin-1')
        poll=text.split('post\tnext-vita-project-poll\n',1)[1].split('\npost\t',1)[0]
        self.assertIn('poll_closed\t1\n', poll)
        counts=re.findall(r'poll_votes_(\d)\t(\d+)',poll)
        self.assertEqual(counts,[('0','16'),('1','1'),('2','1'),('3','1')])
        result=text.split('post\tnext-vita-project-poll-result\n',1)[1].split('\npost\t',1)[0]
        self.assertIn('bars_title.pt\tResultado final',result)
        for text in ('bar.pt\tNew Browser for Vita|84|16 votos - 84,21%',
                     'bar.en\tReddit Vita|5|1 vote - 5.26%'):
            self.assertIn(text,result)
    def test_online_catalog_parser_accepts_official_closed_snapshot(self):
        source=(ROOT/'src/store.c').read_text(encoding='utf-8')
        self.assertIn('json_object_get(poll,"closed")',source)
        self.assertIn('json_object_get(poll,"counts")',source)
        self.assertIn('p->poll_has_counts=1;',source)
    def test_versions_and_id(self):
        cmake=(ROOT/'CMakeLists.txt').read_text(encoding='utf-8')
        self.assertIn('set(WS_TITLEID "WFFS00042")',cmake)
        self.assertIn('set(WS_UPDATER_TITLEID "WFFS00043")',cmake)
        self.assertIn('set(WS_FILE_VERSION "1.1.4")',cmake)
if __name__=='__main__':
    unittest.main()
