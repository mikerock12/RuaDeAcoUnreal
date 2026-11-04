# Rua de Aço Nativo (C++20 / raylib / Unreal) — Guia de Agentes (`AGENTS.md`)

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

Leia `README.md`, `IMPLEMENTACAO_RUA_DE_ACO_UNREAL.md` e `contexto.md` antes de alterar qualquer arquivo deste repositório (`mikerock12/RuaDeAcoUnreal`).

## Regras Inegociáveis

1. **Fonte canônica de regras e balanceamento**: O repositório `mikerock12/RuaDeAco` (TypeScript/Phaser) é a fonte canônica de gameplay, frame data, hitboxes/hurtboxes, velocidades, pulo, gravidade, agarrão universal e stage finishers.
2. **Núcleo C++20 desacoplado (`Source/RuaDeAcoRuntime/`)**:
   - Deve compilar sem depender de cabeçalhos do Unreal Engine (`CoreMinimal.h`) nem de `raylib.h`, permitindo execução headless determinística em 60 Hz (`Dt = 1/60`), testes automatizados (`CTest`) e reutilização tanto pelo executável nativo (`Source/Prototype/` com raylib) quanto pelo módulo Unreal (`Source/RuaDeAco/`).
3. **Sem comandos Git destrutivos**:
   - Proibido usar `git reset --hard`, `git clean -fdx`, `git restore .`, `git checkout -- .` ou `git push --force`.
4. **Autoria limpa**:
   - Commits e PRs nunca devem conter `Co-Authored-By` de IA nem assinaturas equivalentes.
5. **Segredos e binários**:
   - Nunca versionar chaves Android (`*.keystore`, `*.jks`), senhas ou tokens no Git.
6. **Documentação obrigatória por etapa**:
   - Ao final de cada etapa concluída, atualizar obrigatoriamente `AGENTS.md` e `contexto.md` com o que foi feito, verificações executadas, pendências e responsável.

## Comandos Operacionais

- Build legado / script rápido Windows:
  - `powershell -ExecutionPolicy Bypass -File .\Scripts\Build-Windows.ps1 -Configuration Release`
- Build e testes CMake (a partir da Fase G2):
  - `cmake --preset windows-msvc-release`
  - `cmake --build --preset windows-msvc-release`
  - `ctest --preset windows-msvc-release --output-on-failure`

## Registro de Etapas

- **2026-11-01 / Fase G0 (Inventário e Baseline P0)**: Criados `AGENTS.md` e `contexto.md`. Base `main` verificada em `a5b40ff8afd65900c59008d8a0a75b60da0d3a4b` (tag `v0.1.0`). Compilação baseline `Scripts\Build-Windows.ps1 -Configuration Release` executada com sucesso no MSVC 2022 (`v17.14.44`), gerando `Saved\Packages\Windows\RuaDeAcoUnreal.exe`. Diagnosticada defasagem do runtime C++ atual (apenas 4 lutadores, 1 arena, sem CMake/CTest, sem agarrão universal e com caminhos absolutos legados `D:\PROJETOS\RuaDeAco`). Próxima etapa: Fase G2 (CMake + núcleo C++20 headless + runner de golden replays da Fase G1). Responsável: Coordenador SLT Games.

- **2026-11-01 / Fase G1 (Fixtures e Replays Golden de `RuaDeAco` Concluída)**: Exportado `docs/parity/golden_fixtures.json` no repositório canônico `mikerock12/RuaDeAco` (PR #9 mesclado em `b41dd21` em `master` e `web-beta`, 60/60 arquivos e 591/591 testes cliente + 71/71 servidor aprovados). Iniciando Fase G2 neste repositório (`RuaDeAcoUnreal`) para portar o catálogo dos 6 lutadores, 3 arenas, agarrão universal, CMake e CTest consumindo o `golden_fixtures.json`. Responsável: Coordenador SLT Games.

- **2026-11-01 / Fases G2 e G3 (CMake, CTest, 6 Lutadores, 3 Arenas, Agarrão Universal, Stage Finishers e Áudio Suno)**: Implementados `CMakeLists.txt`, `CMakePresets.json` e `Tests/RDAGoldenTests.cpp`. Expandido `Source/RuaDeAcoRuntime/` e `Source/Prototype/PrototypeMain.cpp` para suportar os 6 lutadores (`GutoBarba`, `RafaMare`, `NoirReflexo`, `AstroRiso`, `DanteSinal`, `LeoVioleta`), as 3 arenas (`CaisDaCidade`, `CozinhaMacabra`, `Sitio`), agarrão universal (`F+G` / `J+K`), projéteis, especiais, supers (`50` de barra), finalizações de palco nas 3 arenas, áudio `.ogg` da versão web, modos Vs CPU (3 dificuldades), Versus Local e Treinamento (frame data + hitboxes), menu de pausa com lista de golpes e controles touch na tela. Sincronizados sprites v4, retratos, palcos e áudios `.ogg` em `Content/RuaDeAco/` e `Data/generated/golden_fixtures.json`. CTest 2/2 suítes aprovadas (`RDAGoldenTests`: 44/44 checks OK, benchmark 10.000 ticks em 7,26 ms = 0,00073 ms/tick; `RDAHeadlessSmoke`: OK). Próxima etapa: Fase G4 (empacotamento Windows `.exe` Inno Setup + `.zip` e Android ARM64 `.apk`). Responsável: Coordenador SLT Games.

- **2026-11-01 / Fase G4 (Empacotamento Windows x64 + Android ARM64 Concluído)**: Criados `Platform/Windows/RuaDeAco.iss`, `Scripts/Build-Windows-Installer.ps1`, `Platform/Android/AndroidManifest.xml`, `Platform/Android/res/values/strings.xml`, `Platform/Android/src/com/smellsliketech/ruadeaco/MainActivity.java`, `Platform/Android/jni/AndroidBridge.cpp`, `Scripts/Build-Android.ps1` e `.github/workflows/cpp-ci.yml`. Gerados e verificados os três artefatos oficiais v0.2.0 em `Saved/Packages/Release-0.2.0/`:
  - `RuaDeAco-0.2.0-Windows-x64-Setup.exe`: `52.827.371` bytes, SHA-256 `e46115c6ed4064bf375fa11b4f679104846d2c327e7790b09de996f9e1b21ddd`
  - `RuaDeAco-0.2.0-Windows-x64.zip`: `50.990.468` bytes, SHA-256 `3ade0f795dec70ad8710a8425f0d6143e05c86efab7662be7a2a464e6b51a7c2`
  - `RuaDeAco-0.2.0-Android-arm64.apk`: `38.670.466` bytes, SHA-256 `6b379f441d196268a25301581ee413dd107656af95f82c2558574cef688912a5` (`apksigner verify` v2/v3 aprovado)
  Próxima etapa: PR/merge em `mikerock12/RuaDeAcoUnreal` e início da Fase G5 (`MikeAndDenyse-Nightfall`). Responsável: Coordenador SLT Games.

- **2026-11-01 / Fases G5 e G6 Concluídas no Ecossistema SLT Games**: PR #1 deste repositório (`RuaDeAcoUnreal`) mesclado em `main` (`9dc9232`), e `MikeAndDenyse-Nightfall` concluiu `NightfallNative` C++20 + raylib (`77/77` checks + UDP LAN loopback) e empacotamento v1.0.0. Próxima etapa: Fase G7 (publicação da Release `v0.2.0` neste repositório e `v1.0.0` em `MikeAndDenyse-Nightfall`). Responsável: Coordenador SLT Games.
- **2026-11-01 / Fase G7 Concluída**: Publicada a Release GitHub estável `v0.2.0` (`https://github.com/mikerock12/RuaDeAcoUnreal/releases/tag/v0.2.0`) com `RuaDeAco-0.2.0-Windows-x64-Setup.exe` (`52.827.371 B`, SHA-256 `e46115c6...21ddd`), `RuaDeAco-0.2.0-Windows-x64.zip` (`50.990.468 B`, SHA-256 `3ade0f79...1a7c2`), `RuaDeAco-0.2.0-Android-arm64.apk` (`38.670.466 B`, SHA-256 `6b379f44...912a5`) e `SHA256SUMS.txt`. Próxima etapa: Fase G8 (integração de `/games/` no portal). Responsável: Coordenador SLT Games.
- **2026-11-01 / Fase G8 Concluída**: Página `/games/rua-de-aco/` publicada no portal `https://softwares.smellsliketech.com.br/games/rua-de-aco/` (PR #38, merge `5fe367e`, Worker `f24ab0eb-d181-435b-b8ce-5764468138c8`) com API `/api/games/releases/rua-de-aco-*` (`200`) e redirects `/download/games/rua-de-aco-*` (`302`) validados em produção. Próxima etapa: Fase G9 (Backups Google Drive). Responsável: Coordenador SLT Games.


## Ícones Windows — 03/11/2026

Preparada versão 0.3.1. Logo original Rua de Aço preservado, com origem e SHA-256 registrados. ICO com dez tamanhos (16–256 px) incorporado ao jogo e instalador; PNG incorporado ao PE para SetWindowIcon, atalhos e desinstalação apontam ao EXE. CMake/MSVC e Inno Setup compilam os recursos; o script direto do Rua de Aço também. Verificação PE dos quatro executáveis comparou todas as imagens ao ICO aprovado; as duas janelas reais devolveram ícone 256×256 e logs de aplicação. Instalador RDA: 342 arquivos iguais ao ZIP, headless, 29 capturas e desinstalação aprovados. Nightfall: instalador compilado e ícone extraído; execução portátil com ícone conferida. Instalação Nightfall 1.0.1 já existe em Program Files e foi preservada, impedindo repetir o teste de instalação neste host.

APKs recebem apenas versão de manutenção, com a mesma assinatura v2/v3 anterior e alinhamento 16 KiB; gameplay permanece o da correção anterior. PR/CI, releases, portal e backup desta etapa serão consolidados pelo coordenador no contexto do portal. Testes físicos Android continuam pendentes. Responsável: Codex.


## Atualizacao de plataforma 0.4.0 — em validacao, 03/11/2026

Configuracoes persistentes de tela, qualidade, audio, teclado/controle e touch integradas ao frontend compartilhado. Android tem HUD sempre visivel, tamanho/opacidade/margem/canhoto e vibracao; icones tradicionais e adaptativos derivam da mesma arte Windows aprovada, com procedencia em Platform/Android/Branding (NightfallNative no Nightfall). Windows oferece resolucao, modos de janela, VSync, limites FPS e render interno ate 4K. Nightfall usa subsistema GUI e simulacao fixa 60Hz. Rua de Aco abre o cliente Web oficial integrado para compatibilidade de salas; combate offline continua C++.

Candidatos locais compilaram para Windows e Android x86_64; CTest anterior 4/4 RDA e 3/3 Nightfall. QA ampliado exige configuracoes por toque e persistencia apos reiniciar; CI, online real, empacotamento final, novas capturas, releases, portal e backups ainda pendentes. Suno fica por ultimo por instrucao do usuario, indisponivel por verificacao do site; nenhum audio novo gerado. Sem homologacao Android fisica. Responsavel: Codex.


### Configuracoes e icones APK — 03/11/2026, em validacao
Configuracoes de plataforma e touch persistentes, icones Android da arte Windows aprovada, pausa de audio no background e executavel Windows GUI implementados. Simulacao de frontend desacoplada do FPS; P2 aceita segundo gamepad. QA passou a conferir 15 recursos PNG compilados por jogo, dois XML adaptativos, preferencias apos reinicio e gestos reais no emulador. Primeiro CI encontrou falta de carregamento explicito da biblioteca para o JNI de lifecycle no Nightfall e seletor de borda obsoleto no driver RDA; corrigidos, gates serao repetidos. Nao ha nova release ainda. Audio Suno adiado para o fim pelo usuario, sem clips gerados; nao expor ajustes sem audio correspondente. Sem Android fisico/dupla Wi-Fi. Responsavel: Codex.
