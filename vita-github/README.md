# GitHub Vita Lite for PS Vita

A lightweight, fully static GitHub browser bundled in the **WolffsStore** repository. It is built for the original PS Vita web browser using regular HTML and CSS, with no client-side JavaScript.

## Links

- Home: https://wolffsroom.github.io/WolffsStore/
- DeltaruneVita: https://wolffsroom.github.io/WolffsStore/WolffsRoom/DeltaruneVita/
- WolffsStore: https://wolffsroom.github.io/WolffsStore/WolffsRoom/WolffsStore/

## Automatically included repositories

Every GitHub Actions run discovers **all public repositories owned by WolffsRoom** using the public GitHub API. It **never mirrors a private repository**. These five exclusions in repos.json are always skipped:

- ButterscotchVita
- WolffsRoom
- registro_frequencia
- MTH-Vita
- InfinityBladeVita

The two pinned repositories (DeltaruneVita and WolffsStore) are displayed first. Newly public repositories are added on the next build, and repositories that become private disappear from the next generated snapshot. For urgent removal, manually rerun the workflow rather than waiting for its scheduled build.

## GitHub Pages deployment

The GitHub Actions workflow is at .github/workflows/vita-github-pages.yml and publishes from the Actions artifact. If not yet enabled, go to repository Settings > Pages and set **Source: GitHub Actions**. Run the workflow manually from the Actions tab to publish now.

Automated scheduled builds happen every 12 hours. They can be delayed or skipped by GitHub. No PC is required.

## GitHub formatting and media fidelity

- All GitHub `#gh-light-mode-only` images are skipped, and `#gh-dark-mode-only` images are preferred (including `<picture>` sources). The Vita site uses a high-contrast dark theme so white artwork remains visible.
- HTML attributes such as width="48" and height="50" are retained; tables and centered content use old-style HTML and basic CSS suitable for PS Vita.
- GIFs are mirrored as animated GIFs (not static PNGs), with resizing and frame-duration preservation for large files.
- Shields.io badges are rendered as lightweight, large (15px) native-text HTML/CSS labels, with original badge colors and inline fallback styles; badge text is no longer rasterized into unreadable SVG thumbnails. Ordinary SVG illustrations are still converted into PNG.
- Images use local URLs in the generated Pages artifact, improving TLS compatibility versus loading third-party assets directly.
- All content is rendered server-side in the build process using Python. GitHub Pages **cannot** run Python requests or dynamic APIs on each Vita page load.

## Features and limits

- Repository home pages, Markdown README, directory listings, limited text file previews, open issues, and releases.
- To keep builds and Pages sizes within limits, the generator caps the number of mirrored image files, source previews, and per-file HTML pages per repository. It links overflow items to their original GitHub URLs; these original links may not open on the Vita.
- Git trees exceeding GitHub's API tree size limit may be only partially indexed; the site prints a warning.
- Images from some external hosts cannot be downloaded or converted due to network restrictions or unsupported formats.
- GitHub release assets are linked to their original GitHub download URLs, not hosted on GitHub Pages.
- GIF animation may be limited by the PS Vita browser's own capabilities, even though the generated files preserve animation.

## Local testing

With Python 3.11 or newer:

    pip install -r vita-github/requirements.txt
    python vita-github/generate.py _site
    python -m http.server 8000 --directory _site

Open http://YOUR-PC-IP:8000/ in the original Vita browser on the same Wi-Fi network. Optional GH_TOKEN environment variable increases GitHub API rate limits. Never include a token in the site files.
