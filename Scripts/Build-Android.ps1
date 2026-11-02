#requires -Version 5.1
<#
.SYNOPSIS
    Compila a biblioteca C++20 nativa arm64-v8a (libruadeaco.so) com Android NDK 27 e empacota/assina o APK RuaDeAco-0.2.0-Android-arm64.apk (targetSdk 36).
#>
[CmdletBinding()]
param(
    [string]$Version = "0.2.0"
)

$ErrorActionPreference = "Stop"
$Root = Split-Path -Parent $PSScriptRoot
$AndroidDir = Join-Path $Root "Platform\Android"
$BuildDir = Join-Path $Root "build\android-arm64"
$ReleaseDir = Join-Path $Root "Saved\Packages\Release-$Version"

$SdkRoot = "C:\Users\Maicon\AppData\Local\Android\Sdk"
$BuildTools = Join-Path $SdkRoot "build-tools\36.0.0"
$PlatformJar = Join-Path $SdkRoot "platforms\android-36\android.jar"
$NdkClang = Join-Path $SdkRoot "ndk\27.0.12077973\toolchains\llvm\prebuilt\windows-x86_64\bin\aarch64-linux-android26-clang++.cmd"
$JdkHome = "C:\Program Files\Microsoft\jdk-17.0.21.21-hotspot"
$Javac = Join-Path $JdkHome "bin\javac.exe"
$Keytool = Join-Path $JdkHome "bin\keytool.exe"
$JarTool = Join-Path $JdkHome "bin\jar.exe"

$Aapt2 = Join-Path $BuildTools "aapt2.exe"
$D8 = Join-Path $BuildTools "d8.bat"
$ZipAlign = Join-Path $BuildTools "zipalign.exe"
$ApkSigner = Join-Path $BuildTools "apksigner.bat"

if (Test-Path $BuildDir) { Remove-Item $BuildDir -Recurse -Force }
New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
New-Item -ItemType Directory -Force -Path $ReleaseDir | Out-Null

# 1. Compila o core C++20 RuaDeAcoRuntime + JNI Bridge para arm64-v8a
$LibDir = Join-Path $BuildDir "apk_stage\lib\arm64-v8a"
New-Item -ItemType Directory -Force -Path $LibDir | Out-Null
$SoPath = Join-Path $LibDir "libruadeaco.so"

Write-Host "Compilando libruadeaco.so (arm64-v8a, C++20, NDK 27)..."
& $NdkClang `
    -std=c++20 -O2 -fPIC -shared -static-libstdc++ `
    -I (Join-Path $Root "Source\RuaDeAcoRuntime\Public") `
    (Join-Path $Root "Source\RuaDeAcoRuntime\Private\RDAFighterCatalog.cpp") `
    (Join-Path $Root "Source\RuaDeAcoRuntime\Private\RDAFighter.cpp") `
    (Join-Path $Root "Source\RuaDeAcoRuntime\Private\RDACombatWorld.cpp") `
    (Join-Path $AndroidDir "jni\AndroidBridge.cpp") `
    -llog -lm `
    -o $SoPath
if ($LASTEXITCODE -ne 0) { throw "Falha na compilacao NDK de libruadeaco.so" }

# 2. Compila recursos Android com aapt2
$CompiledResZip = Join-Path $BuildDir "compiled_res.zip"
$GenJavaDir = Join-Path $BuildDir "gen"
$BaseApk = Join-Path $BuildDir "base.unaligned.apk"
New-Item -ItemType Directory -Force -Path $GenJavaDir | Out-Null

& $Aapt2 compile --dir (Join-Path $AndroidDir "res") -o $CompiledResZip
if ($LASTEXITCODE -ne 0) { throw "Falha no aapt2 compile" }

# Prepara assets enxutos (Art dos 6 lutadores + 3 arenas + UI)
$AssetsDir = Join-Path $BuildDir "assets\RuaDeAco\Art"
New-Item -ItemType Directory -Force -Path $AssetsDir | Out-Null
Copy-Item (Join-Path $Root "Content\RuaDeAco\Art\*") $AssetsDir -Recurse -Force

& $Aapt2 link `
    -I $PlatformJar `
    --manifest (Join-Path $AndroidDir "AndroidManifest.xml") `
    --java $GenJavaDir `
    -A (Join-Path $BuildDir "assets") `
    -o $BaseApk `
    $CompiledResZip
if ($LASTEXITCODE -ne 0) { throw "Falha no aapt2 link" }

# 3. Compila classes Java (R.java + MainActivity.java) e converte para classes.dex com d8
$ClassesDir = Join-Path $BuildDir "classes"
New-Item -ItemType Directory -Force -Path $ClassesDir | Out-Null

$JavaFiles = @(
    (Get-ChildItem $GenJavaDir -Recurse -Filter "*.java" | Select-Object -ExpandProperty FullName),
    (Join-Path $AndroidDir "src\com\smellsliketech\ruadeaco\MainActivity.java")
)
& $Javac -source 11 -target 11 -encoding UTF-8 -classpath $PlatformJar -d $ClassesDir $JavaFiles
if ($LASTEXITCODE -ne 0) { throw "Falha no javac" }

$ClassFiles = Get-ChildItem $ClassesDir -Recurse -Filter "*.class" | Select-Object -ExpandProperty FullName
$DexOutDir = Join-Path $BuildDir "apk_stage"
& $D8 --lib $PlatformJar --min-api 26 --output $DexOutDir $ClassFiles
if ($LASTEXITCODE -ne 0) { throw "Falha no d8" }

# 4. Empacota classes.dex + lib/arm64-v8a/libruadeaco.so dentro do APK
Copy-Item $BaseApk (Join-Path $BuildDir "unsigned.apk") -Force
Push-Location $DexOutDir
& $JarTool uf (Join-Path $BuildDir "unsigned.apk") "classes.dex" "lib/arm64-v8a/libruadeaco.so"
Pop-Location
if ($LASTEXITCODE -ne 0) { throw "Falha ao adicionar classes.dex e libruadeaco.so no APK" }

# 5. Alinha com zipalign (4 bytes) e assina com apksigner
$AlignedApk = Join-Path $BuildDir "aligned.apk"
& $ZipAlign -f -p 4 (Join-Path $BuildDir "unsigned.apk") $AlignedApk
if ($LASTEXITCODE -ne 0) { throw "Falha no zipalign" }

$Keystore = Join-Path $env:TEMP "slt_games_local_signing.keystore"
if (-not (Test-Path $Keystore)) {
    & $Keytool -genkeypair -v -keystore $Keystore -storepass "sltgames2026" -keypass "sltgames2026" `
        -alias "sltgames" -keyalg RSA -keysize 2048 -validity 10000 `
        -dname "CN=Smells Like Tech, OU=Games, O=SLT, L=Brasil, S=SP, C=BR" | Out-Null
}

$FinalApk = Join-Path $ReleaseDir "RuaDeAco-$Version-Android-arm64.apk"
& $ApkSigner sign --ks $Keystore --ks-pass pass:sltgames2026 --key-pass pass:sltgames2026 --out $FinalApk $AlignedApk
if ($LASTEXITCODE -ne 0) { throw "Falha no apksigner sign" }

& $ApkSigner verify --verbose $FinalApk
if ($LASTEXITCODE -ne 0) { throw "Falha no apksigner verify" }

& $ZipAlign -c -v 4 $FinalApk | Select-Object -Last 5
Write-Host "APK Android ARM64 assinado e verificado: $FinalApk ($((Get-Item $FinalApk).Length) bytes)"
