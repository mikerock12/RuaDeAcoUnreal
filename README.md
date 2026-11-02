# Rua de Aço — C++20 Native Cross-Platform & Unreal 5.8

Implementação nativa em **C++20** do **Rua de Aço** para **Windows x64** e **Android ARM64**, com núcleo determinístico de 60 Hz (`Source/RuaDeAcoRuntime`) em paridade canônica com a versão Web (`RuaDeAco`) e integração preparada para **Unreal Engine 5.8** (`Source/RuaDeAco`).

Este repositório é **independente** do jogo Phaser em [`mikerock12/RuaDeAco`](https://github.com/mikerock12/RuaDeAco).

| Projeto | Tecnologia | Papel |
| --- | --- | --- |
| [RuaDeAco](https://github.com/mikerock12/RuaDeAco) | Phaser 3 / TypeScript | Versão Web e exportador canônico (`golden_fixtures.json`) |
| **RuaDeAcoUnreal** | C++20 / CMake / raylib / Unreal 5.8 | Versão nativa **Windows x64** (`.exe` / `.zip`) e **Android ARM64** (`.apk`) |

Plano de implementação: [`IMPLEMENTACAO_RUA_DE_ACO_UNREAL.md`](IMPLEMENTACAO_RUA_DE_ACO_UNREAL.md).

## Versão 0.2.0 — Elenco Completo e Paridade 60 Hz

Jogável nativamente em Windows x64 e Android ARM64:

- **6 Lutadores completos** com stats, golpes normais, especiais, projéteis e Supers (50 de barra) canônicos:
  1. **Guto Barba** (HP 1100, Defesa 1.15x, *Ombro de Ferro*, *Quebra-Quilha*, Super * Marreta do Cais*)
  2. **Rafa Maré** (HP 980, Velocidade 225, *Chute Maré*, *Espuma Cortante*, Super *Turbilhão Atlântico*)
  3. **Noir Reflexo** (HP 950, Contra-golpe/Parry, *Lâmina Sombra*, *Passo Espelhado*, Super *Eclipse Total*)
  4. **Astro Riso** (HP 920, Mobilidade aérea, *Estrela Cadente*, *Giro Cósmico*, Super *Supernova de Circo*)
  5. **Dante Sinal** (HP 1000, Zoner elétrico, *Pulso de Linha*, *Torre de Alta Tensão*, Super *Curto-Circuito Geral*)
  6. **Leo Violeta** (HP 1050, Equilibrado, *Corte Violeta*, *Nota Dissonante*, Super *Sinfonia da Meia-Noite*)
- **3 Arenas com Stage Finishers** cinematográficos no round decisivo:
  - **Cais da Cidade** (*Maré Alta no Cais*)
  - **Cozinha Macabra** (*Caldeirão Fervente*)
  - **Sítio da Serra** (*Porteira da Serra*)
- **Mecânicas completas**: Agarrão Universal (`F+G` / `J+K`, janela de tech de 8 frames, arremesso de 125 de dano), Parry (redução de 85%), Armor/Super Armor, Hitstop e Pushback exatos.
- **Modos de Jogo**: Versus CPU (3 dificuldades de IA), Versus Local 2P, Modo Treino (Frame Data HUD, Hitbox/Hurtbox Viewer `F1`, Reset `F2`, Dummy CPU Toggle `F3`), Menu de Pausa com Move List (`M`) e Controles Touch (`F4` ou tela touch no Android).
- **Áudio**: Faixas `.ogg` Suno e SFX com fallback procedural em 44,1 kHz.

### Controles

| Ação | Jogador 1 | Jogador 2 | Gamepad |
| --- | --- | --- | --- |
| Mover | A / D | ← / → | Analógico / D-pad |
| Pular / Agachar | W / S | ↑ / ↓ | Analógico / D-pad |
| Soco | F | J | A / Cross |
| Chute | G | K | B / Circle |
| Agarrão Universal / Stage Finisher | F + G (ou T) | J + K (ou O) | X / Square |
| Especial | E | I | RB / R1 |
| Super (50 de barra) | Q | P | RT / R2 |
| Defesa / Parry | R | U | Y / Triangle |
| Pausa / Move List | Esc / M | Esc / M | Start |
| Hitboxes / Frame Data | F1 | F1 | Select |

## Build, Testes Golden (CTest) e Empacotamento

### 1. CMake + CTest (Windows MSVC Release)

```powershell
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
ctest --preset windows-msvc-release --output-on-failure
```

A suíte `RDAGoldenTests` valida **44 verificações canônicas** contra `Data/generated/golden_fixtures.json` (stats dos 6 lutadores, 3 arenas, agarrão universal, projéteis, parry, super, stage finisher, determinismo de 60 Hz e benchmark de 10.000 ticks headless em ~7,2 ms), além do teste `RDAHeadlessSmoke` (`RuaDeAcoUnreal.exe --headless-smoke`).

### 2. Pacote e Instalador Windows x64 (`.zip` e `-Setup.exe`)

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\Build-Windows-Installer.ps1 -Version "0.2.0"
```

Gera em `Saved\Packages\Release-0.2.0\`:
- `RuaDeAco-0.2.0-Windows-x64-Setup.exe` (Instalador Inno Setup 6)
- `RuaDeAco-0.2.0-Windows-x64.zip` (Pacote portátil)

### 3. Pacote Android ARM64 (`.apk`)

```powershell
powershell -ExecutionPolicy Bypass -File .\Scripts\Build-Android.ps1 -Version "0.2.0"
```

Compila `libruadeaco.so` (`arm64-v8a`, C++20, NDK 27), empacota assets em `assets/Content/RuaDeAco/`, alinha com `zipalign` e assina/verifica (`apksigner verify` v2/v3) gerando `Saved\Packages\Release-0.2.0\RuaDeAco-0.2.0-Android-arm64.apk`.

## Unreal Engine 5.8

O módulo `Source/RuaDeAco` integra o mesmo `Source/RuaDeAcoRuntime` ao `ARDAGameMode` e ao HUD 2D para uso com a Unreal Engine 5.8 (`D:\Epic\Launcher\UE_5.8`).

## Créditos

**Maicon Nunes** — [@mikerock12](https://github.com/mikerock12)

Jogo original (Phaser): [Rua de Aço](https://github.com/mikerock12/RuaDeAco)
