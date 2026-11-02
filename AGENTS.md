# Rua de Aço Nativo (C++20 / raylib / Unreal) — Guia de Agentes (`AGENTS.md`)

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
