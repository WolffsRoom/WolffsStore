# GitHub Vita Lite (Wolff's Store)

A lightweight GitHub mirror for the original PS Vita web browser, hosted in the WolffsStore repository through GitHub Pages. It has no client-side JavaScript, external CSS, fonts or scripts.

## Live site (after enabling Pages)

- Home: https://wolffsroom.github.io/WolffsStore/
- DeltaruneVita: https://wolffsroom.github.io/WolffsStore/WolffsRoom/DeltaruneVita/
- WolffsStore: https://wolffsroom.github.io/WolffsStore/WolffsRoom/WolffsStore/

This is a static snapshot, not a server/API. GitHub Pages cannot run Python or serve dynamic endpoints. Only repositories in repos.json are generated.

## One-time Pages activation

1. Open https://github.com/WolffsRoom/WolffsStore/settings/pages
2. In Build and deployment > Source, select GitHub Actions.
3. Open Actions > Vita GitHub Lite - Pages and run Run workflow once (or wait for a scheduled build).
4. After successful deployment, open the links above.

The workflow runs every 12 hours, when these files are updated, and manually. A schedule can be delayed by GitHub. You do not need to keep a PC or server running.

## What is generated

- Repository home pages, README with Markdown formatting, and directory navigation.
- Text file previews for up to 130 small source/docs files per repository; metadata placeholders for the remaining files.
- Releases, asset links, and the 25 most recent open issues.
- Local, resized PNG/JPEG copies of up to 80 README/release images per repository (including SVG-to-PNG conversion), where the host allows downloads.
- Relative internal links, including the GitHub Pages project prefix /WolffsStore/.

Source content is fetched from GitHub on the Actions runner. The Vita only downloads static HTML, CSS and converted images. Some release downloads and links outside mirrored repositories still open GitHub HTTPS and may fail in the Vita browser.

## Add repositories

Edit vita-github/repos.json with an owner and repo, commit and wait for the workflow. Generating every source file for large repos takes time and makes many HTML pages.

## Run locally

With Python 3.11+:

1. pip install -r vita-github/requirements.txt
2. python vita-github/generate.py _site
3. python -m http.server 8000 --directory _site

Open http://YOUR-PC-IP:8000/ on the Vita (same Wi-Fi). An optional GH_TOKEN environment variable increases API limits; never insert tokens in the HTML.

## PS Vita compatibility

Because github.io uses HTTPS, the Vita must support its TLS and certificate chain. If the page never loads even though the HTML is simple, test on a PC and try an updated TLS/certificate solution on the Vita. A TLS failure is independent of HTML compatibility.

VPK downloads are not hosted or proxied by this static Pages site. Do not mirror private repositories or secrets.
