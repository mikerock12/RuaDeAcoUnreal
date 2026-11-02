# Palcos e camadas das finalizações

`RDAStageVisuals.h` porta os atores/camadas de `CaisStageView.ts`,
`KitchenStageView.ts`, `SitioStageView.ts` e seus módulos `*Ambience.ts`.
Os PNGs são os atuais em `Content/RuaDeAco/Art/Environments`. O RNG decorativo
é isolado do combate; a pausa congela sua evolução.

```cpp
RDAStageVisuals::StageRenderer stage;
if (!stage.Load(ArtRoot)) { /* pacote incompleto: assets ausentes */ }
const auto snapshot = World->Snapshot();
stage.Update(dtSeconds, snapshot);
stage.DrawBack(snapshot);
const bool victimInStage = RDAStageVisuals::StageRenderer::VictimInStageLayer(snapshot);
if (victimInStage) DrawVictim();
stage.DrawFront(snapshot);
DrawForegroundFighters(victimInStage); // não repetir a vítima já desenhada
stage.DrawOverlay(snapshot);
stage.Unload(); // antes de CloseWindow, depois do último desenho
```

A vítima entra entre as camadas do cenário no frame 60 do Cais, 108 da Cozinha
e 112 do Sítio. Antes disso, ela permanece em primeiro plano. O vencedor fica
acima do cenário. A sombra da vítima deve ser ocultada durante a finalização.

`WorldSnapshot.visualFinisher` fornece posições/timings originais do monstro,
reveal/crop/facing/mandíbulas, portas, mascarado/tridente e sangue. Esses dados
também orientam a vítima no core: o renderer não usa um destino aproximado.
`active` permanece verdadeiro em `matchOver` após uma finalização.

Na animação/efeitos do lutador, usar os campos `pose*` capturados antes dos
contatos: `poseState`, `poseAnim`, `poseStateFrame`, `poseMoveId`,
`poseMoveAnimationKey`, `poseMoveConnected`, `poseMoveTotalFrames` e
`poseHitActiveFrom/To`. O snapshot já fornece o agarrão do vencedor até o frame
84 e o recuo no Sítio. Essas poses não modificam a simulação.

`MoveAnimFrame` mantém as quatro poses semânticas do TypeScript: startup,
preparação, impacto (frame visual 2 durante toda a janela ativa) e recuperação.
Em golpes sem hitbox, a janela visual considera eventos de projétil/buff/redução.
No agarrão universal, `grabPoseIndex` seleciona o quadro; whiff após o frame 9
usa o frame 11 quando `poseMoveConnected` for falso.

Para projéteis, casar `sourceMoveId` com o efeito original e usar `ageFrames` e
`armingFrames`. O primeiro efeito do dono não identifica todos os projéteis.

Sintaxe isolada MSVC aprovada; os gates completos de core e replays foram
executados em árvore limpa após a mudança da ABI dos snapshots. Capturas e QA
integrados Windows/Android ficam registrados pelo coordenador; compilação não
equivale a validação visual.

Responsável: Codex / agente de paridade e palcos, 02/11/2026.
