#!/usr/bin/env python3
"""Build a JavaScript-free, PS Vita-friendly GitHub Pages mirror."""
import hashlib
import html
import io
import json
import os
import posixpath
import re
import sys
from collections import defaultdict
from math import ceil
from pathlib import Path
from urllib.parse import quote, unquote, urlparse
from xml.etree import ElementTree

import cairosvg
import markdown
import requests
from bs4 import BeautifulSoup
from PIL import Image, ImageOps, ImageSequence

HERE = Path(__file__).resolve().parent
OUT = Path(sys.argv[1] if len(sys.argv) > 1 else "_site").resolve()
OUT.mkdir(parents=True, exist_ok=True)
CONFIG = json.loads((HERE / "repos.json").read_text(encoding="utf-8"))
REPOS = []  # Filled exclusively from the public GitHub API, with explicit exclusions.
SESSION = requests.Session()
SESSION.headers.update({"User-Agent": "WolffsStore-Vita-GitHub-Lite/1.0", "Accept": "application/vnd.github+json"})
if os.getenv("GH_TOKEN"):
    SESSION.headers["Authorization"] = "Bearer " + os.environ["GH_TOKEN"]
IMAGES = {}
IMAGE_COUNTS = defaultdict(int)
ALLOWED_IMG = {"github.com", "raw.githubusercontent.com", "img.shields.io", "avatars.githubusercontent.com", "user-images.githubusercontent.com", "user-attachments.githubusercontent.com", "deltarune.com", "www.deltarune.com", "media.giphy.com", "media.tenor.com", "i.imgur.com", "cdn.discordapp.com"}
TEXT_EXT = {".md", ".markdown", ".txt", ".json", ".yml", ".yaml", ".c", ".h", ".cpp", ".hpp", ".py", ".sh", ".ini", ".toml", ".xml", ".cmake", ".css", ".html", ".js", ".ts"}
MAX_TEXT_FILES = 25  # Keep building all repositories reasonably fast.
MAX_FILE_PAGES = 1600  # Large source trees link unmirrored files to their originals.
MAX_DIRECTORY_READMES = 12
MAX_IMAGE_COUNT = 75  # Per repository; DeltaruneVita receives an increased allowance.
MAX_GIF_FRAMES = 100


def get_json(path):
    url = "https://api.github.com/" + path.lstrip("/")
    r = SESSION.get(url, timeout=35)
    r.raise_for_status()
    return r.json()


def public_repositories():
    """Only public repos of the configured owner; NEVER read private ones."""
    owner = CONFIG["owner"]
    ignored = {name.lower() for name in CONFIG["exclude"]}
    found = []
    page = 1
    while True:
        data = get_json("users/" + quote(owner, safe="") + "/repos?type=owner&per_page=100&page=" + str(page))
        for record in data:
            name = record.get("name", "")
            if record.get("private", True) or record.get("visibility", "public") != "public":
                continue
            if record.get("owner", {}).get("login", "").lower() != owner.lower():
                continue
            if name.lower() in ignored:
                continue
            found.append((owner, name))
        if len(data) < 100:
            break
        page += 1
        if page > 20:
            raise RuntimeError("Repository listing pagination limit reached")
    pins = {name.lower(): i for i, name in enumerate(CONFIG.get("pinned", []))}
    found.sort(key=lambda item: (pins.get(item[1].lower(), len(pins)), item[1].lower()))
    if not found:
        raise RuntimeError("No public repositories found")
    return found


def raw_url(owner, repo, branch, path):
    return "https://raw.githubusercontent.com/" + "/".join(quote(p, safe="") for p in (owner, repo, branch)) + "/" + "/".join(quote(p, safe="") for p in path.split("/"))


def raw_text(owner, repo, branch, path):
    r = SESSION.get(raw_url(owner, repo, branch, path), timeout=30)
    r.raise_for_status()
    return r.content.decode("utf-8-sig", errors="replace")


def repo_page(owner, repo):
    return OUT / owner / repo / "index.html"


def tree_page(owner, repo, directory):
    return OUT / owner / repo / "tree" / directory / "index.html"


def file_page(owner, repo, path):
    return OUT / owner / repo / "blob" / path / "index.html"


def section_page(owner, repo, section):
    return OUT / owner / repo / section / "index.html"


def link(from_page, to_page, suffix=""):
    relative = os.path.relpath(to_page.parent, from_page.parent).replace(os.sep, "/")
    return quote(relative, safe="/.") + ("" if relative == "." else "/") + suffix


def asset_link(from_page, asset):
    relative = os.path.relpath(asset, from_page.parent).replace(os.sep, "/")
    return quote(relative, safe="/.")


def esc(value):
    return html.escape(str(value), quote=True)


def render_page(path, title, content, nav=None):
    path.parent.mkdir(parents=True, exist_ok=True)
    home = link(path, OUT / "index.html")
    css = asset_link(path, OUT / "style.css")
    nav_html = ""
    if nav:
        owner, repo = nav
        base = repo_page(owner, repo)
        nav_html = '<div class="tabs"><a href="' + link(path, base) + '">Code / README</a> | <a href="' + link(path, section_page(owner, repo, "releases")) + '">Releases</a> | <a href="' + link(path, section_page(owner, repo, "issues")) + '">Issues</a></div>'
    page = '<!DOCTYPE html><html lang="en"><head><meta charset="utf-8"><meta name="viewport" content="width=device-width,initial-scale=1"><title>' + esc(title) + ' - GitHub Vita Lite</title><link rel="stylesheet" href="' + css + '"></head><body>'
    page += '<header><a href="' + home + '"><strong>GitHub Vita Lite</strong></a><span> - Wolff&#39;s Store</span></header>'
    page += '<main>' + nav_html + content + '</main><footer>Lightweight pages built by GitHub Actions. No JavaScript. Content from public GitHub repositories.</footer></body></html>'
    path.write_text(page, encoding="utf-8")


def safe_image_source(src, owner, repo, branch, context):
    src = html.unescape(src.strip())
    if src.startswith("//"):
        src = "https:" + src
    if not src.startswith(("https://", "http://", "data:")):
        clean = src.split("?", 1)[0].split("#", 1)[0]
        target = posixpath.normpath(posixpath.join(context, unquote(clean)))
        if target.startswith("../") or target == "..":
            return ""
        src = raw_url(owner, repo, branch, target)
    parsed = urlparse(src)
    host = (parsed.hostname or "").lower()
    if parsed.scheme != "https" or (host not in ALLOWED_IMG and not host.endswith(".githubusercontent.com")):
        return ""
    return src


def mirror_image(src, owner, repo, branch, context, page):
    url = safe_image_source(src, owner, repo, branch, context)
    if not url:
        return ""
    if url in IMAGES:
        return asset_link(page, IMAGES[url]) if IMAGES[url] else ""
    key = (owner, repo)
    allowance = 135 if repo.lower() == "deltarunevita" else MAX_IMAGE_COUNT
    is_gif_url = urlparse(url).path.lower().endswith(".gif")
    if IMAGE_COUNTS[key] >= allowance and not is_gif_url:
        return ""
    IMAGE_COUNTS[key] += 1
    try:
        with SESSION.get(url, stream=True, timeout=35, headers={"Accept": "image/*"}) as r:
            r.raise_for_status()
            raw = bytearray()
            for chunk in r.iter_content(65536):
                raw.extend(chunk)
                if len(raw) > 22_000_000:
                    raise ValueError("image exceeds 22 MB input limit")
            source = bytes(raw)
            kind = r.headers.get("Content-Type", "").split(";")[0].lower()
        if "svg" in kind or source.lstrip().startswith(b"<svg") or urlparse(url).path.lower().endswith(".svg"):
            svg_root = ElementTree.fromstring(source)
            view_box = svg_root.attrib.get("viewBox", "")
            try:
                native_width = float(re.split(r"[ ,]+", view_box.strip())[2]) if view_box else float(re.match(r"[0-9.]+", svg_root.attrib.get("width", "900")).group())
            except (IndexError, ValueError, AttributeError):
                native_width = 900
            render_width = min(900, max(24, int(native_width)))
            source = cairosvg.svg2png(bytestring=source, output_width=render_width)
        digest = hashlib.sha256(url.encode()).hexdigest()[:22]
        with Image.open(io.BytesIO(source)) as im:
            animated = (getattr(im, "n_frames", 1) > 1 and (im.format == "GIF" or "gif" in kind or is_gif_url))
            if animated:
                dest = OUT / "_img" / (digest + ".gif")
                dest.parent.mkdir(parents=True, exist_ok=True)
                # Keep source GIF intact if already small. Otherwise resize AND keep movement.
                if len(source) <= 2_500_000 and im.width <= 800:
                    dest.write_bytes(source)
                else:
                    stride = max(1, ceil(im.n_frames / MAX_GIF_FRAMES))
                    frames, durations = [], []
                    total_ms = 0
                    for n in range(im.n_frames):
                        im.seek(n)
                        total_ms += max(20, int(im.info.get("duration") or 100))
                        if n % stride != 0 and n != im.n_frames - 1:
                            continue
                        frame = im.convert("RGBA")
                        frame.thumbnail((640, 640), Image.Resampling.LANCZOS)
                        frames.append(frame.convert("RGB").quantize(colors=128))
                        durations.append(max(20, total_ms))
                        total_ms = 0
                    if not frames:
                        raise ValueError("empty GIF")
                    frames[0].save(dest, format="GIF", save_all=True, append_images=frames[1:],
                                   duration=durations, loop=im.info.get("loop", 0),
                                   disposal=2, optimize=True)
            else:
                im = ImageOps.exif_transpose(im)
                im.thumbnail((900, 1800), Image.Resampling.LANCZOS)
                im.load()
                has_alpha = im.mode in ("RGBA", "LA") or (im.mode == "P" and "transparency" in im.info)
                is_photo = not has_alpha and im.width * im.height > 150_000
                ext = ".jpg" if is_photo else ".png"
                dest = OUT / "_img" / (digest + ext)
                dest.parent.mkdir(parents=True, exist_ok=True)
                if is_photo:
                    im.convert("RGB").save(dest, "JPEG", quality=82, optimize=True)
                else:
                    im.convert("RGBA" if has_alpha else "RGB").save(dest, "PNG", optimize=True)
        IMAGES[url] = dest
        return asset_link(page, dest)
    except Exception as exc:
        print("Image skipped:", url[:130], type(exc).__name__, str(exc)[:130], flush=True)
        IMAGES[url] = None
        return ""



def github_link(href, owner, repo, branch, current_dir, page, entries):
    href = html.unescape(href).strip()
    if not href or href.startswith("#"):
        return href
    if href.lower().startswith(("javascript:", "data:", "file:")):
        return ""
    parsed = urlparse(href)
    if parsed.scheme in ("http", "https"):
        if parsed.netloc.lower() != "github.com":
            return href
        bits = unquote(parsed.path.strip("/")).split("/")
        if len(bits) < 2 or (bits[0], bits[1]) not in REPOS:
            return href
        dst_owner, dst_repo = bits[:2]
        rest = bits[2:]
        if not rest:
            dest = repo_page(dst_owner, dst_repo)
        elif rest[0] == "releases":
            dest = section_page(dst_owner, dst_repo, "releases")
        elif rest[0] == "issues":
            dest = section_page(dst_owner, dst_repo, "issues")
        elif len(rest) >= 3 and rest[0] in ("tree", "blob"):
            newpath = "/".join(rest[2:])
            dest = tree_page(dst_owner, dst_repo, newpath) if rest[0] == "tree" else file_page(dst_owner, dst_repo, newpath)
        else:
            return href
        return link(page, dest) + ("#" + quote(parsed.fragment) if parsed.fragment else "")
    if parsed.scheme or href.startswith("//"):
        return ""
    target = posixpath.normpath(posixpath.join(current_dir, unquote(parsed.path)))
    if target == ".":
        dest = repo_page(owner, repo)
    elif target.startswith("../") or target == "..":
        return ""
    elif target in entries and entries[target].get("type") == "tree":
        dest = tree_page(owner, repo, target)
    else:
        dest = file_page(owner, repo, target)
    return link(page, dest) + ("#" + quote(parsed.fragment) if parsed.fragment else "")


def markdown_html(text, owner, repo, branch, current_dir, page, entries):
    html_text = markdown.markdown(text or "", extensions=["extra", "sane_lists", "toc"], output_format="html")
    soup = BeautifulSoup(html_text, "html.parser")
    for tag in list(soup.find_all(["script", "style", "iframe", "form", "object", "embed", "video", "audio", "svg", "source"])):
        tag.decompose()
    for picture in list(soup.find_all("picture")):
        picture.unwrap()
    for tag in soup.find_all(True):
        for attribute in list(tag.attrs):
            if attribute.lower().startswith("on") or attribute.lower() not in {"href", "src", "alt", "width", "height", "title", "id", "class", "align", "colspan", "rowspan"}:
                del tag.attrs[attribute]
        if tag.name == "img":
            original = tag.get("src", "")
            converted = mirror_image(original, owner, repo, branch, current_dir, page)
            if converted:
                tag["src"] = converted
                # Keep explicit GitHub README size constraints such as width="48" or height="50".
                for dimension in ("width", "height"):
                    value = str(tag.get(dimension, "")).strip()
                    if value and not re.fullmatch(r"[1-9][0-9]{0,3}(?:px)?", value):
                        tag.attrs.pop(dimension, None)
            else:
                tag.replace_with(soup.new_string("[image: " + str(tag.get("alt", "not available")) + "]"))
        if tag.name == "a":
            original = tag.get("href", "")
            fixed = github_link(original, owner, repo, branch, current_dir, page, entries)
            if fixed:
                tag["href"] = fixed
            else:
                tag.unwrap()
        if tag.name in ("div", "p", "table") and tag.get("align", "").lower() == "center":
            tag["class"] = list(tag.get("class", [])) + ["align-center"]
    return str(soup)


def breadcrumbs(owner, repo, directory, page):
    parts = directory.split("/") if directory else []
    output = ['<a href="' + link(page, repo_page(owner, repo)) + '">' + esc(owner + "/" + repo) + "</a>"]
    for idx, part in enumerate(parts):
        target = "/".join(parts[:idx + 1])
        output.append('<a href="' + link(page, tree_page(owner, repo, target)) + '">' + esc(part) + "</a>")
    return '<div class="crumb">' + " / ".join(output) + "</div>"


def make_repo(owner, repo):
    print("Building", owner + "/" + repo, flush=True)
    info = get_json("repos/" + owner + "/" + repo)
    branch = info["default_branch"]
    try:
        tree = get_json("repos/" + owner + "/" + repo + "/git/trees/" + quote(branch, safe="") + "?recursive=1")
    except requests.HTTPError as exc:
        if exc.response is None or exc.response.status_code not in (404, 409, 422):
            raise
        tree = {"tree": []}  # Empty repository: still publish its landing page.
    if tree.get("truncated"):
        print("WARNING: partial Git tree for", repo, flush=True)
    entries = {x["path"]: x for x in tree["tree"]}
    children = defaultdict(list)
    dirs = {""}
    files = []
    for item in tree["tree"]:
        path = item["path"]
        parent = posixpath.dirname(path)
        children[parent].append(item)
        if item["type"] == "tree":
            dirs.add(path)
        elif item["type"] == "blob":
            files.append(item)
    files.sort(key=lambda x: (0 if x["path"].lower().endswith(".md") else 1 if x["path"].lower().endswith(".txt") else 2, x["path"].count("/"), x["path"].lower()))
    selected = {x["path"] for x in files if Path(x["path"]).suffix.lower() in TEXT_EXT and x.get("size", 0) <= 100000 and x.get("size", 0) > 0}
    selected = set([x["path"] for x in files if x["path"] in selected][:MAX_TEXT_FILES])
    published_file_pages = {x["path"] for x in files[:MAX_FILE_PAGES]}
    readmes_built = 0
    text_cache = {}

    def content(path):
        if path not in text_cache:
            text_cache[path] = raw_text(owner, repo, branch, path)
        return text_cache[path]

    for directory in sorted(dirs):
        page = repo_page(owner, repo) if directory == "" else tree_page(owner, repo, directory)
        items = sorted(children[directory], key=lambda x: (x["type"] != "tree", x["path"].lower()))
        table = ['<table class="files"><tr><th>Type</th><th>Filename</th><th>Size</th></tr>']
        for item in items:
            name = posixpath.basename(item["path"])
            dest = tree_page(owner, repo, item["path"]) if item["type"] == "tree" else file_page(owner, repo, item["path"])
            href = (link(page, dest) if item["type"] == "tree" or item["path"] in published_file_pages
                    else raw_url(owner, repo, branch, item["path"]))
            label = "DIR" if item["type"] == "tree" else "FILE"
            size = "—" if item["type"] == "tree" else (str(item.get("size", 0)) + " B")
            table.append('<tr><td>' + label + '</td><td><a href="' + esc(href) + '">' + esc(name) + '</a></td><td>' + esc(size) + '</td></tr>')
        table.append("</table>")
        heading = '<h1>' + esc(info["name"]) + '</h1><p>' + esc(info.get("description") or "GitHub repository") + '</p>' if not directory else "<h2>Files</h2>"
        body = breadcrumbs(owner, repo, directory, page) + heading + "".join(table)
        readme = next((x["path"] for x in items if x["type"] == "blob" and posixpath.basename(x["path"]).lower() == "readme.md"), None)
        if readme and entries[readme].get("size", 0) < 130000 and readmes_built < MAX_DIRECTORY_READMES:
            readmes_built += 1
            try:
                body += '<section class="readme"><h2>README</h2>' + markdown_html(content(readme), owner, repo, branch, directory, page, entries) + "</section>"
            except requests.RequestException as e:
                print("README unavailable:", readme, str(e)[:100])
        render_page(page, owner + "/" + repo, body, (owner, repo))

    for item in files:
        path = item["path"]
        if path not in published_file_pages:
            continue
        page = file_page(owner, repo, path)
        body = breadcrumbs(owner, repo, posixpath.dirname(path), page) + '<h2>' + esc(posixpath.basename(path)) + '</h2>'
        body += '<p class="small">' + str(item.get("size", 0)) + ' bytes · <a href="' + esc(raw_url(owner, repo, branch, path)) + '">Original / download</a></p>'
        if path in selected:
            try:
                source = content(path)
                if path.lower().endswith((".md", ".markdown")):
                    body += '<section class="readme">' + markdown_html(source, owner, repo, branch, posixpath.dirname(path), page, entries) + '</section>'
                else:
                    body += '<pre class="code">' + esc(source[:120000]) + "</pre>"
            except requests.RequestException as e:
                body += '<p>Unable to load source: ' + esc(e) + '</p>'
        else:
            body += "<p>Binary or large source file. Use the original GitHub download link above.</p>"
        render_page(page, path, body, (owner, repo))

    releases_page = section_page(owner, repo, "releases")
    release_data = get_json("repos/" + owner + "/" + repo + "/releases?per_page=15")
    body = '<h1>Releases - ' + esc(repo) + '</h1>'
    for release in release_data:
        body += '<section class="release"><h2>' + esc(release.get("name") or release.get("tag_name")) + '</h2><p class="small">Tag: ' + esc(release.get("tag_name")) + ' · ' + esc(release.get("published_at") or "")[:10] + '</p>'
        for asset in release.get("assets", []):
            body += '<p><a href="' + esc(asset["browser_download_url"]) + '">Download: ' + esc(asset["name"]) + '</a> (' + str(round(asset.get("size", 0) / 1048576, 1)) + ' MB)</p>'
        body += markdown_html(release.get("body") or "", owner, repo, branch, "", releases_page, entries) + "</section>"
    if not release_data:
        body += "<p>No releases found.</p>"
    render_page(releases_page, "Releases " + repo, body, (owner, repo))

    issues_page = section_page(owner, repo, "issues")
    issue_data = get_json("repos/" + owner + "/" + repo + "/issues?state=open&per_page=25")
    issues = [x for x in issue_data if "pull_request" not in x]
    body = '<h1>Open Issues - ' + esc(repo) + '</h1>'
    for issue in issues:
        body += '<section class="release"><h3>#' + str(issue["number"]) + " " + esc(issue["title"]) + '</h3><p class="small">' + esc(issue.get("created_at", "")[:10]) + ' · <a href="' + esc(issue["html_url"]) + '">Open on GitHub</a></p>'
        body += markdown_html(issue.get("body") or "", owner, repo, branch, "", issues_page, entries) + "</section>"
    if not issues:
        body += "<p>No open issues.</p>"
    render_page(issues_page, "Issues " + repo, body, (owner, repo))
    print("Done:", repo, len(dirs), "directories", len(files), "files", IMAGE_COUNTS[(owner, repo)], "images", flush=True)


def main():
    REPOS[:] = public_repositories()
    print("Selected public repos:", len(REPOS), [repo for _, repo in REPOS], flush=True)
    (OUT / ".nojekyll").write_text("", encoding="utf-8")
    (OUT / "style.css").write_text('''*{box-sizing:border-box}body{margin:0;color:#24292f;background:#f6f8fa;font:15px Arial,Helvetica,sans-serif;line-height:1.5}header{background:#24292f;color:white;padding:12px 16px}header a{color:white;text-decoration:none;font-size:19px}header span{color:#c9d1d9}main{max-width:920px;margin:14px auto;background:white;padding:18px;border:1px solid #d0d7de}a{color:#0969da;text-decoration:none}a:hover{text-decoration:underline}h1{font-size:26px;margin:12px 0}h2{font-size:21px}h3{font-size:17px}.tabs,.crumb{padding:10px 0;border-bottom:1px solid #d8dee4;margin-bottom:14px}.tabs a{font-weight:bold}.files{width:100%;border-collapse:collapse;font-size:14px}.files td,.files th{padding:7px 6px;border-bottom:1px solid #d8dee4;text-align:left;word-break:break-word}.files th{background:#f6f8fa}.files td:first-child{width:44px;font-size:11px;color:#57606a}.files td:last-child{width:65px;color:#57606a}.readme{margin-top:18px;border:1px solid #d8dee4;padding:14px;overflow:hidden;word-wrap:break-word}.readme h1,.readme h2{padding-bottom:6px;border-bottom:1px solid #d8dee4}.readme img,.release img{max-width:100%}.readme img[width],.release img[width]{max-width:100%}.readme .align-center,.release .align-center{text-align:center}.readme table.align-center,.release table.align-center{margin-left:auto;margin-right:auto}.readme table,.release table{border-collapse:collapse;max-width:100%;table-layout:auto}.readme td,.readme th{vertical-align:middle}.readme table img{max-width:100%}.readme td,.readme th,.release td,.release th{border:1px solid #d8dee4;padding:5px 8px}.readme tr:nth-child(even){background:#f6f8fa}pre{overflow:auto;white-space:pre-wrap;word-wrap:break-word;background:#f6f8fa;border:1px solid #d8dee4;padding:10px;font:12px monospace}.small{color:#57606a;font-size:12px}.release{padding:12px 0;border-bottom:1px solid #d8dee4;overflow:hidden;word-wrap:break-word}.release img{max-width:100%}blockquote{border-left:4px solid #d8dee4;padding-left:12px;color:#57606a;margin-left:0}footer{max-width:920px;margin:18px auto;color:#57606a;font-size:12px;text-align:center}@media(max-width:960px){main{margin:0;padding:10px;border:0}.readme{padding:9px}header{padding:8px}}''', encoding="utf-8")
    links = []
    for owner, repo in REPOS:
        links.append('<li><a href="' + link(OUT / "index.html", repo_page(owner, repo)) + '">' + esc(owner + "/" + repo) + '</a></li>')
    render_page(OUT / "index.html", "Home", '<h1>GitHub for PS Vita</h1><p>Fast, formatted HTML for the original PlayStation Vita web browser. No JavaScript required.</p><h2>Repositories</h2><ul>' + "".join(links) + '</ul><p class="small">Static pages updated by GitHub Actions. All eligible public repositories are discovered automatically.</p>')
    failed = []
    for owner, repo in REPOS:
        try:
            make_repo(owner, repo)
        except Exception as exc:
            print("REPO ERROR:", owner, repo, type(exc).__name__, str(exc)[:160], flush=True)
            failed.append(repo)
            render_page(repo_page(owner, repo), repo, '<h1>' + esc(repo) + '</h1><p>Repository temporarily unavailable in this snapshot.</p>', (owner, repo))
            for section in ("releases", "issues"):
                render_page(section_page(owner, repo, section), section, '<p>Data unavailable for this repository.</p>', (owner, repo))
    print("Completed:", OUT, "failures:", failed, flush=True)
    if failed:
        print("WARNING: some repositories could not be fully mirrored:", failed, flush=True)


if __name__ == "__main__":
    main()
