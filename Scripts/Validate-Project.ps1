#requires -Version 5.1
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot

$required = @(
    "RuaDeAco.uproject",
    "IMPLEMENTACAO_RUA_DE_ACO_UNREAL.md",
    "Source\RuaDeAco\RuaDeAco.Build.cs",
    "Source\RuaDeAcoRuntime\Public\RDACombatWorld.h",
    "Source\Prototype\PrototypeMain.cpp",
    "Content\RuaDeAco\Art\Characters\GutoBarba\idle.png",
    "Content\RuaDeAco\Art\Characters\RafaMare\idle.png",
    "Content\RuaDeAco\Art\Environments\CaisDaCidade\background.png"
)

$failed = $false
foreach ($rel in $required) {
    $path = Join-Path $Root $rel
    if (-not (Test-Path $path)) {
        Write-Host "FALTA: $rel" -ForegroundColor Red
        $failed = $true
    }
    else {
        Write-Host "OK    $rel" -ForegroundColor Green
    }
}

$phaserCandidates = @(
    $env:RUA_DE_ACO_WEB_DIR,
    (Join-Path (Split-Path -Parent $Root) "RuaDeAco"),
    "D:\PROJETOS\RuaDeAco"
) | Where-Object { $_ }
$phaser = $phaserCandidates | Where-Object { Test-Path (Join-Path $_ ".git") } | Select-Object -First 1
if ($phaser) {
    Push-Location $phaser
    $dirty = git status --porcelain
    Pop-Location
    if ($dirty) {
        Write-Host "AVISO: o projeto Phaser tem mudancas locais (nao deveriam vir deste repo)." -ForegroundColor Yellow
    }
    else {
        Write-Host "OK    Projeto Phaser intacto." -ForegroundColor Green
    }
}

if ($failed) { exit 1 }
Write-Host "Validacao concluida."
