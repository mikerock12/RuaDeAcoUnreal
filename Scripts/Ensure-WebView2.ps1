$ErrorActionPreference='Stop'
$Root=Split-Path -Parent $PSScriptRoot
$Version='1.0.4314.35'
$Checksum='90778deb1b77e36292828b4b5833c9255c27533f40e0ddd55891ae718944edc4'
$Destination=Join-Path $Root 'Intermediate/WebView2SDK'
if(Test-Path -LiteralPath (Join-Path $Destination 'build/native/include/WebView2.h')){return $Destination}
New-Item -ItemType Directory -Force -Path $Destination | Out-Null
$Archive=Join-Path $Root 'Intermediate/webview2-sdk.zip'
Invoke-WebRequest -Uri "https://api.nuget.org/v3-flatcontainer/microsoft.web.webview2/$Version/microsoft.web.webview2.$Version.nupkg" -OutFile $Archive
if((Get-FileHash -LiteralPath $Archive -Algorithm SHA256).Hash.ToLower() -ne $Checksum){throw 'WebView2 SDK checksum mismatch'}
Expand-Archive -LiteralPath $Archive -DestinationPath $Destination -Force
return $Destination
