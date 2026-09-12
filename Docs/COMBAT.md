# Combate — vertical slice 0.1

Frame data e caixas calibradas a partir do Rua de Aço Phaser (reimplementadas em C++).

## Guto Barba (tanque)

| Golpe | Botão | Frames | Ativo | Dano | Nível |
| --- | --- | --- | --- | --- | --- |
| Cotovelada | Soco | 20 | 6–9 | 66 | mid |
| Chute frontal | Chute | 29 | 10–14 | 82 | low |
| Rasteira do Urso | Agachar + chute | 36 | 14–18 | 118 | low, knockdown |
| Soco aéreo | Soco no ar | 24 | 6–15 | 72 | overhead |
| Chute aéreo | Chute no ar | 28 | 8–17 | 120 | overhead |

Vida 1250, andada 1.75, pulo 12.8, peso 1.28.

## Rafa Maré (rushdown)

| Golpe | Botão | Frames | Ativo | Dano | Nível |
| --- | --- | --- | --- | --- | --- |
| Soco rápido | Soco | 15 | 4–6 | 45 | high |
| Chute alto | Chute | 32 | 12–16 | 102 | overhead |
| Chute baixo | Agachar + soco | 23 | 7–10 | 58 | low |
| Rasteira | Agachar + chute | 31 | 11–15 | 92 | low, knockdown |
| Soco aéreo | Soco no ar | 22 | 5–14 | 58 | overhead |
| Chute aéreo | Chute no ar | 26 | 7–16 | 96 | overhead |

Vida 1000, andada 3.05, pulo 15.6, peso 0.9.

## Regras do slice

- Origem lógica nos pés. Tempos e caixas não seguem o tamanho do PNG.
- Overhead não é bloqueado agachado; low não é bloqueado em pé.
- Pushbox separa os corpos proporcionalmente ao peso.
- Round 99 s, melhor de 3 (`RoundsToWin = 2`).
- Sem especiais, agarrão, projéteis, meter ou CPU neste slice.
