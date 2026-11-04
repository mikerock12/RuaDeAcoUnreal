# Contexto do Projeto — Rua de Aço Nativo (`contexto.md`)
## Plataforma e icones Android — 03/11/2026

Configuracoes persistentes e icones APK tradicionais/adaptativos implementados em 0.4.0. Windows tem modos de tela, VSync/FPS e render interno ate4K; Android tem HUD sempre visivel, controles por toque ajustaveis/canhoto, vibracao e instrucoes proprias. Volumes musica/voz/efeitos ligam aos respectivos buses com ganhos canonicos. CTest4/4 e CI41756265525 aprovados, com93capturas/9percursos Android e preferencias apos reinicio. Cliente oficial online integrado ao Windows/WebView2 e Android/WebView, alternativa de navegador e bootstrapper Microsoft assinado; teste adicional de abrir/retornar OnlineActivity esta em CI. Salas e inicio identico em Windows integrado/navegador foram observados; a janela oculta de QA limitou rAF a cerca1FPS e a partida foi interrompida sem resultado. Nao afirmar homologacao de luta sustentada nesse host oculto. As instalacoes existentes foram preservadas; Windows portatil gerou29quadros+5settings, quatroEXEs PE(icon) e APKs comparados com arte aprovada. APK ARM64 assinado com certificado anterior v2/v3 e alinhamento16KiB. Release, merge e backup coordenados no portal.

Sem homologacao Android fisica, audio ouvido, desempenho/toque fisicos ou dupla Wi-Fi. Responsavel: Codex.



## Versão de entrega 0.3.0 — resultado de 03/11/2026

Aplicação e50d6cf validada: CTest 3/3, 46 checks de componentes, 33 replays TypeScript/16.170 passos (campos discretos exatos, tolerância float 0,002). Windows instalado em diretório isolado: 342 arquivos iguais ao ZIP, headless, 29 capturas reais e desinstalação limpa. CI 41588386978 aprovado nos três jobs: Android 15/API 35 x86_64/SwiftShader, nove percursos, seis lutadores e três arenas, em 16:9 e telefone inicialmente em retrato girando para 20:9. As 81 capturas foram revisadas; arena/lutadores, HUD, inputs, pausa e background/retorno corretos, sem erros de assets ou crashes.

APK ARM64 com os 338 assets verificados por SHA-256/CRC, assinatura v2/v3, certificado igual à versão 0.2.0 e alinhamento de 16 KiB. O original TypeScript continua canônico; o frontend C++ reaproveitado acompanha suas regras e apresentação. Touch controla P1; 2P local requer controles externos. Sem aparelhos físicos: atualização ARM64 instalada, áudio ouvido, desempenho e multitouch físico não foram homologados. Replays não comparam hashes completos nem provam igualdade visual pixel a pixel. Publicação e backups são coordenados no AGENTS.md/contexto.md do portal. Responsável: Codex.

Os registros anteriores abaixo documentam a investigação e suas pendências históricas; o resultado atual está neste bloco.

## Superfície e seleção Android — 03/11/2026

Após a correção ZIP, o CI 41586905713 carregou imagens, menus e partidas reais e completou quatro percursos de entrada/pausa/retorno. Falhou ao retomar a pausa no quinto percurso com `adb input tap` sem duração. A revisão visual também encontrou um erro real: o toque no botão Cais ativava o joystick invisível do menu e incrementava a arena para Cozinha. Os controles de combate touch agora só atuam durante uma partida; seleção emite o estágio real para o smoke exigir igualdade com a escolha.

Android usa `InitWindow(0,0)` para aproveitar a superfície física, evitando a dupla escala da janela solicitada no raylib 5.5; orientação landscape e fullscreen são configurados antes de iniciar a thread nativa. O driver usa toque imóvel de 120 ms e exige pausa/retomada, arena/lutadores exatos e ausência de erros de assets. A nova matriz cobre seis lutadores em 1280×720 e três arenas adicionais partindo de telefone portrait 720×1600, que deve girar para 1600×720 (20:9). Execução dessa matriz e novos pacotes Windows/Android ainda pendentes. O script do instalador Windows agora aguarda o processo GUI e lê `Process.ExitCode`, sem depender de `$LASTEXITCODE` anterior. Responsáveis: Codex QA Android e coordenador.

## Empacotamento Android — 03/11/2026

O teste real em emulador revelou nomes ZIP com barras invertidas produzidos por aapt2 no Windows. Android não os resolve, embora ferramentas Windows normalizem sua exibição. `Scripts/normalize-apk-paths.py` reescreve os nomes locais e centrais antes de zipalign/assinatura e verifica os 338 assets contra a origem por SHA-256 e CRC. O smoke também rejeita nomes brutos inválidos antes da instalação. Reexecução Android pendente; não publicar candidatos anteriores. Windows instalado: 342 arquivos idênticos ao ZIP, headless e 29 capturas reais aprovados, desinstalação limpa. Componentes finais: 46 checks; CTest 3/3. Responsável: Codex.

## Correção 0.3.0 em validação — 02/11/2026

A implementação 0.2.0 publicada era insuficiente: sua tela Android simplificava o jogo, descartava áudio e os antigos testes chamados golden não executavam os replays TypeScript. Esse histórico não comprova paridade.

O port agora consome definições, colisões calibradas, múltiplas janelas, comandos, eventos, passivas e poses atuais exportados do original. A comparação real cobre 33 replays / 16.170 passos, com igualdade dos campos discretos exportados e tolerância float de 0,002 px; os hashes completos internos não são comparados e não há lockstep nativo com a Web. Os testes de componentes são separados (45 verificações antes da revisão final da CPU). Fonte e método em Docs/PARITY_REPLAYS.md.

Frontend C++/raylib compartilhado com Android NativeActivity, 224 animações canônicas, efeitos/vozes por golpe e lutador, controles multitouch com bordas, menus touch, teclado e dois gamepads. O HUD touch controla P1; versus local 2P requer controles externos ou teclado. Assets são sincronizados do original por Scripts/Sync-CanonicalPresentation.py com manifesto de hashes. As camadas das três arenas e suas finalizações reutilizam a apresentação original. Render smoke Windows e CTest passaram em uma árvore nova; geração dos candidatos finais e execução real Android no CI seguem em validação. Runtime MSVC estático evita depender de uma instalação de Visual Studio/redistribuível no destino.

Pendências antes de concluir: congelar fonte após revisão final da CPU/palcos, repetir build/testes/capturas finais, empacotar e validar instalador, executar emulador Android, conferir assinatura e certificado de atualização, PR/CI/merge, releases, portal e backups privados com restauração. O usuário autorizou reutilizar a chave e a senha Android existentes sem exibi-las/publicá-las. Sem aparelhos físicos disponíveis; não declarar homologação física nem conclusão de áudio/desempenho em ARM64 a partir de compilação ou imagens. Responsável: Codex.

## Localização real dos projetos — 02/11/2026

Raiz de trabalho confirmada: `D:\PROJETOS\SoftwaresSmellsLikeTech`. Não procurar nem criar cópias em `D:\PROJETOS\RuaDeAco`, `D:\PROJETOS\RuaDeAcoUnreal` ou `D:\MikeAndDenyse`: referências a esses locais em registros antigos são históricas.

| Pasta real | Papel |
|---|---|
| `D:\PROJETOS\SoftwaresSmellsLikeTech` | Portal existente e coordenação da publicação; preserva seus cinco submódulos de softwares. |
| `D:\PROJETOS\SoftwaresSmellsLikeTech\RuaDeAco` | Jogo original TypeScript/Phaser atualizado; fonte canônica de regras, lutadores, golpes, apresentação, arte e áudio para o port C++. |
| `D:\PROJETOS\SoftwaresSmellsLikeTech\RuaDeAcoUnreal` | Base C++ reaproveitada e atualizada conforme o original; sua implementação antiga não define o gameplay nem as capturas atuais. |
| `D:\PROJETOS\SoftwaresSmellsLikeTech\MikeAndDenyse` | Repositório Nightfall; referência Unity preservada em `NightfallUnity/` e port C++ em `NightfallNative/`. |
| `D:\PROJETOS\SoftwaresSmellsLikeTech\RuaDeAco_AndroidBuild` | Árvore auxiliar existente de build Android/Capacitor do Rua de Aço; não substitui a fonte canônica `RuaDeAco` nem o build nativo em `RuaDeAcoUnreal`. |
| `D:\PROJETOS\SoftwaresSmellsLikeTech\RuaDeAco_AudioMasters` | Matrizes de áudio existentes (`music/`); preservar os originais. Não é uma fonte alternativa de gameplay. |

Confirmar o Git de cada pasta antes de editar; estas pastas de jogos são projetos independentes dentro da raiz do portal, não novos submódulos nem assets estáticos a publicar. Capturas do site devem vir da versão indicada em execução e mostrar os lutadores atuais. Responsável: Codex.

**Última atualização:** 01/11/2026  
**Repositório:** `https://github.com/mikerock12/RuaDeAcoUnreal`  
**Branch principal:** `main`

---

## 1. Visão Geral e Arquitetura

Este repositório contém a implementação nativa em C++20 do jogo de luta **Rua de Aço**, estruturada em três camadas:

1. **`Source/RuaDeAcoRuntime/`**: Núcleo de simulação de combate em C++20 a 60 Hz fixos (`1/60 s`), determinístico e independente de motor gráfico (sem dependência de Unreal ou raylib).
2. **`Source/Prototype/`**: Frontend nativo leve de alta performance em C++20 + `raylib 5.5` (`ThirdParty/raylib`), responsável pela janela, renderização pixel art, animações por spritesheet, HUD, menus, controles de teclado/gamepad/toque e áudio (`raudio`), empacotado para **Windows x64** (`.exe` / `.zip`) e **Android ARM64** (`.apk` NDK).
3. **`Source/RuaDeAco/`**: Adaptadores opcionais para Unreal Engine 5 (`ARuaDeAcoArenaGameMode`, `ARuaDeAcoFighterCharacter`).

---

## 2. Diagnóstico de Baseline (Fase G0 — 01/11/2026)

- **Commit inicial (`main`)**: `a5b40ff8afd65900c59008d8a0a75b60da0d3a4b` (`v0.1.0`).
- **Build Baseline**: `Scripts\Build-Windows.ps1 -Configuration Release` compilou sem erros com Visual Studio 2022 (`v17.14.44`), produzindo `Saved\Packages\Windows\RuaDeAcoUnreal.exe`.
- **Lacunas identificadas para corrigir nas Fases G2–G4**:
  - Catálogo atual tem apenas 4 lutadores (`guto`, `rafa`, `noir`, `astro`) com estatísticas antigas; faltam **Dante Sinal** (`dante`) e **Léo Violeta** (`leo`), além dos valores atualizados de caminhada/pulo/gravidade da versão web atual.
  - Há apenas 1 arena implícita (Cais); faltam **Cozinha Macabra** (`cozinha`) e **Sítio** (`sitio`), bem como seleção de arena e finalizações específicas de cada palco.
  - Faltam agarrão universal (`F+G` / `J+K`), projéteis/especiais completos, barra de super/finisher alinhada, modos completos (Vs CPU, Versus Local, Treino com visualização de hitboxes/frame data), remapeamento e HUD touch para Android.
  - Caminhos hardcoded `D:\PROJETOS\RuaDeAco` em `Scripts/Validate-Project.ps1` e `D:/PROJETOS/RuaDeAcoUnreal/Content` em `PrototypeMain.cpp` precisam ser substituídos por resolução relativa portátil.
  - Não havia `CMakeLists.txt`, `CTest`, instalador Inno Setup nem projeto Android NDK (`Platform/Android`).

---

## 3. Histórico de Etapas

| Data | Fase / Marco | Verificações | Responsável |
|---|---|---|---|
| 01/11/2026 | **Fase G0**: Inventário, criação de `AGENTS.md` e `contexto.md`, build baseline Windows Release | `Scripts\Build-Windows.ps1 -Configuration Release` aprovado (exit 0) | Coordenador SLT Games |
| 01/11/2026 | **Fase G1**: Exportado `golden_fixtures.json` canônico em `RuaDeAco` (PR #9, merge `b41dd21`) | 591 testes cliente + 71 servidor aprovados; JSON pronto para ingestão C++ | Coordenador SLT Games |
| 01/11/2026 | **Fases G2 e G3**: Paridade C++20 + raylib (6 lutadores, 3 arenas, agarrão universal, finishers, Suno SFX, CMake + CTest) | CTest 2/2 suítes (`44/44` checks golden + smoke headless), 10.000 ticks em `7,26 ms` (`0,00073 ms/tick`) | Coordenador SLT Games |
| 01/11/2026 | **Fase G4**: Empacotamento Windows (`.exe` Inno Setup + `.zip`), Android ARM64 (`.apk` NDK 27 + SDK 36) e CI (`.github/workflows/cpp-ci.yml`) | `RuaDeAco-0.2.0-Windows-x64-Setup.exe` (`52.827.371 B`), `.zip` (`50.990.468 B`), `.apk` (`38.670.466 B`, `apksigner verify` v2/v3 OK) | Coordenador SLT Games |
| 01/11/2026 | **Fases G5 e G6**: PR #1 mesclado em `main` (`9dc9232`); `MikeAndDenyse-Nightfall` v1.0.0 concluído | Pronto para publicação da Release `v0.2.0` na Fase G7 | Coordenador SLT Games |
| 01/11/2026 | **Fase G7**: Publicação da Release GitHub estável `v0.2.0` (`https://github.com/mikerock12/RuaDeAcoUnreal/releases/tag/v0.2.0`) | 4 assets publicados e verificados por digest SHA-256 | Coordenador SLT Games |
| 01/11/2026 | **Fase G8**: Publicação de `/games/rua-de-aco/` no portal (`5fe367e`, Worker `f24ab0eb-d181-435b-b8ce-5764468138c8`) | Páginas 200, APIs 200 com SHA-256 v0.2.0 e redirects 302 validados | Coordenador SLT Games |


## Ícones Windows — 03/11/2026

Preparada versão 0.3.1. Logo original Rua de Aço preservado, com origem e SHA-256 registrados. ICO com dez tamanhos (16–256 px) incorporado ao jogo e instalador; PNG incorporado ao PE para SetWindowIcon, atalhos e desinstalação apontam ao EXE. CMake/MSVC e Inno Setup compilam os recursos; o script direto do Rua de Aço também. Verificação PE dos quatro executáveis comparou todas as imagens ao ICO aprovado; as duas janelas reais devolveram ícone 256×256 e logs de aplicação. Instalador RDA: 342 arquivos iguais ao ZIP, headless, 29 capturas e desinstalação aprovados. Nightfall: instalador compilado e ícone extraído; execução portátil com ícone conferida. Instalação Nightfall 1.0.1 já existe em Program Files e foi preservada, impedindo repetir o teste de instalação neste host.

APKs recebem apenas versão de manutenção, com a mesma assinatura v2/v3 anterior e alinhamento 16 KiB; gameplay permanece o da correção anterior. PR/CI, releases, portal e backup desta etapa serão consolidados pelo coordenador no contexto do portal. Testes físicos Android continuam pendentes. Responsável: Codex.


## Atualizacao de plataforma 0.4.0 — em validacao, 03/11/2026

Configuracoes persistentes de tela, qualidade, audio, teclado/controle e touch integradas ao frontend compartilhado. Android tem HUD sempre visivel, tamanho/opacidade/margem/canhoto e vibracao; icones tradicionais e adaptativos derivam da mesma arte Windows aprovada, com procedencia em Platform/Android/Branding (NightfallNative no Nightfall). Windows oferece resolucao, modos de janela, VSync, limites FPS e render interno ate 4K. Nightfall usa subsistema GUI e simulacao fixa 60Hz. Rua de Aco abre o cliente Web oficial integrado para compatibilidade de salas; combate offline continua C++.

Candidatos locais compilaram para Windows e Android x86_64; CTest anterior 4/4 RDA e 3/3 Nightfall. QA ampliado exige configuracoes por toque e persistencia apos reiniciar; CI, online real, empacotamento final, novas capturas, releases, portal e backups ainda pendentes. Suno fica por ultimo por instrucao do usuario, indisponivel por verificacao do site; nenhum audio novo gerado. Sem homologacao Android fisica. Responsavel: Codex.


### Configuracoes e icones APK — 03/11/2026, em validacao
Configuracoes de plataforma e touch persistentes, icones Android da arte Windows aprovada, pausa de audio no background e executavel Windows GUI implementados. Simulacao de frontend desacoplada do FPS; P2 aceita segundo gamepad. QA passou a conferir 15 recursos PNG compilados por jogo, dois XML adaptativos, preferencias apos reinicio e gestos reais no emulador. Primeiro CI encontrou falta de carregamento explicito da biblioteca para o JNI de lifecycle no Nightfall e seletor de borda obsoleto no driver RDA; corrigidos, gates serao repetidos. Nao ha nova release ainda. Audio Suno adiado para o fim pelo usuario, sem clips gerados; nao expor ajustes sem audio correspondente. Sem Android fisico/dupla Wi-Fi. Responsavel: Codex.


## Superficie Android — 03/11/2026

O gate pos-merge encontrou inicializacao intermitente com a imagem comprimida verticalmente, enquanto as areas de toque permaneciam nas coordenadas esperadas. A inicializacao passa a obter as dimensoes fisicas da janela Android via WindowMetrics/Display antes de InitWindow, evitando dimensoes zero e a orientacao ambigua da raylib. O QA de Nightfall exige o titulo na posicao correta e a abertura real das configuracoes. No RDA, o teste online aguarda o jogo terminar o preload antes de capturar. Nao afrouxar assertions nem atribuir o problema a persistencia sem examinar a imagem. Nightfall prepara1.1.1/code16, preservando release1.1.0; RDA0.4.0 ainda candidato. CI/build/publicacao desta correcao pendentes. Responsavel: Codex.


Em 04/11/2026, o log do segundo formato Nightfall comprovou NativeWindow temporaria de1x1, apesar de WindowMetrics1000x600. A raylib5.5 calculava screenScale com esse buffer temporario e mantinha o jogo preto/comprimido. cmake/AndroidSurface.cmake preserva o upstream imutavel (SHA-256 a17a8c75...) e compila apenas uma copia de rcore com ANativeWindow_setBuffersGeometry aplicado antes da consulta inicial de dimensoes. A alteracao e marcada no codigo gerado e no log SLT_BUFFER_GEOMETRY. Validacao em andamento, sem nova publicacao desta manutencao. Responsavel: Codex.

## Plataforma0.4.0 publicada — 04/11/2026

PR4 mesclado em eb15db8451c18264a4154a3c411beaad2c642bbf; release estavel v0.4.0 com instalador/ZIP Windows e APK ARM64, digests remotos e seis downloads do portal conferidos. CTest4/4,46 checks,33 replays/16.170 passos, CI41784045643 e main41786346768 aprovados. QA Android emulado: nove percursos/94 capturas, seis lutadores, tres arenas, configuracoes/persistencia,16:9/20:9 e online oficial renderizado com retorno ao mesmo PID. Dois clientes de navegador compararam21 hashes ateframe1200; host Windows oculto nao demonstrou partida sustentada. Certificado anterior/v2/v3/16KiB e icones tradicionais/adaptativos conferidos.

Portal PR46 publicado no Worker75e932af-689b-4629-8fd3-72b5b964bc91: configuracoes e capturas atuais, versoes/plataformas explicitas; verify60/60 e19 checks Chrome aprovados, seis APIs/302/HEAD e seis binarios SHA/tamanho, privado401. Windows portatil executado com29 cenas e cinco ajustes; instalacao existente preservada. Original RuaDeAco permanece canonico e inalterado em0a15430; este port mantem o audio original. Backup privado final desta versao em consolidacao pelo portal. Sem Android fisico/dupla Wi-Fi, audio/performance/multitouch ARM64 ou atualizacao instalada fisica homologados. Responsavel: Codex.

## Rotacao Android ainda transitoria — correcao em validacao, 04/11/2026

O CI Nightfall41793542473 passou Windows e builds Android/banco75, mas a captura de titulo mostrou compressao vertical. Log comprovou SLT_BUFFER_GEOMETRY960x540/result0 seguido de display540x960 e offsets0,656: ANativeWindow continuou reportando retrato durante a rotacao, mesmo apos aceitar o novo buffer. O audio inicializou corretamente (16musicas/51falas/8efeitos); ele nao explica o defeito visual.

A copia identificada do backend raylib agora usa diretamente as dimensoes fisicas obtidas da Activity para SetupFramebuffer, conservando o tamanho bruto reportado apenas para diagnostico. A matriz nao deve incorporar a orientacao transitoria. Upstream5.5 e seu SHA continuam intactos. QA exige SLT_PHYSICAL_RENDER_SIZE igual a captura, alem das assertions visuais e toques existentes; nenhuma assertion foi afrouxada. Mesma correcao aplicada aos dois jogos: Nightfall1.2.0 ainda candidato e manutencao Rua de Aco0.4.1/code401, sem substituir tags0.4.0/1.1.1 publicadas. Novos gates/builds/publicacao pendentes. Responsavel: Codex.
