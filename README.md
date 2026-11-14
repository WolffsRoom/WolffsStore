# Wolff's Store

O site [Wolff's Room](https://wolffsroom.github.io/WolffsRoomSite/) como app de PS Vita: mesmo visual, fontes,
organização, animações e sons.

- Intro: tela preta com "What could Wolff be up to...?" digitado no centro. Ao tocar na tela ou apertar
  ✕/START, a pergunta some com o efeito de TV desligando, a capa do projeto atual liga grande no centro,
  se depixeliza em 8 camadas, ganha o halo de pixels e encaixa no lugar; depois entram o topo (com a onda
  das letras), os textos, as categorias e a barra de dicas. ✕/START durante a intro pula para o final.
- Animações do site: transição em dither entre telas, cards e caixas entrando em sequência, halo que abre
  no item com foco, a volta ao início liga a capa de novo (TV, mosaico e halo) com um som próprio, botão "afunda" ao apertar, barras de progresso enchendo em 12 passos, barra de rolagem
  em pixels. Mesmos efeitos sonoros (sintetizador chiptune com pulso 12,5/25/50%, triângulo, ruído e eco).
- Página inicial com o destaque, categorias (Ports, Ferramentas, Traduções, Reviews) e "Últimos em…".
- Lista por categoria com os grupos Em andamento / Finalizado / Próximos / Outras equipes.
- Página do post: capa, texto, seções, capturas de tela (tela cheia), vídeo, ficha, barras de progresso e links
  (abrem no navegador do Vita).
- Inglês e português (SELECT), som liga/desliga (START). As escolhas ficam em `ux0:data/WolffsStore/`.

### v0.2: loja

- **Sincroniza com o site** ao abrir e no △ (ou no botão ↻ do topo): baixa o `store.json` que o site gera
  (`https://wolffsroom.github.io/WolffsRoomSite/store.json`) e as capas/capturas que faltam. Tudo fica em
  `ux0:data/WolffsStore/cache/`, então sem internet abre com o último catálogo (ou o que veio no VPK).
- **Downloads** (caixa "Downloads" na página do projeto, configurada no `post.md` do site):
  - VPK: baixa e instala sozinho. Se o jogo precisa dos arquivos de dados, mostra o aviso
    "Esse jogo precisa dos arquivos de dados do jogo, confira as instruções no GitHub" e o botão das instruções.
    Com `titleid`, mostra a versão instalada e "Atualizar" quando há versão nova.
  - Tradução: baixa o zip e extrai em `ux0:rePatch/<TITLEID>/`; avisa se o plugin rePatch não está no
    `config.txt` do taiHEN.
  - Plugin/arquivo: baixa para `ux0:Downloads/` e aponta as instruções.
- **Segundo plano**, estilo PKGj: fila de downloads (□ abre a tela Downloads, com cancelar e limpar),
  faixa de progresso acima da barra de dicas, Vita não suspende enquanto baixa, botão PS travado só
  durante a instalação, checagem de espaço livre antes de começar.
- **Auto-atualização**: ao abrir, a v0.5 ou posterior consulta a release mais recente de
  `WolffsRoom/WolffsStore` e procura o asset `WolffsStore.vpk`. Se a versão da tag `v*` for maior,
  baixa e reinstala: o updater (`WOLF00002`, dentro do VPK) instala a versão nova com o app fechado e
  abre o Wolff's Store de novo. ○ deixa para depois. O campo `app` do `store.json` continua aceito como
  fallback, inclusive para levar a v0.41 até a v0.5 (anunciada como `0.50` para compatibilidade).

## Controles

| Botão | Ação |
| --- | --- |
| Direcional / analógico esquerdo | Mover o foco |
| ✕ | Abrir |
| ○ | Voltar |
| △ | Sincronizar com o site |
| □ | Downloads |
| L / R, analógico direito, arrastar na tela | Rolar |
| SELECT | Pesquisar |
| START | Configurações |

No topo ficam os botões Sincronizar, Downloads, Configurações e Pesquisa. As configurações têm
efeitos sonoros, música de fundo (chiptune misteriosa em lá menor, em loop), idioma (PT-BR / English) e brilho
(40–100%, escurece a imagem do app). O que o app instala fica registrado em
`ux0:data/WolffsStore/installed.txt`, e cada download mostra se está instalado, atualizado ou com
versão nova (tela Downloads, seção Instalados).

## Dados

Os posts vêm do repo do site. Depois de mudar um post lá, regenere `data/`:

```
pip install pillow pyyaml
python tools/gen_data.py C:\caminho\para\WolffsRoomSite
```

O script usa o mesmo leitor de posts do site (`tools/build.py`) e grava `data/catalog.txt` (Latin-1),
as texturas da interface e as capas/capturas em `.wrt`. O build inclui o catálogo e `data/tex`, mas não
inclui `data/posts`: capas e capturas são baixadas para o cache na primeira sincronização.

## Build

Igual ao VitaSeed (Docker com VitaSDK):

```
powershell -ExecutionPolicy Bypass -File scripts\build.ps1
# ou, regenerando os dados antes:
powershell -ExecutionPolicy Bypass -File scripts\build.ps1 -Site C:\caminho\para\WolffsRoomSite
```

Gera `artifacts/current/WolffsStore.vpk` (Title ID `WOLF00001`). O VPK contém o catálogo textual e os
recursos da interface; as imagens dos posts ficam em `ux0:data/WolffsStore/cache/` após a sincronização.

### Release no GitHub

`.github/workflows/release.yml` compila o VPK no container `vitasdk/vitasdk` a cada push na `main`
(o VPK fica nos artefatos do Actions) e, num tag `v*`, publica a release com o VPK:

```
git tag v0.1
git push origin v0.1
```

## LiveArea

- `sce_sys/icon0.png`: a patinha.
- `sce_sys/livearea/contents/bg0.png`: elementos do banner do YouTube (faixas em dither, estrelas de pixel,
  linhas pontilhadas) com "WOLFF'S STORE".
- `sce_sys/livearea/contents/startup.png`: a logo Wolff's Store.

## Lançar uma versão (auto-atualização)

1. Suba `WS_FILE_VERSION`/`WS_SFO_VERSION` no `CMakeLists.txt`.
2. Crie e envie a tag correspondente, no formato `v0.8`, `v0.9`… ou `v1.0`:
   `git tag v0.93 && git push origin v0.93`
3. O workflow publica a release em `WolffsRoom/WolffsStore` com o asset fixo `WolffsStore.vpk`.
   A URL permanente da versão mais recente é
   `https://github.com/WolffsRoom/WolffsStore/releases/latest/download/WolffsStore.vpk`.

O site expõe a v0.5 como `0.50` no campo `app` do `store.json`, para que a v0.41 a reconheça como
mais nova. Da v0.5 em diante, o app também consulta o GitHub diretamente e não depende do rebuild do site.

## Créditos e licenças

- A instalação de VPKs (`src/install.c`, `resources/head.bin`) e o updater seguem o VitaShell
  (TheFloW, GPLv3), via VitaSeed; por isso o Wolff's Store é distribuído sob a GPLv3.
- Rede (`src/net.c`) e JSON (`src/json.c`) vêm do VitaSeed. SHA-1: Brad Conte (domínio público).

- Fontes: Press Start 2P (CodeMan38) e Pixelify Sans (Stefie Justprince), ambas SIL Open Font License 1.1
  (ver `assets/fonts/*OFL*.txt`).
- Capas e capturas de tela pertencem aos respectivos jogos e projetos.
