# Contexto do Projeto — Rua de Aço Nativo (`contexto.md`)

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
