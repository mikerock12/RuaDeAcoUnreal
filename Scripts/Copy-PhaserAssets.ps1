#requires -Version 5.1
<#
.SYNOPSIS
    Copia um conjunto controlado de assets do Rua de Aco Phaser.
    NAO modifica o projeto original.
#>

[CmdletBinding()]
param(
    [string]$PhaserRoot = "D:\PROJETOS\RuaDeAco"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Src = Join-Path $PhaserRoot "public\assets"
if (-not (Test-Path $Src)) {
    throw "Projeto Phaser nao encontrado em $PhaserRoot (somente leitura)."
}

function Copy-Need([string]$From, [string]$To) {
    New-Item -ItemType Directory -Path (Split-Path $To) -Force | Out-Null
    Copy-Item $From $To -Force
    Write-Host "staging: $To"
}

$guto = @(
    "idle.png","corrida.png","walk-backward.png","jump-neutral.png","jump-forward.png","jump-backward.png",
    "fall.png","landing.png","crouch.png","standing-light.png","standing-heavy.png","forward-heavy.png",
    "crouch-light.png","crouch-heavy.png","air-light-neutral.png","air-heavy-neutral.png",
    "hit.png","knockdown.png","knockout.png","victory.png","wake-up.png","block-standing.png","block-crouching.png"
)

foreach ($f in $guto) {
    Copy-Need (Join-Path $Src "fighters\guto-barba\$f") (Join-Path $Root "Content\RuaDeAco\Art\Characters\GutoBarba\$f")
    Copy-Need (Join-Path $Src "fighters\rafa-mare\$f") (Join-Path $Root "Content\RuaDeAco\Art\Characters\RafaMare\$f")
}

Copy-Need (Join-Path $Src "stages\cais-da-cidade\remaster\background.png") (Join-Path $Root "Content\RuaDeAco\Art\Environments\CaisDaCidade\background.png")
Copy-Need (Join-Path $Src "ui\hud-frame.png") (Join-Path $Root "Content\RuaDeAco\Art\UI\hud-frame.png")
Copy-Need (Join-Path $Src "fonts\PressStart2P-Regular.ttf") (Join-Path $Root "Content\RuaDeAco\Fonts\PressStart2P-Regular.ttf")
Copy-Need (Join-Path $Src "fonts\PressStart2P-OFL.txt") (Join-Path $Root "Content\RuaDeAco\Fonts\PressStart2P-OFL.txt")
if (Test-Path (Join-Path $Src "audio\music\cais-da-cidade.ogg")) {
    Copy-Need (Join-Path $Src "audio\music\cais-da-cidade.ogg") (Join-Path $Root "Content\RuaDeAco\Audio\Music\cais-da-cidade.ogg")
}

Write-Host "Copia concluida. O diretorio $PhaserRoot nao foi modificado."
