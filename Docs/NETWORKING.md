# Networking

O vertical slice é **local 1x1**.

A simulação em `RuaDeAcoRuntime` já é passo-fixo e não depende de relógio de render, o que deixa a porta aberta para lockstep/rollback depois.

Não implementar online neste repositório até o slice Windows estar estável. Rollback é um projeto próprio, não um detalhe de replication da Unreal.
