# Wolff's Store





### v1.0.3 - CapUnlocker installation warning and YouTube alpha

- The v1.0.3 catalog uses store-v103.json; previous v1.0.x builds remain
  on their existing feed to avoid showing installs without plugin advisories.
- VPK items can declare optional aviso_plugin metadata. A blocking UI overlay
  explains the recommendation before starting the download, but the plugin
  itself is not required; the user can confirm or cancel the installation.
- The YouTube on TV alpha v0.34 is added through the Store-only Google Drive
  mirror and the YTVCB0001 Title ID.
- Pizza Tower SAGE 2019 v0.62 updates are catalog-only and retain the existing
  cover; the installer uses its official GitHub release asset.

### v1.0.2 - Download das traduções GameLab

- Renomeados os downloads: The Wolf Among Us (PTBR),
  The Walking Dead: Temporada 1 (PTBR) e Temporada 2 (PTBR).
- O Google Drive passa a ser o primeiro espelho; MediaFire é tentado como
  alternativa, e MEGA permanece disponível para download manual.
- MediaFire tem tempo de resolução máximo de 15 segundos, evitando que
  a loja fique permanentemente parada em zero durante consultas de CDN.
- Novos estados Resolvendo link... e Conectando... tornam a UI informativa,
  ocultando falsos 0%/0 KB até o início da transferência.
- Conexões lentas/falhas terminam e passam ao próximo espelho, preservando
  a instalação segura com senha fixa wolffsroom.

### v1.0.1 - Correções nas traduções

- Senha padronizada em wolffsroom para os ZIPs/VPKs compactados e pacotes
  rePatch: independe de campo eventualmente ausente no catálogo.
- ZIP com criptografia WinZip AES passa a ser extraído via libzip/OpenSSL,
  com validação de conteúdo, CRC, tamanho, cancelamento e logs de erro.
- Janela Instalar tradução redesenhada no estilo dos botões da loja.
  Textos em Latin-1 após a conversão do sistema de idiomas;
  rePatch detectado em amarelo, ausente com aviso e instalação bloqueada.
- Status inferior passa a exibir Sincronizando! e Catálogo atualizado!,
  com a exclamação ao final da mensagem.

### v1.0 - Traduções instaláveis e catálogo reorganizado

- Downloads ocupa a primeira caixa lateral dos posts, antes de Detalhes. Se um arquivo
  ainda não puder ser instalado pelo app, são exibidos os links manuais.
- Traduções VPK dentro de ZIP: extração do ZIP (inclusive com senha) e instalação do
  VPK pela rotina nativa do Wolff's Store (promoter).
- Traduções rePatch ZIP: verificação do plugin no taiHEN, confirmação e cópia de
  segurança _bkp antes de substituir modificações existentes.
- Arquivos WipEout 2048: modo app_arquivos, destino ux0:app/PCSF00007/,
  backup dos arquivos substituídos e aviso para atualizar LiveArea usando FMVita
  ou File Manager; downloads que contêm apenas o torrent não são instaláveis.
- Os arquivos do site usam a data original de publicação; o app não confunde
  atualização de ficha com novidade. A integração mantém a música/OST e o
  atualizador com Title ID WFFS00042 da série 0.99.x.



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

### v1.0.4: plugins opcionais no Vita

- Downloads de plugins são organizados em ux0:Downloads/Plugins/.
- TouchOnVita (PCSE00919) oferece **Apenas baixar** ou **Baixar e instalar**.
- A instalação ocorre somente após confirmação, valida o ZIP oficial v0.59, cria backup da configuração taiHEN e do plugin existente e instala em ur0:tai/DDTouchUI.suprx e ux0:data/DDTouchUI/.
- Preserva mode.ini existente; sons protegidos do jogo não são distribuídos. **Reinicie o PS Vita** para aplicar a configuração taiHEN.
- Em caso de erro de cópia, tenta restaurar o backup em ux0:data/WolffsStore/plugin-backups/DDTouchUI/.

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
  baixa e reinstala: o updater (`WFFS00043`, dentro do VPK) instala a versão nova com o app fechado e
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

Gera `artifacts/current/WolffsStore.vpk` (Title ID `WFFS00042`). O VPK contém o catálogo textual e os
recursos da interface; as imagens dos posts ficam em `ux0:data/WolffsStore/cache/` após a sincronização.

### Release no GitHub

`.github/workflows/release.yml` compila o VPK no container `vitasdk/vitasdk` a cada push na `main`
(o VPK fica nos artefatos do Actions) e, num tag `v*`, publica a release com o VPK:

```
git tag v0.1
git push origin v0.1
```

### Migração de Title ID e atualização estável (v0.99.2)

A Store instalada é WFFS00042 e o updater temporário é WFFS00043.
Antes da migração, utilizavam-se os IDs WOLF00001 e WOLF00002.

Durante a atualização automática, o updater v0.99.2 primeiro instala
WFFS00042 e depois só exclui WOLF00001 quando o SFO instalado comprova
simultaneamente TITLE_ID=WOLF00001, TITLE="Wolff's Store" e uma versão
APP_VER 00.xx (anterior a v1.0). Se qualquer condição falhar, mantém o
aplicativo intacto. O marcador
ux0:data/WolffsStore/legacy_id_migration_v0992.done evita repetir essa
migração nas atualizações futuras. Não apaga ux0:data/WolffsStore.

Em instalações manuais do VPK, o updater não é executado e a bolha
antiga não é apagada automaticamente; nesse caso, confirme nome, versão e
Title ID antes de excluí-la manualmente pelo VitaShell/LiveArea.

O comando psgm:play passou a usar sceAppMgrLaunchAppByUri(0x20000, ...)
para iniciar diretamente o updater e a Store, em vez de voltar à LiveArea.
A música só começa após a Home estar carregada, desenhada e a animação de
entrada ter terminado. A limpeza dos helpers foi adiada até esse ponto,
evitando execução durante AFK na tela de abertura.


## LiveArea

- `sce_sys/icon0.png`: a patinha.
- `sce_sys/livearea/contents/bg0.png`: elementos do banner do YouTube (faixas em dither, estrelas de pixel,
  linhas pontilhadas) com "WOLFF'S STORE".
- `sce_sys/livearea/contents/startup.png`: a logo Wolff's Store.

## Lançar uma versão (auto-atualização)

1. Suba `WS_FILE_VERSION`/`WS_SFO_VERSION` no `CMakeLists.txt`.
2. Crie e envie a tag correspondente, no formato `v0.8`, `v0.9`… ou `v1.0`:
   `git tag v0.99.9 && git push origin v0.99.9`
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

### v0.99.5 - Background OST e diagnostico de LiveArea

A musica acompanha o horario local do Vita em Automatico: 06-11 Morning
Pixels; 12-17 Afternoon Arcade; 18-05 Night Shift. Em Configuracoes,
Background OST alterna Automatico, Original (sintetizador antigo),
Morning Pixels, Afternoon Arcade e Night Shift. Os .pcm 24 kHz mono 16-bit
sao transmitidos em loop sem carregar os arquivos inteiros na RAM;
mudancas de faixa fazem fade-out/fade-in. A preferencia fica em settings.txt.

DIAGNOSTICO: a Store NAO desinstala WFFS00043 nem WOLF00002 enquanto
estiver executando; os helpers podem permanecer na LiveArea para isolar
o bug da primeira abertura. Os marcadores de cleanup permanecem intactos.
No proximo release, a limpeza devera ser feita fora da sessao da Store.

Inclui a correcao de video da v0.99.3: modo nativo do SceAvPlayer,
prioridade valida, MP4s otimizados v2 e logs de codec/primeiros frames.

Empacotamento v0.99.5: as tres faixas sao adicionadas ao VPK apenas
pela lista ws_files, evitando duplicacao no vita-create-vpk de Linux CI.


### v0.99.6 — menu de configuracoes compacto, video com diagnostico

A tela de Configuracoes agora possui as categorias Audio, Visual e Geral.
A raiz tem apenas tres linhas e cada submenu abre ate tres ajustes; o botao
O volta um nivel e START encerra Configuracoes. O ajuste OST continua gravado.

Video: tres metodos de tentativa (native GXM; callback GXM; callback
CDRAM/CPU), audio PCM independente e timeout com troca automatica
de decoder sem quadros. A mensagem mostra os erros em hexadecimal,
e video_diagnostic.txt fica diretamente em ux0:data/WolffsStore para
possibilitar coleta mesmo quando a pasta logs/ nao for copiada.

A limpeza do updater segue desativada na sessao ativa; isso evita o
retorno observado a LiveArea. Remocao off-session requer confirmacao
no Vita fisico para nao reintroduzir o problema.


### v0.99.7 - fix signed AvPlayer handles and guarded helper cleanup

Video: the Vita returns pointer-like AvPlayer handles in the 0x81xxxxxx
memory range, but the SDK declares SceAvPlayerHandle as signed int. Old
code used <= 0 and rejected legitimate 0x819xxxxx values from the Vita
video_diagnostic log. Now it distinguishes 0x80xxxxxx error codes from
valid player handles in playback, pause, seek, audio and close.

Auto-updater is removed only once the Home has remained stable for
45 consecutive seconds, without active jobs, modals, videos or IME.
The worker checks exact installed TITLE_ID and TITLE before deletion.
Markers are retained when deletion fails. Test the impact of
SceShell LiveArea refresh on actual Vita hardware after updating.


### v0.99.8 - Boot sync, audio extraction and download titles

The first network sync now completes and applies the newest featured post and
artwork BEFORE playing the opening question/intro, even with a cached catalog.
Offline/failure falls back to saved or bundled posts. Fast fetches show no
loading copy; a loading bounce appears only after 1.1 seconds and rotating
contextual phrases after 3.4 seconds if startup is genuinely slow.

The three 24-kHz PCM soundtracks are preloaded into ~5 MB of main memory
before starting the realtime BGM audio thread. Thus the mixer performs
no Vita storage reads during ZIP/VPK extraction, and the output grain is
1024 samples (21 ms at 48 kHz), reducing underruns during heavy I/O.
If memory is unavailable, the old streaming path remains a fallback.

Downloads and installed entries now show the current project title rather
than a download mirror label. Pokemon Emerald Voxel 3DS - with PT/BR is
included in the offline catalog and on the site; its Google Drive link
remains a manual alternative rather than an automatically installed VPK.


### v0.99.9 - restored typewriter after initial synchronization

v0.99.8 started synchronizing every boot before displaying the question.
On successful synchronization, apply_sync() can spend seconds decoding
covers and rebuilding the featured project's pixel mosaic on the main
thread. Its old frame-start now_s became stale, so setting title_t0 to
that value meant the typing timeline was already finished when the
question was first drawn.

The v0.99.9 transition stores the monotonic clock origin on startup and
samples now_s = clock_s() - app_clock_t0 AFTER apply_sync() finishes,
before resetting title_t0 and scheduling the original letter sound cues.
The same timestamp reset is used offline. All original 45-ms character
reveal, cursor, typing SFX and the introductory TV transition remain.

### v1.0.6 - Installed version recognition

- Fixed a persistent Update label after successful VPK installation when the publisher did not bump the internal APP_VER (DeltaruneVita v0.81 contains SFO version 00.80).
- Only prefer the successful Store installation version when installed.txt matches the actual app Title ID and stored path. External VPK installations and newer internal SFO versions continue to be detected.
- Existing successful installs already recorded under ux0:data/WolffsStore/installed.txt are recognized immediately; game reinstall is not required.
- The game VPK and SFO remain untouched.


### v1.0.7 — Interface, categorias e Journal

- Categorias no Vita: Ports (Finalizados, Em desenvolvimento, Próximos, Outras equipes), Apps/Mods (Apps, Ferramentas, Modificações, Plugins), Traduções e Journal (News, Reviews, Enquetes).
- Visual de News mais compacto, sem confundir pequenas notícias com lançamentos/atualizações.
- Novo conjunto de efeitos sonoros sintéticos de download, cancelamento, conclusão e aviso, mantendo o estilo original.
- Barra de download com animação de varredura, configurações com entrada suavizada e logo do updater animada; tela de sincronização inicial sem pata.
- Modal de notas da nova versão após a animação inicial, exibida uma vez por atualização reconhecida.
- **Enquetes — estrutura experimental:** seleção e confirmação, estado respondido, prazo configurável e barras de resultados animadas. Nenhuma votação real é habilitada enquanto o backend HTTPS com armazenamento seguro não estiver implantado. Consulte a pasta poll-backend do repositório do site. O VPK nunca contém um token GitHub.
- Os registros de instalação da versão 1.0.6 foram preservados, incluindo a correção do botão Atualizar/Reinstalar.

A categoria e o identificador de enquete são publicados pelo catálogo; a interface do Vita só oferece envio quando um endereço seguro de serviço estiver configurado. Validação em Vita físico de votos e exibição do resultado final continua pendente.
