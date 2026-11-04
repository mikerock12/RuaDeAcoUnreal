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
    [string]$Configuration = "Release",
    [ValidatePattern('^\d+\.\d+\.\d+$')]
    [string]$Version = "0.4.0"
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
    (Join-Path $Root "Source\Prototype\PrototypeMain.cpp"),
    (Join-Path $Root "Platform\Windows\WindowsBranding.cpp"),
    (Join-Path $Root "Platform\Windows\OnlineWindow.cpp")
)

$IncludeRuntime = Join-Path $Root "Source\RuaDeAcoRuntime\Public"
$IncludeRaylib = Join-Path $Root "ThirdParty\raylib\include"
$LibRaylib = Join-Path $Root "ThirdParty\raylib\lib"
$ExePath = Join-Path $OutDir "RuaDeAcoUnreal.exe"
$WebView2 = & (Join-Path $PSScriptRoot "Ensure-WebView2.ps1")

$OptFlags = if ($Configuration -eq "Debug") { @("/Od", "/Zi", "/D_DEBUG") } else { @("/O2", "/DNDEBUG") }
$Fo = ($ObjDir.TrimEnd('\') + '\')
$clRsp = Join-Path $ObjDir "compile.rsp"
$linkRsp = Join-Path $ObjDir "link.rsp"
$BrandingDir = Join-Path $Root 'Platform/Windows/Branding'
$ResourceFile = Join-Path $ObjDir 'GameResources.rc'
$ResourceObject = Join-Path $ObjDir 'GameResources.res'
$ResourceText = Get-Content -LiteralPath (Join-Path $Root 'Platform/Windows/GameResources.rc.in') -Raw -Encoding UTF8
$ResourceText.Replace('@SLT_BRANDING_DIR@', $BrandingDir.Replace('\','/')).Replace('@SLT_PRODUCT_NAME@', 'Rua de Aço').Replace('@SLT_VERSION_COMMAS@', "$Version.0".Replace('.',',')).Replace('@SLT_VERSION@', $Version) | Set-Content -LiteralPath $ResourceFile -Encoding UTF8

@(
    "/nologo",
    "/c",
    "/std:c++20",
    "/EHsc",
    "/utf-8",
    "/W3",
    "/MT",
    "/DNOMINMAX",
    "/DPLATFORM_DESKTOP",
    "/DUSE_LIBTYPE_SHARED",
    "/DGRAPHICS_API_OPENGL_33",
    "/I$IncludeRuntime",
    "/I$WebView2/build/native/include",
    "/I$IncludeRaylib",
    "/I$(Join-Path $Root 'Platform/Windows')",
    "/Fo$Fo"
) + $OptFlags + $Sources | Set-Content -Path $clRsp -Encoding ascii

$objs = $Sources | ForEach-Object { Join-Path $ObjDir ((Split-Path $_ -Leaf) -replace '\.cpp$', '.obj') }
@(
    "/nologo",
    "/SUBSYSTEM:WINDOWS",
    "/ENTRY:mainCRTStartup",
    "/OUT:$ExePath",
    "/LIBPATH:$LibRaylib",
    "/LIBPATH:$WebView2/build/native/x64"
) + $objs + @(
    $ResourceObject,
    "raylibdll.lib",
    "WebView2LoaderStatic.lib",
    "ole32.lib",
    "oleaut32.lib",
    "shlwapi.lib",
    "version.lib",
    "winmm.lib",
    "gdi32.lib",
    "user32.lib",
    "shell32.lib",
    "opengl32.lib",
    "advapi32.lib"
) | Set-Content -Path $linkRsp -Encoding ascii

Write-Host "Compilando prototipo $Configuration..."
cmd.exe /c "call `"$VcVars`" && rc.exe /nologo /fo `"$ResourceObject`" `"$ResourceFile`" && cl.exe `"@$clRsp`" && link.exe `"@$linkRsp`""
if ($LASTEXITCODE -ne 0) {
    throw "Falha na compilacao (codigo $LASTEXITCODE)."
}

Copy-Item (Join-Path $Root "ThirdParty\raylib\lib\raylib.dll") $OutDir -Force
$ContentSrc = Join-Path $Root "Content"
$ContentDst = Join-Path $OutDir "Content"
if (Test-Path $ContentDst) {
    $ResolvedTarget = (Resolve-Path -LiteralPath $ContentDst).Path
    $ExpectedTarget = [IO.Path]::GetFullPath((Join-Path $Root 'Saved/Packages/Windows/Content'))
    if ($ResolvedTarget -ne $ExpectedTarget) { throw 'Refusing to clean an unexpected content staging path.' }
    Remove-Item -LiteralPath $ResolvedTarget -Recurse -Force
}
Copy-Item $ContentSrc $ContentDst -Recurse -Force
Copy-Item (Join-Path $WebView2 "LICENSE.txt") (Join-Path $OutDir "LICENSE-WebView2.txt") -Force
Copy-Item (Join-Path $WebView2 "NOTICE.txt") (Join-Path $OutDir "NOTICE-WebView2.txt") -Force

Write-Host ""
$Bootstrapper=Join-Path $Root 'Intermediate/MicrosoftEdgeWebview2Setup.exe'
if(-not(Test-Path -LiteralPath $Bootstrapper)){Invoke-WebRequest 'https://go.microsoft.com/fwlink/p/?LinkId=2124703' -OutFile $Bootstrapper}
$Signature=Get-AuthenticodeSignature -LiteralPath $Bootstrapper
if($Signature.Status -ne 'Valid' -or $Signature.SignerCertificate.Subject -notmatch 'O=Microsoft Corporation'){throw 'Microsoft WebView2 bootstrapper signature invalid'}
Copy-Item -LiteralPath $Bootstrapper -Destination $OutDir -Force
Write-Host "EXE: $ExePath"
Write-Host "Para jogar: Start-Process `"$ExePath`""
