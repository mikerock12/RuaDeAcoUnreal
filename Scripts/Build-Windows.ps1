#requires -Version 5.1
<#
.SYNOPSIS
    Compila o prototipo jogavel Win64 (C++ / raylib) do Rua de Aco Unreal.

.DESCRIPTION
    A Unreal Engine 5.8 ainda nao e obrigatoria para este alvo.
    O combate e o HUD compilam com MSVC e geram um .exe em Saved/Packages/Windows.
#>

[CmdletBinding()]
param(
    [ValidateSet("Debug", "Release")]
    [string]$Configuration = "Release"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$VcVars = "D:\Dev\VisualStudio2022\VC\Auxiliary\Build\vcvars64.bat"
if (-not (Test-Path $VcVars)) {
    $vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
    if (Test-Path $vswhere) {
        $vsPath = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
        $candidate = Join-Path $vsPath "VC\Auxiliary\Build\vcvars64.bat"
        if (Test-Path $candidate) { $VcVars = $candidate }
    }
}
if (-not (Test-Path $VcVars)) {
    throw "vcvars64.bat nao encontrado. Instale Visual Studio 2022 com C++."
}

$OutDir = Join-Path $Root "Saved\Packages\Windows"
New-Item -ItemType Directory -Path $OutDir -Force | Out-Null
$ObjDir = Join-Path $Root "Intermediate\Prototype"
New-Item -ItemType Directory -Path $ObjDir -Force | Out-Null

$Sources = @(
    (Join-Path $Root "Source\RuaDeAcoRuntime\Private\RDAFighterCatalog.cpp"),
    (Join-Path $Root "Source\RuaDeAcoRuntime\Private\RDAFighter.cpp"),
    (Join-Path $Root "Source\RuaDeAcoRuntime\Private\RDACombatWorld.cpp"),
    (Join-Path $Root "Source\Prototype\PrototypeMain.cpp")
)

$IncludeRuntime = Join-Path $Root "Source\RuaDeAcoRuntime\Public"
$IncludeRaylib = Join-Path $Root "ThirdParty\raylib\include"
$LibRaylib = Join-Path $Root "ThirdParty\raylib\lib"
$ExePath = Join-Path $OutDir "RuaDeAcoUnreal.exe"

$OptFlags = if ($Configuration -eq "Debug") { @("/Od", "/Zi", "/D_DEBUG") } else { @("/O2", "/DNDEBUG") }
$Fo = ($ObjDir.TrimEnd('\') + '\')
$clRsp = Join-Path $ObjDir "compile.rsp"
$linkRsp = Join-Path $ObjDir "link.rsp"

@(
    "/nologo",
    "/c",
    "/std:c++20",
    "/EHsc",
    "/utf-8",
    "/W3",
    "/MD",
    "/DNOMINMAX",
    "/DPLATFORM_DESKTOP",
    "/DGRAPHICS_API_OPENGL_33",
    "/I$IncludeRuntime",
    "/I$IncludeRaylib",
    "/Fo$Fo"
) + $OptFlags + $Sources | Set-Content -Path $clRsp -Encoding ascii

$objs = $Sources | ForEach-Object { Join-Path $ObjDir ((Split-Path $_ -Leaf) -replace '\.cpp$', '.obj') }
@(
    "/nologo",
    "/SUBSYSTEM:WINDOWS",
    "/ENTRY:mainCRTStartup",
    "/OUT:$ExePath",
    "/LIBPATH:$LibRaylib"
) + $objs + @(
    "raylib.lib",
    "winmm.lib",
    "gdi32.lib",
    "user32.lib",
    "shell32.lib",
    "opengl32.lib",
    "advapi32.lib"
) | Set-Content -Path $linkRsp -Encoding ascii

Write-Host "Compilando prototipo $Configuration..."
cmd.exe /c "call `"$VcVars`" && cl.exe `"@$clRsp`" && link.exe `"@$linkRsp`""
if ($LASTEXITCODE -ne 0) {
    throw "Falha na compilacao (codigo $LASTEXITCODE)."
}

Copy-Item (Join-Path $Root "ThirdParty\raylib\lib\raylib.dll") $OutDir -Force
$ContentSrc = Join-Path $Root "Content"
$ContentDst = Join-Path $OutDir "Content"
if (Test-Path $ContentDst) {
    Remove-Item $ContentDst -Recurse -Force
}
Copy-Item $ContentSrc $ContentDst -Recurse -Force

Write-Host ""
Write-Host "EXE: $ExePath"
Write-Host "Para jogar: Start-Process `"$ExePath`""
