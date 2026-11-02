# Rua de Aço — port nativo C++20

O [Rua de Aço original TypeScript/Phaser](https://github.com/mikerock12/RuaDeAco) define as regras, os lutadores, os golpes, a arte e o áudio atuais. Este repositório reaproveita a base C++ existente e a atualiza conforme essa referência; a implementação Unreal antiga não é a fonte de gameplay.

A versão 0.3.0 está em validação para Windows x64 e Android ARM64. O mesmo frontend C++/raylib desenha menus, arenas, animações, controles e áudio nas duas plataformas. No Android ele usa NativeActivity; a antiga tela Java simplificada não participa do build.

- Seis lutadores: Guto Barba, Rafa Maré, Noir Reflexo, Astro Riso, Dante Sinal e Léo Violeta.
- Três arenas: Cais da Cidade, Cozinha Macabra e Sítio.
- Comandos, colisões calibradas, múltiplas janelas de golpes, agarrões, especiais, supers, passivas e finalizações importados do contrato original.
- Versus CPU, versus local e treinamento. Dois jogadores locais exigem teclados ou controles externos; o HUD touch controla P1.
- Cenas, poses e efeitos usam assets originais e metadados por animação. Apenas os lutadores da partida carregam todas as suas folhas.
- O multiplayer por sala continua disponível na edição Web; os ports nativos não anunciam lockstep com ela.

## Fonte, testes e limites

[Docs/PARITY_REPLAYS.md](Docs/PARITY_REPLAYS.md) descreve a comparação de 33 replays e 16.170 passos contra o motor TypeScript, com tolerância de 0,002 para valores float e igualdade dos campos discretos exportados. Os testes de catálogo e componentes são separados. Não se comparam os hashes internos completos dos motores e testes de replays não comprovam renderização, áudio, desempenho ou hardware.

O build Windows possui `--render-smoke <pasta>`, que exporta capturas do frontend real, com os seis lutadores e as três finalizações em cenários preparados. Trata-se de um teste de renderização em Windows, sem alegar teste em celular físico.

## Construir e verificar

Com MSVC x64, CMake e Ninja:

```powershell
cmake --preset windows-msvc-release
cmake --build --preset windows-msvc-release
ctest --preset windows-msvc-release --output-on-failure
./Scripts/Build-Windows-Installer.ps1 -Version 0.3.0
```

Depois de atualizar o exportador na pasta irmã `../RuaDeAco`:

```powershell
python Scripts/generate-canonical-data.py
python Scripts/Sync-CanonicalPresentation.py --source ../RuaDeAco
```

Android requer SDK/Build Tools 36, NDK 27.0.12077973, JDK 17, CMake e Ninja. raylib 5.5 tem origem oficial e hash fixado no CMake. `-RaylibSource` permite usar uma cópia local verificada.

```powershell
./Scripts/Build-Android.ps1 -Abi arm64-v8a -UnsignedOnly
./Scripts/Build-Android.ps1 -Abi x86_64 -UnsignedOnly
```

O APK x86_64 serve exclusivamente ao emulador de QA. O APK unsigned ainda não é instalável. Para assinar uma atualização, configure `SLT_ANDROID_KEYSTORE`, `SLT_ANDROID_STORE_PASS` e `SLT_ANDROID_KEY_PASS` no ambiente e omita `-UnsignedOnly`. A chave de produção existente é reutilizada; o script não gera outra identidade. Não versionar chaves, senhas ou binários.

Os artefatos ficam em `Saved/Packages/Release-0.3.0/`. APKs públicos exigem assinatura v2/v3, compatibilidade de certificado e alinhamento de 16 KiB conferidos. O instalador Windows não possui assinatura Authenticode comercial.

O módulo Unreal em `Source/RuaDeAco/` é preservado como frontend opcional. O caminho distribuível leve usa `Source/Prototype/`; o core `Source/RuaDeAcoRuntime/` permanece independente de raylib e Unreal.

**Autor:** Maicon Nunes / Smells Like Tech. Licenças de terceiros acompanham seus componentes.
