#define MyAppName "Rua de Aco"
#ifndef MyAppVersion
#define MyAppVersion "0.4.0"
#endif
#define MyAppPublisher "Smells Like Tech"
#define MyAppURL "https://softwares.smellsliketech.com.br/games/rua-de-aco/"
#define MyAppExeName "RuaDeAcoUnreal.exe"

[Setup]
AppId={{8B4A9C12-5D3E-4F81-9A22-7C6E1042FA01}
AppName={#MyAppName}
AppVersion={#MyAppVersion}
AppPublisher={#MyAppPublisher}
AppPublisherURL={#MyAppURL}
AppSupportURL={#MyAppURL}
AppUpdatesURL={#MyAppURL}
DefaultDirName={autopf}\SmellsLikeTech\RuaDeAco
DefaultGroupName={#MyAppName}
DisableProgramGroupPage=yes
PrivilegesRequired=lowest
PrivilegesRequiredOverridesAllowed=dialog
OutputDir=..\..\Saved\Packages\Release-{#MyAppVersion}
OutputBaseFilename=RuaDeAco-{#MyAppVersion}-Windows-x64-Setup
Compression=lzma2/fast
SolidCompression=yes
WizardStyle=modern
SetupIconFile=Branding\AppIcon.ico
ArchitecturesAllowed=x64compatible
ArchitecturesInstallIn64BitMode=x64compatible
UninstallDisplayIcon={app}\{#MyAppExeName}

[Languages]
Name: "brazilianportuguese"; MessagesFile: "compiler:Languages\BrazilianPortuguese.isl"

[Tasks]
Name: "desktopicon"; Description: "{cm:CreateDesktopIcon}"; GroupDescription: "{cm:AdditionalIcons}"; Flags: unchecked

[Files]
Source: "..\..\Saved\Packages\Windows\{#MyAppExeName}"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\Saved\Packages\Windows\raylib.dll"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\Saved\Packages\Windows\Content\*"; DestDir: "{app}\Content"; Flags: ignoreversion recursesubdirs createallsubdirs

Source: "..\..\Saved\Packages\Windows\LICENSE-WebView2.txt"; DestDir: "{app}"; Flags: ignoreversion
Source: "..\..\Saved\Packages\Windows\NOTICE-WebView2.txt"; DestDir: "{app}"; Flags: ignoreversion

Source: "..\..\Saved\Packages\Windows\MicrosoftEdgeWebview2Setup.exe"; DestDir: "{app}"; Flags: ignoreversion

[Icons]
Name: "{group}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; IconFilename: "{app}\{#MyAppExeName}"; IconIndex: 0
Name: "{autodesktop}\{#MyAppName}"; Filename: "{app}\{#MyAppExeName}"; WorkingDir: "{app}"; IconFilename: "{app}\{#MyAppExeName}"; IconIndex: 0; Tasks: desktopicon

[Code]
function NeedsWebView2: Boolean;
var Version: String;
begin
  Result := not ((RegQueryStringValue(HKLM32, 'SOFTWARE\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}', 'pv', Version) and (Version <> '') and (Version <> '0.0.0.0')) or
    (RegQueryStringValue(HKCU, 'Software\Microsoft\EdgeUpdate\Clients\{F3017226-FE2A-4295-8BDF-00C3A9A7E4C5}', 'pv', Version) and (Version <> '') and (Version <> '0.0.0.0')));
end;

[Run]
Filename: "{app}\MicrosoftEdgeWebview2Setup.exe"; Parameters: "/silent /install"; StatusMsg: "Preparando componente Microsoft do modo online (requer internet)..."; Check: NeedsWebView2; Flags: waituntilterminated
Filename: "{app}\{#MyAppExeName}"; Description: "{cm:LaunchProgram,{#StringChange(MyAppName, '&', '&&')}}"; Flags: nowait postinstall skipifsilent
