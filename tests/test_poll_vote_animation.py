"""Static regression for the native voting feedback assets and render path."""
import unittest
from pathlib import Path
from zipfile import ZipFile

ROOT = Path(__file__).resolve().parents[1]

class PollVoteAnimationTests(unittest.TestCase):
    def test_vote_assets_are_separate_and_small(self):
        box = ROOT / "assets/polls/caixa.png"
        paper = ROOT / "assets/polls/papel.png"
        self.assertTrue(box.is_file())
        self.assertTrue(paper.is_file())
        self.assertLess(box.stat().st_size, 100_000)
        self.assertLess(paper.stat().st_size, 100_000)

    def test_animation_does_not_claim_vote_success_or_block_thread(self):
        code = (ROOT / "src/main.c").read_text(encoding="utf8")
        self.assertIn('poll_anim_t0 = now_s;', code)
        self.assertIn('poll_box_page_y = y + 12;', code)
        self.assertIn('CONFIRMAR VOTO', code)
        self.assertNotIn('poll_selected==i?"[X]":"[ ]"', code)
        self.assertIn('cover_tex[pi]', code)
        self.assertIn('600,338', code)
        self.assertIn('poll_local_answered(g_posts[chosen].poll_id)',code)
        self.assertIn('g_posts[i].cat != CAT_REVIEWS) return i;',code)
        self.assertIn('CONFIRMANDO...', code)
        self.assertIn('VOTO PROCESSADO!', code)
        self.assertIn('Obrigado por participar!',code)
        self.assertIn('Resultado sairá em %d dia(s)',code)
        self.assertIn('const int just_confirmed',code)
        self.assertIn('if (just_confirmed && poll_box_tex)',code)
        # The already-voted branch precedes the selectable-options branch.
        poll=code.split('static void build_poll(int pi)',1)[1].split('static void build_post',1)[0]
        receipt=poll.split('} else if(answered) {',1)[1].split('} else {',1)[0]
        self.assertNotIn('A_POLL_CHOOSE',receipt)
        self.assertNotIn('A_POLL_SEND',receipt)
        self.assertIn('utf8_to_latin1(lang==LANG_PT ? "Escolha uma opção:"', code)
        self.assertIn('Erro de conexão HTTPS:', code)
        self.assertIn('top - 126 + bob',code)
        self.assertNotIn('120 * t',code)
        self.assertIn('draw_poll_vote_animation();', code)
        self.assertIn('poll_vote(poll_selected)', code)
        self.assertIn('app0:data/polls/caixa.png', code)
        self.assertIn('app0:data/polls/papel.png', code)
        body = code.split('static void draw_poll_vote_animation(void)', 1)[1].split('static void draw_viewer', 1)[0]
        self.assertNotIn('fs_read(', body)
        self.assertNotIn('sceKernelDelayThread(', body)

    def test_packaging_paths(self):
        cmake = (ROOT / "CMakeLists.txt").read_text(encoding="utf8")
        self.assertIn('assets/polls/caixa.png data/polls/caixa.png', cmake)
        self.assertIn('assets/polls/papel.png data/polls/papel.png', cmake)
        self.assertIn('data/certs/cacert.pem',cmake)
        net=(ROOT/'src/net.c').read_text('utf8')
        self.assertIn('CURLOPT_CAINFO',net)
        self.assertIn('CURLOPT_SSL_VERIFYPEER, 1L',net)
        self.assertTrue((ROOT/'data/certs/cacert.pem').is_file())

if __name__ == "__main__":
    unittest.main()
