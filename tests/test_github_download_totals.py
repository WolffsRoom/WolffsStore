"""The Vita catalog and sidebar carry the GitHub count without false zero."""
import unittest
from pathlib import Path
ROOT=Path(__file__).resolve().parents[1]

class GitHubDownloadTotalTests(unittest.TestCase):
    def test_count_is_present_in_both_catalog_parsers(self):
        src=(ROOT/'src/store.c').read_text('utf8')
        catalog=(ROOT/'src/catalog.c').read_text('utf8')
        header=(ROOT/'src/catalog.h').read_text('utf8')
        generator=(ROOT/'tools/gen_data.py').read_text('utf8')
        self.assertIn('github_downloads_total',src)
        self.assertIn('github_downloads_total',catalog)
        self.assertIn('long long github_downloads_total',header)
        self.assertIn('github_downloads_total',generator)
        self.assertIn('p->github_downloads_total = -1',src)
        self.assertIn('p->github_downloads_total = -1',catalog)

    def test_counter_precedes_details(self):
        main=(ROOT/'src/main.c').read_text('utf8')
        section=main.split('static void build_post(int pi)',1)[1].split('static void build_dls',1)[0]
        self.assertLess(section.index('if (p->github_downloads_total >= 0'),
                        section.index('if (t->ninfo)'))
        self.assertIn('Total de downloads',section)
        self.assertIn('Total downloads',section)
