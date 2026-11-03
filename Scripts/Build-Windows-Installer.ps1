#requires -Version 5.1
<#
.SYNOPSIS
    Compila, testa e gera o pacote portatil (.zip) e o instalador Inno Setup (.exe) do Rua de Aco para Windows x64.
#>
[CmdletBinding()]
param(
    [string]$Version = "0.3.1"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot

# 1. Compila o executavel Release e sincroniza Content/
& (Join-Path $PSScriptRoot "Build-Windows.ps1") -Configuration Release -Version $Version

# 2. Executa smoke headless no executavel empacotado
$ExePath = Join-Path $Root "Saved\Packages\Windows\RuaDeAcoUnreal.exe"
$SmokeProcess = Start-Process -FilePath $ExePath -ArgumentList '--headless-smoke' -PassThru -Wait -WindowStyle Hidden
if ($SmokeProcess.ExitCode -ne 0) {
    throw "Falha no smoke headless do executavel Windows."
}
Write-Host 'Smoke headless do executavel empacotado aprovado.'

# 3. Prepara diretorio de release
$ReleaseDir = Join-Path $Root "Saved\Packages\Release-$Version"
New-Item -ItemType Directory -Force -Path $ReleaseDir | Out-Null

# 4. Gera o arquivo .zip portatil com 7-Zip (ou Compress-Archive fallback)
$ZipPath = Join-Path $ReleaseDir "RuaDeAco-$Version-Windows-x64.zip"
if (Test-Path $ZipPath) { Remove-Item $ZipPath -Force }
$SevenZip = "C:\Program Files\7-Zip\7z.exe"
$WinPkgDir = Join-Path $Root "Saved\Packages\Windows"
if (Test-Path $SevenZip) {
    & $SevenZip a -tzip -mx=5 $ZipPath "$WinPkgDir\*" | Out-Null
    if ($LASTEXITCODE -ne 0) { throw 'Falha ao gerar o ZIP portatil.' }
} else {
    Compress-Archive -Path "$WinPkgDir\*" -DestinationPath $ZipPath -Force
}

# 5. Gera o instalador Setup.exe com Inno Setup 6
$Iscc = "C:\Users\Maicon\AppData\Local\Programs\Inno Setup 6\ISCC.exe"
if (-not (Test-Path $Iscc)) {
    throw "Inno Setup 6 (ISCC.exe) nao encontrado em $Iscc"
}
$IssPath = Join-Path $Root "Platform\Windows\RuaDeAco.iss"
& $Iscc /Q "/DMyAppVersion=$Version" $IssPath
if ($LASTEXITCODE -ne 0) {
    throw "Falha ao compilar instalador Inno Setup."
}

$SetupPath = Join-Path $ReleaseDir "RuaDeAco-$Version-Windows-x64-Setup.exe"
Write-Host "ZIP Portatil : $ZipPath ($((Get-Item $ZipPath).Length) bytes)"
Write-Host "Instalador   : $SetupPath ($((Get-Item $SetupPath).Length) bytes)"
