# Rua de Aço Unreal C++

Nova implementação do **Rua de Aço** em **C++**, com arquitetura preparada para **Unreal Engine 5.8** (Paper 2D, Enhanced Input, UMG).

Este repositório é **independente** do jogo Phaser em `D:\PROJETOS\RuaDeAco`. Nada neste projeto altera, substitui ou converte o código JavaScript original.

| Projeto | Tecnologia | Papel |
| --- | --- | --- |
| [RuaDeAco](https://github.com/mikerock12/RuaDeAco) | Phaser / TypeScript | Versão Web |
| **RuaDeAcoUnreal** | C++ / Unreal 5.8 | Versão Windows (Android depois) |

Plano de implementação: [`IMPLEMENTACAO_RUA_DE_ACO_UNREAL.md`](IMPLEMENTACAO_RUA_DE_ACO_UNREAL.md).

## Protótipo 0.1 (vertical slice)

Jogável agora, **sem Unreal Editor**:

- 1 cenário: **Cais da Cidade**
- **Guto Barba** (P1) vs **Rafa Maré** (P2)
- Andar, pular, agachar, soco, chute, defesa
- Hitbox, hurtbox, pushbox, dano, barra de vida
- KO, melhor de 3 rounds, reinício
- Teclado e gamepad
- Executável Windows x64

### Controles

| Ação | Jogador 1 | Jogador 2 | Gamepad |
| --- | --- | --- | --- |
| Mover | A / D | ← / → | analógico / D-pad |
| Pular / agachar | W / S | ↑ / ↓ | analógico / D-pad |
| Soco | F | J | A |
| Chute | G | K | B |
| Defesa | R | U | Y |
| Pausa | Esc | Esc | Start |
| Hitboxes | F1 | F1 | — |

### Como gerar o .exe

Requisitos: Visual Studio 2022 com workload **Game Development with C++** (MSVC + Windows SDK 10.0.26100).

```powershell
cd D:\PROJETOS\RuaDeAcoUnreal
powershell -ExecutionPolicy Bypass -File .\Scripts\Build-Windows.ps1
Start-Process .\Saved\Packages\Windows\RuaDeAcoUnreal.exe
```

O pacote de teste fica em `Saved\Packages\Windows\` (exe + `Content` + `raylib.dll`).

## Unreal Engine 5.8

A Unreal Engine **5.8.2** está em `D:\Epic\Launcher\UE_5.8` (o plano citava `D:\Epic\UE_5.8`; o Launcher instalou neste caminho). O módulo `Source/RuaDeAco` já segue o plano (PaperCharacter, componentes, GameMode, Enhanced Input, Data Assets).

Para abrir o editor:

1. Abrir `RuaDeAco.uproject` (ou gerar project files).
2. Compilar o editor:

```powershell
$env:UE_ROOT = "D:\Epic\Launcher\UE_5.8"
powershell -ExecutionPolicy Bypass -File .\Scripts\Build-UnrealEditor.ps1
& "$env:UE_ROOT\Engine\Binaries\Win64\UnrealEditor.exe" "D:\PROJETOS\RuaDeAcoUnreal\RuaDeAco.uproject"
```

A simulação de combate do protótipo vive em `Source/RuaDeAcoRuntime` e deve ser reutilizada pelos actors Unreal, em vez de reescrever as regras.

## Princípios

- Não converter Phaser → C++ automaticamente. As regras foram **reimplementadas**.
- Assets do Phaser entram só por migração controlada (Guto, Rafa, Cais neste slice).
- Pixel art em 640×360, escala inteira, nearest-neighbor.
- Local 1x1 agora; online só em fase posterior.
- O projeto Phaser continua a ser a versão Web.

## Layout

```
RuaDeAcoUnreal/
  RuaDeAco.uproject
  Source/RuaDeAco/            módulo Unreal C++
  Source/RuaDeAcoRuntime/     simulação de combate portável
  Source/Prototype/           alvo Win64 jogável (raylib)
  Content/RuaDeAco/           arte do vertical slice
  Scripts/Build-Windows.ps1
```

## Créditos

**Maicon Nunes** — [@mikerock12](https://github.com/mikerock12)

Jogo original (Phaser): [Rua de Aço](https://github.com/mikerock12/RuaDeAco)
