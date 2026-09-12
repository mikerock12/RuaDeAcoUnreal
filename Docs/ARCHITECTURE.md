# Arquitetura — Rua de Aço Unreal

O combate é uma simulação determinística a 60 Hz, separada da apresentação.

```
Input (teclado / gamepad / Enhanced Input)
        │
        ▼
RuaDeAcoRuntime (C++ portável)
  Fighter + CombatWorld
        │
        ├── Prototype Win64 (raylib)     ← jogável no 0.1
        └── Actors Unreal (Paper 2D)     ← quando UE 5.8 estiver instalada
```

## Módulo Unreal (`Source/RuaDeAco`)

| Classe | Papel |
| --- | --- |
| `ARDAFighterCharacter` | PaperCharacter: movimento, orientação, estados |
| `URDAHealthComponent` | Vida / KO |
| `URDACombatComponent` | Golpe ativo |
| `URDAHitboxComponent` | Área ofensiva |
| `URDAHurtboxComponent` | Área que recebe dano |
| `URDAMovementStateComponent` | Idle / walk / jump / crouch |
| `ARDAGameMode` / `ARDAGameState` / `ARDAPlayerState` | Round, timer, vitórias |
| `ARDAPlayerController` | Enhanced Input |
| `URDAHUDWidget` | Vida, nomes, timer, round, KO |
| `FRDAAttackData` / `FRDAFighterStats` | Dados, não constantes espalhadas |

## Runtime portável (`Source/RuaDeAcoRuntime`)

Reinterpretação das regras do Phaser, sem traduzir JavaScript:

- espaço 640×360, chão Y=304, bordas 36–604
- pushbox por peso
- hitbox vs hurtbox AABB
- hit stop, hit stun, knockback, knockdown
- melhor de 3, intro FIGHT, KO, reinício do round

Personagens iniciais: **Guto Barba** e **Rafa Maré**.
