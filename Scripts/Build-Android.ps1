#requires -Version 5.1
# Builds the same C++ frontend as Windows. x86_64 is for emulator QA only.
[CmdletBinding()]
param(
    [string]$Version = '0.3.0',
    [int]$VersionCode = 300,
    [ValidateSet('arm64-v8a', 'x86_64')][string]$Abi = 'arm64-v8a',
    [string]$RaylibSource = $env:RAYLIB_SOURCE,
    [switch]$UnsignedOnly,
    [string]$Keystore = $env:SLT_ANDROID_KEYSTORE,
    [string]$KeyAlias = 'sltgames'
)
$ErrorActionPreference = 'Stop'
$Root = Split-Path -Parent $PSScriptRoot
$AndroidDir = Join-Path $Root 'Platform/Android'
$BuildDir = Join-Path $Root "build/android-$Abi"
$ReleaseDir = Join-Path $Root "Saved/Packages/Release-$Version"
$SdkRoot = if ($env:ANDROID_SDK_ROOT) { $env:ANDROID_SDK_ROOT } else { Join-Path $env:LOCALAPPDATA 'Android/Sdk' }
$NdkRoot = if ($env:ANDROID_NDK_ROOT) { $env:ANDROID_NDK_ROOT } else { Join-Path $SdkRoot 'ndk/27.0.12077973' }
$BuildTools = Join-Path $SdkRoot 'build-tools/36.0.0'
$PlatformJar = Join-Path $SdkRoot 'platforms/android-36/android.jar'
$Jdk = if ($env:JAVA_HOME) { $env:JAVA_HOME } else { 'C:/Program Files/Microsoft/jdk-17.0.21.21-hotspot' }
if (-not $UnsignedOnly) {
    if (-not $Keystore -or -not (Test-Path -LiteralPath $Keystore)) { throw 'Set SLT_ANDROID_KEYSTORE to the existing release keystore; never generate a replacement for an update.' }
    if (-not $env:SLT_ANDROID_STORE_PASS -or -not $env:SLT_ANDROID_KEY_PASS) { throw 'Set SLT_ANDROID_STORE_PASS and SLT_ANDROID_KEY_PASS in the build environment.' }
}
function Run-Checked([string]$Exe, [string[]]$Arguments) {
    & $Exe @Arguments
    if ($LASTEXITCODE -ne 0) { throw "Build command failed: $Exe (exit $LASTEXITCODE)" }
}
New-Item -ItemType Directory -Force -Path $BuildDir, $ReleaseDir | Out-Null
$cmakeArgs = @('-S', $Root, '-B', (Join-Path $BuildDir 'native'), '-G', 'Ninja', "-DCMAKE_TOOLCHAIN_FILE=$NdkRoot/build/cmake/android.toolchain.cmake", "-DANDROID_ABI=$Abi", '-DANDROID_PLATFORM=android-26', '-DANDROID_STL=c++_static', '-DCMAKE_BUILD_TYPE=Release')
if ($RaylibSource) { $cmakeArgs += "-DFETCHCONTENT_SOURCE_DIR_RAYLIB=$RaylibSource" }
Run-Checked 'cmake' $cmakeArgs
Run-Checked 'cmake' @('--build', (Join-Path $BuildDir 'native'), '--target', 'ruadeaco', '--parallel', '4')
$Assets = Join-Path $BuildDir 'assets/Content'
New-Item -ItemType Directory -Force -Path $Assets | Out-Null
Copy-Item -LiteralPath (Join-Path $Root 'Content/RuaDeAco') -Destination $Assets -Recurse -Force
$Manifest = Join-Path $BuildDir 'AndroidManifest.xml'
$manifestText = Get-Content (Join-Path $AndroidDir 'AndroidManifest.xml') -Raw
$manifestText = $manifestText -replace 'android:versionCode="\d+"', "android:versionCode=`"$VersionCode`""
$manifestText = $manifestText -replace 'android:versionName="[^"]+"', "android:versionName=`"$Version`""
[IO.File]::WriteAllText($Manifest, $manifestText)
$ResZip = Join-Path $BuildDir 'resources.zip'
Run-Checked (Join-Path $BuildTools 'aapt2.exe') @('compile', '--dir', (Join-Path $AndroidDir 'res'), '-o', $ResZip)
$Gen = Join-Path $BuildDir 'gen'
New-Item -ItemType Directory -Force -Path $Gen | Out-Null
$BaseApk = Join-Path $BuildDir 'base.apk'
Run-Checked (Join-Path $BuildTools 'aapt2.exe') @('link', '-I', $PlatformJar, '--manifest', $Manifest, '--java', $Gen, '-A', (Join-Path $BuildDir 'assets'), '-o', $BaseApk, $ResZip)
$Classes = Join-Path $BuildDir 'classes'
$Stage = Join-Path $BuildDir 'stage'
$Lib = Join-Path $Stage "lib/$Abi"
New-Item -ItemType Directory -Force -Path $Classes, $Lib | Out-Null
$JavaFiles = @(Get-ChildItem $Gen -Recurse -Filter '*.java' | ForEach-Object FullName) + @(Join-Path $AndroidDir 'src/com/smellsliketech/ruadeaco/MainActivity.java')
Run-Checked (Join-Path $Jdk 'bin/javac.exe') (@('-source', '11', '-target', '11', '-encoding', 'UTF-8', '-classpath', $PlatformJar, '-d', $Classes) + $JavaFiles)
$ClassFiles = @(Get-ChildItem $Classes -Recurse -Filter '*.class' | ForEach-Object FullName)
Run-Checked (Join-Path $BuildTools 'd8.bat') (@('--lib', $PlatformJar, '--min-api', '26', '--output', $Stage) + $ClassFiles)
Copy-Item -LiteralPath (Join-Path $BuildDir 'native/libruadeaco.so') -Destination $Lib -Force
$Unsigned = Join-Path $BuildDir 'unsigned.apk'
Copy-Item -LiteralPath $BaseApk -Destination $Unsigned -Force
Push-Location $Stage
try { Run-Checked (Join-Path $Jdk 'bin/jar.exe') @('uf', $Unsigned, 'classes.dex', 'lib') } finally { Pop-Location }
$Aligned = Join-Path $BuildDir 'aligned.apk'
Run-Checked (Join-Path $BuildTools 'zipalign.exe') @('-f', '-P', '16', '4', $Unsigned, $Aligned)
$Suffix = if ($Abi -eq 'arm64-v8a') { 'arm64' } else { 'x86_64-QA' }
$Final = Join-Path $ReleaseDir "RuaDeAco-$Version-Android-$Suffix.apk"
if ($UnsignedOnly) {
    $Final = Join-Path $ReleaseDir "RuaDeAco-$Version-Android-$Suffix-unsigned.apk"
    Copy-Item -LiteralPath $Aligned -Destination $Final -Force
    Get-FileHash -LiteralPath $Final -Algorithm SHA256
    Write-Host "Unsigned QA candidate: $Final"
    return
}
Run-Checked (Join-Path $BuildTools 'apksigner.bat') @('sign', '--ks', $Keystore, '--ks-key-alias', $KeyAlias, '--ks-pass', 'env:SLT_ANDROID_STORE_PASS', '--key-pass', 'env:SLT_ANDROID_KEY_PASS', '--out', $Final, $Aligned)
Run-Checked (Join-Path $BuildTools 'apksigner.bat') @('verify', '--verbose', $Final)
Run-Checked (Join-Path $BuildTools 'zipalign.exe') @('-c', '-P', '16', '4', $Final)
Get-FileHash -LiteralPath $Final -Algorithm SHA256
Write-Host "C++ frontend APK: $Final"
