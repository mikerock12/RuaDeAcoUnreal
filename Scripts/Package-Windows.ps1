#requires -Version 5.1
<#
.SYNOPSIS
    Empacota o prototipo Windows em uma pasta pronta para teste.
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
& (Join-Path $PSScriptRoot "Build-Windows.ps1") -Configuration Release

$Root = Split-Path -Parent $PSScriptRoot
$OutDir = Join-Path $Root "Saved\Packages\Windows"
$Zip = Join-Path $Root "Saved\Packages\RuaDeAcoUnreal-0.1-Windows.zip"
if (Test-Path $Zip) { Remove-Item $Zip -Force }
Compress-Archive -Path (Join-Path $OutDir "*") -DestinationPath $Zip -Force
Write-Host "Pacote: $Zip"
