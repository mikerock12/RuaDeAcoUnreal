# Replays canônicos executados pelo core C++

Revisão: 02/11/2026. Responsável: Codex / agente de paridade.

Os testes antigos conferiam strings no catálogo e cenários próprios do port.
Isso não comprovava paridade. O novo runner executa as entradas exportadas pelo
`CombatWorld` TypeScript original e confronta seus checkpoints, um passo por vez.

## Fonte e reprodução

Referência de gameplay: `mikerock12/RuaDeAco`, `master`/`web-beta`
`0a15430` (PR #10 integrado; regras baseadas em
`62b56c909d23cce54613d547b1f9837f164347bc`). O exportador expandido não altera as
regras do jogo web. O arquivo de origem esperado pelo gerador fica em
`../RuaDeAco/docs/parity/golden_fixtures.json`.

```powershell
# Em RuaDeAco, gerar o fixture executando o motor original:
npx vitest run src/combat/__tests__/parityExporter.test.ts

# Em RuaDeAcoUnreal, com toolchain MSVC carregada:
python Scripts/generate-canonical-data.py
cmake --build --preset windows-msvc-release --target RuaDeAcoTests
./build/windows-msvc-release/RuaDeAcoTests.exe --parity
./build/windows-msvc-release/RuaDeAcoTests.exe
```

O gerador produz `RDACanonicalData.inc`, `RDAGrabGeometry.inc` e
`RDACanonicalReplays.inc`. Eles são dados de origem, não uma tabela de respostas
usada pela simulação. O core recebe as mesmas definições de golpes, comandos,
colisões e geometria; continua calculando física, contatos, dano e progressão.
O runner usa as expectativas exportadas separadamente. Sua assinatura FNV-1a
recusa um fixture diferente daquele compilado.

## Correções

- Dados efetivos de todos os golpes, incluindo múltiplas hitboxes e janelas;
  colisões medidas nos sprites atuais em vez das antigas caixas reduzidas.
- Buffer de direções/botões, prioridades, custos, cancelamentos e tolerância de
  dois botões no início do agarrão, conforme o TypeScript.
- Ordem de movimento, captura de pose, contatos e avanço dos frames; salto,
  correção aérea, pouso, hitstop, stun, knockdown e invulnerabilidade de wakeup.
- Armadura, passivas, parry/riposte, redução com cooldown, enfraquecimento
  ofensivo e efeito de congelamento. Treino conserva vida infinita como a origem.
- Projéteis com arming, origem, limites por dono e eventos originais.
- Agarrões com timelines e offsets de cada vítima, além das poses medidas do
  universal. Finalizações usam a coreografia original dos três palcos, com
  rotação, escala, visibilidade e corte na borda do panelão.

O snapshot inclui `victimPoseFrame`, `victimScale`, `victimInFront` e
`cutBelowY` para que o frontend possa representar essa coreografia.
`MoveDef.animationKey` e `FighterSnapshot.moveAnimationKey` mantêm a chave
original da animação; aliases genéricos de `AnimId` não identificam o sprite
correto de todos os especiais ou das variações aéreas.

`CombatWorld::DrainSoundCues()` entrega `SoundCue { event, fighterIndex, moveId }`
para os inícios de ataque `Swing`, `SpecialCast` e `SuperCast`. A fila é
independente de `DrainSounds()`, que preserva a API anterior. O índice identifica
o lutador que iniciou o golpe (inclusive CPU/P2), evitando usar as vozes de P1
para todos. O snapshot também expõe `moveConnected` para a pose de whiff do agarrão.

## Evidência e limites

33 replays / 16.170 passos: 18 especiais/supers, normais/aéreos/defesa/agarrão dos
seis lutadores e seis finalizações (três arenas, dois sentidos). São comparados
fase/frame, projéteis/agarrão ativos, posição, velocidade, facing, estado/frame,
vida, barra, grounded, golpe e 13 campos de mecânicas por lutador. O teste TS
exige que os especiais e as finalizações sejam realmente alcançados.

Campos discretos são exatos; valores `float` têm tolerância absoluta de 0,002.
Os hashes completos de estado internos dos motores **não** são comparados e
estes resultados não habilitam lockstep nativo com a versão web.

Os 44 checks locais de gameplay permanecem separados, acrescidos de um teste
da atribuição dos três eventos de ataque e da independência da drenagem de
áudio e outro do planejador da CPU (46 checks ao todo). O teste de projétil Dante foi
corrigido para usar baixo → baixo-trás → trás+especial, e o de finalização passou
a aguardar o startup canônico do agarrão. As expectativas canônicas não foram
alteradas para acomodar diferenças do port.

O planejador da CPU segue `src/ai/CpuController.ts`: observação atrasada,
intervalos e probabilidades por dificuldade, defesa, aproximação e seleção dos
golpes. Cada direção do comando dura três frames, seguida pelos botões e pela
recuperação. O teste exige que cada um dos seis lutadores inicie especial e
super por esse buffer real, sem flags de atalho de super/agarrão. Chamadas
repetidas no mesmo frame retornam a mesma entrada. O RNG local determinístico
substitui o `Math.random` injetável da origem; seu estado fica fora do hash de
combate. Isso valida execução dos comandos, não equivalência de decisões
aleatórias entre partidas web e nativas.

Gate final do core em árvore nova: `cmake -S . -B build/parity-final-ai -G Ninja
-DCMAKE_BUILD_TYPE=Release`, `cmake --build build/parity-final-ai --target
RuaDeAcoTests` e `ctest --test-dir build/parity-final-ai --output-on-failure -R
"RDAGoldenTests|RDACanonicalReplayParity"`: 2/2 suítes aprovadas.

Capturas, renderização e áudio em execução, controles Android, instaladores,
releases e publicação são gates independentes sob responsabilidade do coordenador.
