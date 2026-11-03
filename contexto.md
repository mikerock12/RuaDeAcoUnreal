# Contexto do Projeto — Rua de Aço Nativo (`contexto.md`)

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
