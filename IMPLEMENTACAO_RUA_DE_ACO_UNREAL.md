# Rua de Aço Unreal — Plano de Implementação

> **Status:** novo projeto, independente do Rua de Aço atual.
>
> **Objetivo:** criar uma nova implementação do **Rua de Aço** em **Unreal Engine 5.8 + C++**, tomando o jogo atual em Phaser como referência de gameplay, identidade, personagens, assets e regras, **sem substituir o projeto existente**. Local do projeto Rua de Aço Phaser: D:\PROJETOS\RuaDeAco

---

## 1. Princípio do projeto

O Rua de Aço passará a ter duas implementações coexistindo:

| Projeto | Tecnologia | Objetivo |
|---|---|---|
| **Rua de Aço atual** | Phaser / JavaScript | Versão Web, leve e executável no navegador |
| **Rua de Aço Unreal** | Unreal Engine 5.8 / C++ | Nova versão para Windows, posteriormente Android e outras plataformas |

### Regra fundamental

O projeto Unreal **não é uma migração destrutiva, atualização ou substituição do projeto Phaser**.

Ele será um **projeto novo**, criado em outro diretório e preferencialmente em outro repositório GitHub.

O projeto Phaser deverá continuar funcionando e evoluindo independentemente.

### Relação entre os dois projetos

A versão Unreal poderá reaproveitar:

- conceito do jogo;
- personagens;
- nomes;
- golpes;
- regras;
- sprites;
- sprite sheets;
- cenários;
- sons;
- músicas cuja licença permita reutilização;
- HUD e identidade visual;
- referências de gameplay;
- ideias e documentação do projeto atual.

Não deverá haver tentativa de converter automaticamente o código JavaScript/Phaser para C++.

As regras serão **reinterpretadas e reimplementadas nativamente em Unreal C++**.

---

## 2. Diretórios definidos

O disco principal de desenvolvimento será o **D:**.

### Projeto novo

```text
D:\PROJETOS\RuaDeAcoUnreal
```

### Projeto Phaser existente

Permanece no local atual.

O instalador deste plano **não move, renomeia nem modifica** o projeto Phaser.

### Ferramentas

```text
D:\
├── Dev\
│   ├── Installers\
│   ├── VSCache\
│   └── VisualStudio2022\
│
├── Epic\
│   ├── Launcher\
│   └── UE_5.8\
│
└── Projetos\
    └── RuaDeAco-Unreal\
```

### Caminhos importantes para Codex/SSH

Depois da instalação completa, considerar os seguintes caminhos como padrão:

```text
Projeto Unreal
D:\Projetos\RuaDeAco-Unreal

Unreal Engine
D:\Epic\UE_5.8

Unreal Editor
D:\Epic\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe

Unreal Build Tool / Build script
D:\Epic\UE_5.8\Engine\Build\BatchFiles\Build.bat

Unreal Automation Tool
D:\Epic\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat

Visual Studio 2022
D:\Dev\VisualStudio2022

Visual Studio IDE
D:\Dev\VisualStudio2022\Common7\IDE\devenv.exe

MSBuild
D:\Dev\VisualStudio2022\MSBuild\Current\Bin\MSBuild.exe

```

### Componentes que podem permanecer no C:

Nem tudo no ecossistema Windows pode ou deve ser movido para o D:.

Exemplos:

```text
C:\Windows\System32\OpenSSH
C:\ProgramData
C:\Users\<usuario>\AppData
```

O Visual Studio também pode manter pequenos componentes compartilhados e metadados no sistema.

A prioridade de espaço é colocar no **D:** os componentes realmente pesados:

- Unreal Engine;
- Visual Studio;
- cache do instalador do Visual Studio;
- repositório;
- builds;
- assets;
- intermediários;
- pacotes;
- Git.

---

## 3. Variáveis de ambiente previstas

O script de preparação cria:

```text
DEV_ROOT=D:\Dev
UE_ROOT=D:\Epic\UE_5.8
RDA_UNREAL_ROOT=D:\Projetos\RuaDeAco-Unreal
```

Essas variáveis facilitam automações e uso pelo Codex.

Exemplo:

```powershell
cd $env:RDA_UNREAL_ROOT
```

---

## 4. Repositório Git

Recomendação de nome:

```text
RuaDeAcoUnreal
```

O repositório deve ser separado do projeto Phaser.

Estratégia:

```text
RuaDeAco            -> Phaser / Web
RuaDeAcoUnreal     -> Unreal Engine / C++
```

Não usar o repositório atual como branch permanente da versão Unreal.

São produtos irmãos, não duas branches da mesma implementação.

### Git LFS

Usar Git LFS para arquivos binários pesados, principalmente:

```text
*.uasset
*.umap
*.wav
*.mp3
*.ogg
*.png
*.psd
*.fbx
*.blend
```

A lista poderá ser refinada conforme o projeto crescer.

---

## 5. Stack principal

### Engine

```text
Unreal Engine 5.8
```

### Linguagem

```text
C++20 / padrão suportado pela toolchain da Unreal
```

### IDE/toolchain

```text
Visual Studio 2022 Community 17.14+
MSVC
Windows SDK 10.0.26100
```

A Unreal Engine 5.8 também suporta Visual Studio mais novo, mas este projeto padroniza inicialmente em **Visual Studio 2022 17.14+**, que continua suportado pela UE 5.8 e é uma base estável para automação.

### Sistemas Unreal previstos

- Paper 2D;
- Enhanced Input;
- UMG;
- Unreal C++;
- Components;
- GameMode / GameState / PlayerState;
- Data Assets;
- Animation/Flipbooks;
- Collision;
- Audio;
- SaveGame;
- Unreal Automation Tool;
- Unreal Build Tool.

---

## 6. Estrutura inicial do repositório

Estrutura proposta:

```text
RuaDeAco-Unreal/
│
├── .gitattributes
├── .gitignore
├── README.md
├── RuaDeAco.uproject
│
├── Config/
│   ├── DefaultEngine.ini
│   ├── DefaultGame.ini
│   └── DefaultInput.ini
│
├── Content/
│   └── RuaDeAco/
│       ├── Art/
│       │   ├── Characters/
│       │   │   ├── GutoBarba/
│       │   │   └── RafaMare/
│       │   ├── Environments/
│       │   ├── Effects/
│       │   └── UI/
│       │
│       ├── Audio/
│       │   ├── Music/
│       │   └── SFX/
│       │
│       ├── Data/
│       ├── Input/
│       ├── Maps/
│       ├── Materials/
│       └── Widgets/
│
├── Source/
│   ├── RuaDeAco/
│   │   ├── RuaDeAco.Build.cs
│   │   ├── RuaDeAco.cpp
│   │   ├── RuaDeAco.h
│   │   │
│   │   ├── Core/
│   │   ├── Characters/
│   │   ├── Combat/
│   │   ├── Components/
│   │   ├── Game/
│   │   ├── Input/
│   │   ├── UI/
│   │   ├── World/
│   │   └── Data/
│   │
│   └── RuaDeAcoEditor/
│
├── Docs/
│   ├── ARCHITECTURE.md
│   ├── COMBAT.md
│   ├── ASSET_MIGRATION.md
│   └── NETWORKING.md
│
├── Scripts/
│   ├── Build-Windows.ps1
│   ├── Package-Windows.ps1
│   └── Validate-Project.ps1
│
└── ExternalAssets/
    └── README.md
```

Pastas geradas pela Unreal como `Binaries`, `DerivedDataCache`, `Intermediate` e `Saved` não devem ser versionadas.

---

## 7. Arquitetura C++ inicial

### Personagem base

```text
ARDAFighterCharacter : APaperCharacter
```

Responsabilidades:

- movimentação;
- orientação;
- estado do lutador;
- animação;
- interface com combate;
- input;
- knockback;
- stun;
- morte/KO.

### Componentes

```text
URDAHealthComponent
URDACombatComponent
URDAHitboxComponent
URDAHurtboxComponent
URDAMovementStateComponent
```

### Gameplay

```text
ARDAGameMode
ARDAGameState
ARDAPlayerState
ARDAPlayerController
```

### Interface

```text
URDAHUDWidget
URDAHealthBarWidget
URDARoundWidget
URDAMobileControlsWidget
```

### Dados

Ataques e personagens devem migrar gradualmente para estruturas orientadas a dados:

```text
FRDAAttackData
FRDAFighterStats
URDAAttackDataAsset
URDAFighterDataAsset
```

Isso evita hardcode excessivo.

---

## 8. Sistema de combate

Separar três conceitos:

### Hurtbox

Área em que o personagem pode receber dano.

### Hitbox

Área ativa de um ataque.

### Pushbox

Área usada para impedir que dois personagens ocupem fisicamente o mesmo espaço.

Exemplo conceitual:

```text
FRAME 1-2
startup

FRAME 3-4
hitbox ativa

FRAME 5+
recovery
```

Cada ataque deverá possuir dados como:

```text
Damage
StartupFrames
ActiveFrames
RecoveryFrames
HitStun
BlockStun
Knockback
Hitbox
AttackType
CanCancel
```

---

## 9. Personagens iniciais

Primeira prova de conceito:

```text
Guto Barba
Rafa Maré
```

Não adicionar novos lutadores antes de a arquitetura dos dois primeiros estar validada.

---

## 10. Primeiro vertical slice

A primeira versão jogável deverá conter somente:

- 1 cenário;
- Guto Barba;
- Rafa Maré;
- movimentação lateral;
- pulo;
- agachamento;
- soco;
- chute;
- hitbox;
- hurtbox;
- pushbox;
- dano;
- barra de vida;
- KO;
- reinício do round;
- teclado;
- gamepad.

O objetivo é validar arquitetura, sensação dos controles e build.

---

## 11. Input

Utilizar:

```text
Enhanced Input
```

Actions previstas:

```text
IA_Move
IA_Jump
IA_Crouch
IA_Punch
IA_Kick
IA_Block
IA_Grab
IA_Special
IA_Pause
```

Mobile será adicionado somente após o controle tradicional estar estável.

---

## 12. HUD

Primeira versão:

```text
Vida P1
Vida P2
Nome P1
Nome P2
Timer
Round
Indicador de KO
```

O HUD deve ser criado com UMG e receber estado do gameplay, evitando lógica de combate diretamente nos Widgets.

---

## 13. Assets do projeto Phaser

Não copiar todos os assets de uma vez.

Criar uma migração controlada.

Fluxo:

```text
asset original
↓
verificar necessidade/licença
↓
copiar para staging
↓
ajustar resolução/pivô/transparência
↓
importar na Unreal
↓
criar Sprite/Flipbook
↓
validar
```

Registrar futuramente no:

```text
Docs/ASSET_MIGRATION.md
```

---

## 14. Estratégia de renderização

Primeira implementação:

```text
Paper 2D
```

Possível evolução:

```text
personagens 2D
+
cenários 2.5D/3D
+
iluminação
+
partículas
+
efeitos
+
câmera cinematográfica
```

A identidade visual original não deve ser sacrificada apenas para usar recursos 3D.

---

## 15. Multiplayer

### Fase inicial

```text
Local 1x1
```

### Fase posterior

```text
Online 1x1
```

A arquitetura deve evitar decisões que impeçam replication, mas **não implementar multiplayer online durante o primeiro vertical slice**.

Etapas posteriores:

- authoritative server;
- replication;
- prediction;
- sincronização dos estados;
- matchmaking/lobby;
- avaliação de rollback netcode.

Rollback deve ser tratado como projeto específico, não como simples detalhe de replication.

---

## 16. Plataformas

### Prioridade 1

```text
Windows x64
```

### Prioridade 2

```text
Android
```

Android SDK, NDK, Java e ferramentas móveis **não fazem parte da instalação inicial deste script**.

Eles serão instalados quando o vertical slice Windows estiver funcionando.

### Web

A versão Web continuará sendo o projeto Phaser.

A versão Unreal não deverá substituir o Phaser como build de navegador.

---

## 17. Build automatizado

Objetivo para o Codex:

```text
editar
↓
compilar
↓
ler erros
↓
corrigir
↓
compilar novamente
↓
executar testes
```

Comandos futuros deverão utilizar preferencialmente os scripts oficiais da Unreal.

Exemplo conceitual:

```powershell
& "$env:UE_ROOT\Engine\Build\BatchFiles\Build.bat" `
    RuaDeAcoEditor Win64 Development `
    "$env:RDA_UNREAL_ROOT\RuaDeAco.uproject" `
    -WaitMutex
```

Empacotamento futuro:

```text
RunUAT.bat
BuildCookRun
```

---

## 18. Codex via SSH

O PC de desenvolvimento será o host.

O Codex deverá trabalhar prioritariamente em:

```text
D:\Projetos\RuaDeAco-Unreal
```

O host deverá possuir:

- Unreal;
- MSVC;
- Windows SDK;
- Visual Studio;
- Git;
- Git LFS;
- OpenSSH Server.

O SSH não substitui o Unreal Editor para atividades visuais, mas deverá permitir ao agente:

- editar C++;
- alterar arquivos `.ini`;
- manipular scripts;
- trabalhar com Git;
- criar classes;
- compilar;
- executar testes;
- ler logs;
- corrigir erros;
- empacotar builds;
- analisar o projeto.

---

## 19. Fases de implementação

### Fase 0 — Ambiente

- executar `setup_rua_de_aco_unreal.ps1`;
- confirmar Visual Studio;
- confirmar Git/Git LFS;
- confirmar OpenSSH;
- instalar Epic Games Launcher;
- instalar manualmente Unreal Engine 5.8 em `D:\Epic\UE_5.8`.

### Fase 1 — Bootstrap

- criar projeto C++;
- habilitar Paper 2D;
- habilitar Enhanced Input;
- configurar Git;
- configurar Git LFS;
- gerar primeiro build;
- publicar novo repositório.

### Fase 2 — Personagem base

- criar `ARDAFighterCharacter`;
- movimentação;
- pulo;
- orientação;
- estados;
- câmera.

### Fase 3 — Combate base

- health;
- hitbox;
- hurtbox;
- pushbox;
- soco;
- chute;
- hit stun;
- knockback;
- KO.

### Fase 4 — Guto vs Rafa

- importar sprites;
- criar Flipbooks;
- configurar golpes;
- HUD;
- round;
- cenário;
- áudio básico.

### Fase 5 — Qualidade de gameplay

- input buffering;
- cancel windows;
- combos;
- agarrão;
- defesa;
- efeitos;
- câmera;
- feedback de impacto.

### Fase 6 — Build Windows

- Development build;
- Shipping build;
- empacotamento;
- validação em máquina limpa.

### Fase 7 — Android

Somente aqui instalar:

- Android SDK;
- NDK;
- Java/OpenJDK;
- ferramentas exigidas pela versão da Unreal.

Depois:

- touch controls;
- escala de UI;
- desempenho;
- APK/AAB.

### Fase 8 — Multiplayer

- replication;
- sessão;
- host/client;
- sincronização;
- latência;
- avaliar rollback.

---

## 20. Critério para considerar o protótipo 0.1 pronto

```text
[ ] Abre na Unreal 5.8
[ ] Compila por linha de comando
[ ] Compila via Codex/SSH
[ ] Guto é controlável
[ ] Rafa é controlável
[ ] Dois jogadores locais funcionam
[ ] Soco causa dano
[ ] Chute causa dano
[ ] Hitbox/hurtbox funcionam
[ ] Barra de vida funciona
[ ] Existe KO
[ ] Round reinicia
[ ] Build Windows executável é gerado
[ ] Projeto Phaser permanece intacto
```

---

## 21. Instalação da Unreal Engine

A Unreal Engine não será instalada silenciosamente pelo script.

Motivos:

- o Epic Games Launcher exige autenticação;
- a versão/componentes são escolhidos no Launcher;
- instalações automatizadas não devem depender de comportamento não documentado.

Procedimento:

1. executar o script de preparação;
2. abrir o Epic Games Launcher;
3. entrar na conta Epic;
4. acessar **Unreal Engine > Library**;
5. adicionar **Unreal Engine 5.8**;
6. escolher como destino:

```text
D:\Epic\UE_5.8
```

7. concluir a instalação;
8. validar:

```text
D:\Epic\UE_5.8\Engine\Binaries\Win64\UnrealEditor.exe
```

---

## 22. Referências oficiais

Unreal Engine 5.8 Release Notes:

https://dev.epicgames.com/documentation/unreal-engine/unreal-engine-5-8-release-notes

Visual Studio para Unreal Engine:

https://dev.epicgames.com/documentation/unreal-engine/setting-up-visual-studio-development-environment-for-cplusplus-projects-in-unreal-engine

Instalação da Unreal Engine:

https://dev.epicgames.com/documentation/unreal-engine/install-unreal-engine

Visual Studio — instalação por linha de comando:

https://learn.microsoft.com/visualstudio/install/use-command-line-parameters-to-install-visual-studio

OpenSSH Server no Windows:

https://learn.microsoft.com/windows-server/administration/openssh/openssh_install_firstuse

---

## 23. Decisão arquitetural final

```text
Rua de Aço Phaser
    └── continua existindo
        └── foco: Web

Rua de Aço Unreal
    └── projeto novo
        ├── Unreal Engine 5.8
        ├── C++
        ├── Paper 2D
        ├── Windows
        ├── Android posteriormente
        └── multiplayer posteriormente
```

Nenhum trabalho na versão Unreal deve exigir destruir, converter ou abandonar a versão Phaser.
