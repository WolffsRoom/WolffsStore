"""Critical cross-owner GitHub Lite mirror used by Pokémon Emerald on PS Vita."""
import json
from pathlib import Path
import unittest

ROOT = Path(__file__).resolve().parents[1]

class PokemonEmeraldLite(unittest.TestCase):
    def test_hctrrb_is_allowlisted_for_lite_generation(self):
        config = json.loads((ROOT / "repos.json").read_text("utf-8"))
        self.assertIn({"owner":"HctrRB", "repo":"pokeemerald-vita-3ds"},config["external"])

    def test_store_link_mapping_is_generic_for_external_owners(self):
        # GitHub Lite contains static repo pages; the Store maps the source
        # link to the corresponding static /HctrRB/pokeemerald-vita-3ds/ path.
        main = (ROOT.parent / "src/main.c").read_text("utf-8")
        self.assertIn('snprintf(out, cap, "https://wolffsroom.github.io/WolffsStore/%s/%s/%s", owner, repo, section);',main)
        self.assertIn('const char *target = vita_github_url(url, mapped, sizeof(mapped));',main)
        self.assertIn('sceAppMgrLaunchAppByUri(0xFFFFF, uri);',main)
