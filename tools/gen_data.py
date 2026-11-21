#!/usr/bin/env python3
"""Gera os dados do Wolff's Store a partir dos posts do site (repo WolffsRoomSite).

Uso:
    pip install pillow pyyaml
    python tools/gen_data.py <pasta do repo WolffsRoomSite>

Lê content/posts/*/post.md e post.en.md com o mesmo leitor do site
(tools/build.py) e grava em data/:
    catalog.txt          catálogo (texto Latin-1, uma chave por linha)
    tex/*.wrt            texturas da interface (fontes, pata, halos, dither)
    posts/<slug>/*.wrt   capa e capturas de tela de cada post

.wrt = "WRT1" + largura (u16) + altura (u16) + pixels RGBA8. O app carrega
direto na memória de vídeo, sem precisar de libpng.
"""
import os
import re
import shutil
import struct
import sys

from PIL import Image, ImageDraw, ImageFont

HERE = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
OUT = os.path.join(HERE, 'data')
FONTS = os.path.join(HERE, 'assets', 'fonts')

COVER = (512, 288)   # capa: destaque, cards e detalhe
SHOT = (480, 270)    # capturas de tela

LATIN = {'—': '-', '–': '-', '“': '"', '”': '"', '‘': "'", '’': "'", '…': '...', '→': '>', '←': '<',
         '•': '·', '×': 'x', ' ': ' ', '™': '(TM)', '®': '(R)'}


def latin(s):
    s = str(s)
    s = re.sub(r'\[([^\]]+)\]\((https?://[^)\s]+)\)', r'\1', s)
    s = re.sub(r'\*\*([^*]+)\*\*', r'\1', s)
    # Native Vita text uses Latin-1 rather than Markdown styling.
    s = s.replace('**', '').replace('*', '')
    out = []
    for ch in s:
        ch = LATIN.get(ch, ch)
        try:
            ch.encode('latin-1')
            out.append(ch)
        except UnicodeEncodeError:
            out.append('?')
    return ''.join(out).replace('\t', ' ').replace('\n', ' ')


def wrt(img, path):
    img = img.convert('RGBA')
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'wb') as f:
        f.write(b'WRT1' + struct.pack('<HH', *img.size) + img.tobytes())


def fit(img, size, style, pixel):
    """'cobrir' corta para preencher; 'logo'/'conter' encaixa inteiro sobre preto."""
    W, H = size
    rs = Image.NEAREST if pixel else Image.LANCZOS
    img = img.convert('RGBA')
    if style in ('logo', 'conter'):
        k = min(W / img.width, H / img.height)
        if style == 'logo':
            k *= 0.8
        im = img.resize((max(1, round(img.width * k)), max(1, round(img.height * k))), rs)
        bg = Image.new('RGBA', size, (0, 0, 0, 255))
        bg.alpha_composite(im, ((W - im.width) // 2, (H - im.height) // 2))
        return bg
    k = max(W / img.width, H / img.height)
    im = img.resize((max(W, round(img.width * k)), max(H, round(img.height * k))), rs)
    x, y = (im.width - W) // 2, (im.height - H) // 2
    bg = Image.new('RGBA', size, (0, 0, 0, 255))
    bg.alpha_composite(im.crop((x, y, x + W, y + H)))
    return bg


# ---------------------------------------------------------------- fontes

def font_title():
    """WR Start 2P (Press Start 2P com as maiúsculas acentuadas corrigidas) no tamanho nativo (8 px):
    16x14 células de 8x11 (Latin-1 32..255). As 3 linhas de cima são para o acento de Ã, É etc.,
    que fica acima da letra; a letra em si começa na linha 3."""
    F = ImageFont.truetype(os.path.join(FONTS, 'WRStart2P-Regular.ttf'), 8)
    atlas = Image.new('RGBA', (128, 14 * 11), (0, 0, 0, 0))
    for c in range(32, 256):
        m = Image.new('1', (8, 11), 0)
        d = ImageDraw.Draw(m)
        d.fontmode = '1'
        d.text((0, 3), chr(c), font=F, fill=1)
        i = c - 32
        atlas.paste((255, 255, 255, 255), ((i % 16) * 8, (i // 16) * 11), m)
    wrt(atlas, os.path.join(OUT, 'tex', 'font_title.wrt'))


def font_body():
    """Pixelify Sans a 18 px (o tamanho do texto do site), largura própria por letra."""
    F = ImageFont.truetype(os.path.join(FONTS, 'PixelifySans.ttf'), 18)
    CW, CH = 24, 24
    atlas = Image.new('RGBA', (16 * CW, 14 * CH), (0, 0, 0, 0))
    widths = bytearray()
    for c in range(32, 256):
        ch = chr(c)
        m = Image.new('1', (CW, CH), 0)
        d = ImageDraw.Draw(m)
        d.fontmode = '1'
        d.text((1, 0), ch, font=F, fill=1)
        i = c - 32
        atlas.paste((255, 255, 255, 255), ((i % 16) * CW, (i // 16) * CH), m)
        widths.append(max(1, min(CW, round(F.getlength(ch)))))
    wrt(atlas, os.path.join(OUT, 'tex', 'font_body.wrt'))
    with open(os.path.join(OUT, 'tex', 'font_body.wid'), 'wb') as f:
        f.write(bytes(widths))


# ---------------------------------------------------------------- posts

def main():
    if len(sys.argv) < 2:
        sys.exit('uso: python tools/gen_data.py <pasta do repo WolffsRoomSite>')
    site = os.path.abspath(sys.argv[1])
    sys.path.insert(0, os.path.join(site, 'tools'))
    import build  # leitor oficial dos posts do site

    if os.path.isdir(OUT):
        # v0.99.9 introduced the bundled daypart OST under data/music.
        # Refresh generated images/catalog without deleting bundled music or TLS certificates.
        for name in os.listdir(OUT):
            if name in ('music', 'certs'):
                continue
            target = os.path.join(OUT, name)
            if os.path.isdir(target):
                shutil.rmtree(target)
            else:
                os.remove(target)
    os.makedirs(os.path.join(OUT, 'tex'), exist_ok=True)
    font_title()
    font_body()
    for n in ('paw4', 'halo24', 'halo12', 'w25', 'w50', 'w75'):
        wrt(Image.open(os.path.join(site, 'assets', n + '.png')), os.path.join(OUT, 'tex', n + '.wrt'))

    posts, order = build.load_all()
    from store import populate_download_totals
    populate_download_totals(posts)
    feat = next((k for k in order if posts[k]['feature']), order[0])
    L = []

    def put(k, v):
        L.append(f'{k}\t{latin(v)}')

    for slug in order:
        p = posts[slug]
        en = p['en'] or {}
        folder = p['folder']
        put('post', slug)
        if p.get('github_downloads_total') is not None:
            put('github_downloads_total', str(p['github_downloads_total']))
        put('cat', p['cat'])
        put('subcat', p.get('subcat') or '')
        if p.get('poll'):
            survey=p['poll']
            put('poll_id', survey['id'])
            put('poll_question', survey['question'])
            put('poll_question.en', en.get('poll_question') or survey['question'])
            put('poll_starts_at', survey['starts_at'])
            put('poll_ends_at', survey['ends_at'])
            for opt in survey['options']:
                put('poll_option', opt)
            for opt in (en.get('poll_options') or survey['options']):
                put('poll_option.en', opt)
        put('group', build.GROUPS.index(p['group']))
        put('date', p['date'].isoformat())
        if p.get('updated'):
            put('updated', p['updated'].isoformat())
        if slug == feat:
            put('feat', 1)
        if p['pixel']:
            put('pixel', 1)
        if p.get('trophies'):
            put('trofeus', 1)
        if p.get('video_file'):
            vf = p['video_file']
            put('video_arquivo', vf if vf.startswith('http') else 'https://wolffsroom.github.io/WolffsRoomSite/' + vf)
        if p['progress'] is not None:
            put('progress', p['progress'])
        for lg, src in (('pt', p), ('en', {**p, **en})):
            for key in ('title', 'tag', 'status', 'desc', 'feature_text', 'bars_title'):
                if src.get(key):
                    put(f'{key}.{lg}', src[key])
            if src.get('about'):
                put(f'about.{lg}', src['about'])
            for s in src.get('secs') or []:
                put(f'sec.{lg}', s['title'])
                for para in s['paras']:
                    put(f'para.{lg}', para)
                for it in s['items']:
                    put(f'item.{lg}', it)
            for label, pct, note in src.get('bars') or []:
                put(f'bar.{lg}', f'{latin(label)}|{pct}|{latin(note or "")}')
            for k, v in src.get('meta') or []:
                put(f'info.{lg}', f'{latin(k)}|{latin(v)}')
            for k, v in src.get('links') or []:
                put(f'link.{lg}', f'{latin(k)}|{v}')
        dst = os.path.join(OUT, 'posts', slug)
        if p['cover']:
            img = Image.open(os.path.join(folder, os.path.basename(p['cover'])))
            wrt(fit(img, COVER, p['cover_style'], p['pixel']), os.path.join(dst, 'cover.wrt'))
            put('cover', 1)
        n = 0
        for s in p['shots']:
            img = Image.open(os.path.join(folder, os.path.basename(s)))
            wrt(fit(img, SHOT, 'conter', p['pixel']), os.path.join(dst, f'shot{n}.wrt'))
            n += 1
        put('shots', n)
        if p['video']:
            put('video', 'https://youtu.be/' + p['video'])
        put('end', '')
    with open(os.path.join(OUT, 'catalog.txt'), 'wb') as f:
        f.write(('\n'.join(L) + '\n').encode('latin-1'))
    print(f'{len(order)} posts -> {OUT}')


if __name__ == '__main__':
    main()
