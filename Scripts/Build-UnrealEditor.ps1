#requires -Version 5.1
<#
.SYNOPSIS
    Compila o alvo RuaDeAcoEditor (Unreal Engine 5.8 / C++).
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$Project = Join-Path $Root "RuaDeAco.uproject"

$UeCandidates = @(
    $env:UE_ROOT,
    "D:\Epic\Launcher\UE_5.8",
    "D:\Epic\UE_5.8"
) | Where-Object { $_ }

$UeRoot = $null
foreach ($candidate in $UeCandidates) {
    $bat = Join-Path $candidate "Engine\Build\BatchFiles\Build.bat"
    if (Test-Path $bat) {
        $UeRoot = $candidate
        break
    }
}

if (-not $UeRoot) {
    throw "Unreal Engine 5.8 nao encontrada. Instale em D:\Epic\Launcher\UE_5.8 ou defina UE_ROOT."
}

Write-Host "UE_ROOT=$UeRoot"
& "$UeRoot\Engine\Build\BatchFiles\Build.bat" RuaDeAcoEditor Win64 Development $Project -WaitMutex
if ($LASTEXITCODE -ne 0) {
    throw "Build Unreal falhou (codigo $LASTEXITCODE)."
}

Write-Host "Editor: $UeRoot\Engine\Binaries\Win64\UnrealEditor.exe `"$Project`""
