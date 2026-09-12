# Migração de assets (slice 0.1)

Fonte (somente leitura): `D:\PROJETOS\RuaDeAco\public\assets`

Fluxo:

```
asset original
→ verificar necessidade
→ copiar para Content/RuaDeAco/...
→ validar no prototipo
```

Script: `Scripts/Copy-PhaserAssets.ps1` (copia; nunca escreve no Phaser).

## Incluído neste slice

| Destino | Origem |
| --- | --- |
| `Art/Characters/GutoBarba/*.png` | `fighters/guto-barba/` (idle, walk, jump, crouch, golpes, hit, KO, vitória, defesa) |
| `Art/Characters/RafaMare/*.png` | `fighters/rafa-mare/` (mesmo conjunto) |
| `Art/Environments/CaisDaCidade/background.png` | `stages/cais-da-cidade/remaster/background.png` |
| `Art/UI/hud-frame.png` | `ui/hud-frame.png` |
| `Fonts/PressStart2P-Regular.ttf` | `fonts/` (SIL Open Font License) |
| `Audio/Music/cais-da-cidade.ogg` | `audio/music/cais-da-cidade.ogg` |

## Não copiado de propósito

- Demais lutadores
- Cozinha Macabra e Sítio
- Especiais / agarrões / efeitos
- Retratos conceituais (não são sprites de luta)

Guto usa frames 288×288. Rafa usa 256×256. Origem visual: centro-base + 6 px de sola opaca.
