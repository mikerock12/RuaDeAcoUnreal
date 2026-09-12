#requires -Version 5.1
$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Project = Join-Path $Root "RuaDeAco.uproject"
$Archive = Join-Path $Root "Saved\Packages\UnrealWin64"
$UeRoot = if ($env:UE_ROOT -and (Test-Path "$env:UE_ROOT\Engine\Build\BatchFiles\RunUAT.bat")) {
    $env:UE_ROOT
} elseif (Test-Path "D:\Epic\Launcher\UE_5.8\Engine\Build\BatchFiles\RunUAT.bat") {
    "D:\Epic\Launcher\UE_5.8"
} else {
    throw "Unreal Engine 5.8 nao encontrada no D:."
}

New-Item -ItemType Directory -Path $Archive -Force | Out-Null
Write-Host "UE_ROOT=$UeRoot"
Write-Host "Empacotando RuaDeAco Win64 Development..."
& "$UeRoot\Engine\Build\BatchFiles\RunUAT.bat" BuildCookRun `
    -project="$Project" `
    -noP4 `
    -platform=Win64 `
    -clientconfig=Development `
    -serverconfig=Development `
    -utf8output `
    -build `
    -cook `
    -stage `
    -pak `
    -archive `
    -archivedirectory="$Archive" `
    -unattended `
    -nocompileeditor
if ($LASTEXITCODE -ne 0) {
    throw "RunUAT falhou (codigo $LASTEXITCODE)."
}
Write-Host "Pacote em $Archive"
