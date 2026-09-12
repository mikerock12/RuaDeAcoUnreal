#requires -Version 5.1
<#
.SYNOPSIS
    Prepara um PC Windows 10/11 para desenvolvimento do Rua de Aco Unreal.

.DESCRIPTION
    - Usa o disco D: como raiz para ferramentas pesadas e projetos.
    - Instala Git.
    - Configura Git LFS.
    - Instala Visual Studio 2022 Community com Game Development with C++.
    - Instala Epic Games Launcher.
    - Instala e configura OpenSSH Server.
    - Cria variaveis de ambiente para o projeto.
    - NAO instala automaticamente a Unreal Engine, pois essa etapa exige
      selecao/autenticacao no Epic Games Launcher.

.NOTES
    Execute como Administrador.

    Exemplo:
    powershell.exe -ExecutionPolicy Bypass -File .\setup_rua_de_aco_unreal.ps1
#>

[CmdletBinding()]
param()

$ErrorActionPreference = "Stop"
$ProgressPreference = "SilentlyContinue"

# ---------------------------------------------------------------------------
# CONFIGURACAO
# ---------------------------------------------------------------------------

$DriveRoot          = "D:\"
$DevRoot            = "D:\Dev"
$InstallersRoot     = "D:\Dev\Installers"
$VSRoot             = "D:\Dev\VisualStudio2022"
$VSCache            = "D:\Dev\VSCache"
$GitRoot            = "D:\Dev\Git"
$GitLfsRoot         = "D:\Dev\GitLFS"
$EpicRoot           = "D:\Epic"
$EpicLauncherRoot   = "D:\Epic\Launcher"
$UnrealRoot         = "D:\Epic\UE_5.8"
$ProjectsRoot       = "D:\Projetos"
$ProjectRoot        = "D:\Projetos\RuaDeAco-Unreal"
$LogPath            = "D:\Dev\setup-rua-de-aco-unreal.log"

$VSBootstrapperUrl  = "https://aka.ms/vs/17/release/vs_community.exe"
$VSBootstrapperPath = Join-Path $InstallersRoot "vs_community.exe"

# ---------------------------------------------------------------------------
# FUNCOES
# ---------------------------------------------------------------------------

function Write-Step {
    param([string]$Message)
    Write-Host ""
    Write-Host "==> $Message" -ForegroundColor Cyan
}

function Write-OK {
    param([string]$Message)
    Write-Host "[OK] $Message" -ForegroundColor Green
}

function Write-Warn {
    param([string]$Message)
    Write-Host "[AVISO] $Message" -ForegroundColor Yellow
}

function Test-Administrator {
    $identity = [Security.Principal.WindowsIdentity]::GetCurrent()
    $principal = New-Object Security.Principal.WindowsPrincipal($identity)
    return $principal.IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
}

function Test-WingetPackageInstalled {
    param([Parameter(Mandatory)][string]$Id)

    $output = & winget list --id $Id --exact --accept-source-agreements 2>$null
    return ($LASTEXITCODE -eq 0 -and ($output -join "`n") -match [regex]::Escape($Id))
}

function Add-ToMachinePath {
    param([Parameter(Mandatory)][string]$PathToAdd)

    $machinePath = [Environment]::GetEnvironmentVariable("Path", "Machine")
    $entries = @($machinePath -split ";" | Where-Object { $_ })

    if ($entries -notcontains $PathToAdd) {
        $newPath = (($entries + $PathToAdd) -join ";")
        [Environment]::SetEnvironmentVariable("Path", $newPath, "Machine")
        Write-OK "Adicionado ao PATH da maquina: $PathToAdd"
    }
}

# ---------------------------------------------------------------------------
# VALIDACOES
# ---------------------------------------------------------------------------

if (-not (Test-Administrator)) {
    throw "Este script precisa ser executado como Administrador."
}

if (-not (Test-Path $DriveRoot)) {
    throw "O disco D: nao foi encontrado. O script foi interrompido para evitar instalar ferramentas no disco errado."
}

$directories = @(
    $DevRoot,
    $InstallersRoot,
    $VSRoot,
    $VSCache,
    $GitRoot,
    $GitLfsRoot,
    $EpicRoot,
    $EpicLauncherRoot,
    $ProjectsRoot,
    $ProjectRoot
)

foreach ($dir in $directories) {
    if (-not (Test-Path $dir)) {
        New-Item -ItemType Directory -Path $dir -Force | Out-Null
    }
}

Start-Transcript -Path $LogPath -Append | Out-Null

try {
    Write-Step "Verificando Windows Package Manager (winget)"

    if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
        throw @"
winget nao foi encontrado.
Atualize/instale o 'App Installer' pela Microsoft Store e execute este script novamente.
"@
    }

    Write-OK "winget encontrado."

    # -----------------------------------------------------------------------
    # GIT
    # -----------------------------------------------------------------------

    Write-Step "Instalando Git no disco D:"

    $gitExe = Join-Path $GitRoot "cmd\git.exe"

    if (-not (Test-Path $gitExe)) {
        if (Test-WingetPackageInstalled -Id "Git.Git") {
            Write-Warn "O Git ja esta registrado como instalado no Windows, mas nao foi encontrado em $gitExe."
            Write-Warn "O script nao remove uma instalacao existente automaticamente."
            $existingGit = Get-Command git -ErrorAction SilentlyContinue
            if ($existingGit) {
                Write-Warn "Git atual: $($existingGit.Source)"
            }
        }
        else {
            & winget install `
                --id Git.Git `
                --exact `
                --source winget `
                --location $GitRoot `
                --silent `
                --accept-package-agreements `
                --accept-source-agreements

            if ($LASTEXITCODE -ne 0) {
                throw "Falha ao instalar Git pelo winget."
            }
        }
    }

    if (Test-Path $gitExe) {
        Write-OK "Git instalado em $GitRoot"
        Add-ToMachinePath -PathToAdd (Join-Path $GitRoot "cmd")
        $env:Path = "$(Join-Path $GitRoot 'cmd');$env:Path"
    }
    elseif (Get-Command git -ErrorAction SilentlyContinue) {
        $gitExe = (Get-Command git).Source
        Write-Warn "Usando Git existente em: $gitExe"
    }
    else {
        throw "Git nao foi localizado apos a instalacao."
    }

    # -----------------------------------------------------------------------
    # GIT LFS
    # -----------------------------------------------------------------------

    Write-Step "Configurando Git LFS"

    $lfsAvailable = $false

    try {
        & $gitExe lfs version | Out-Host
        if ($LASTEXITCODE -eq 0) {
            $lfsAvailable = $true
        }
    }
    catch {
        $lfsAvailable = $false
    }

    if (-not $lfsAvailable) {
        Write-Warn "Git LFS nao veio disponivel junto com o Git. Tentando instalar pacote separado."

        if (-not (Test-WingetPackageInstalled -Id "GitHub.GitLFS")) {
            & winget install `
                --id GitHub.GitLFS `
                --exact `
                --source winget `
                --location $GitLfsRoot `
                --silent `
                --accept-package-agreements `
                --accept-source-agreements

            if ($LASTEXITCODE -ne 0) {
                throw "Falha ao instalar Git LFS."
            }
        }
    }

    & $gitExe lfs install
    if ($LASTEXITCODE -ne 0) {
        throw "Git LFS foi localizado, mas 'git lfs install' falhou."
    }

    Write-OK "Git LFS configurado."

    # -----------------------------------------------------------------------
    # VISUAL STUDIO 2022
    # -----------------------------------------------------------------------

    Write-Step "Instalando Visual Studio 2022 Community para Unreal C++"

    $devenv = Join-Path $VSRoot "Common7\IDE\devenv.exe"

    if (Test-Path $devenv) {
        Write-OK "Visual Studio ja encontrado em $VSRoot"
    }
    else {
        if (-not (Test-Path $VSBootstrapperPath)) {
            Write-Host "Baixando bootstrapper oficial do Visual Studio 2022..."
            Invoke-WebRequest -Uri $VSBootstrapperUrl -OutFile $VSBootstrapperPath
        }

        $vsArgs = @(
            "--installPath", "`"$VSRoot`"",
            "--path", "cache=`"$VSCache`"",
            "--add", "Microsoft.VisualStudio.Workload.NativeGame",
            "--add", "Microsoft.VisualStudio.Component.VC.Tools.x86.x64",
            "--add", "Microsoft.VisualStudio.Component.Windows11SDK.26100",
            "--add", "Component.Unreal.Debugger",
            "--add", "Component.Unreal.Ide",
            "--add", "Microsoft.VisualStudio.Component.Unreal.TestAdapter",
            "--includeRecommended",
            "--passive",
            "--wait",
            "--norestart",
            "--nocache"
        )

        $process = Start-Process `
            -FilePath $VSBootstrapperPath `
            -ArgumentList $vsArgs `
            -Wait `
            -PassThru

        # Visual Studio Installer may use documented non-zero reboot/success codes.
        if ($process.ExitCode -notin @(0, 3010)) {
            throw "Visual Studio Installer retornou codigo $($process.ExitCode). Consulte o log."
        }

        if ($process.ExitCode -eq 3010) {
            Write-Warn "Visual Studio solicitou reinicializacao do Windows."
        }

        Write-OK "Instalacao/configuracao do Visual Studio concluida."
    }

    # -----------------------------------------------------------------------
    # EPIC GAMES LAUNCHER
    # -----------------------------------------------------------------------

    Write-Step "Instalando Epic Games Launcher"

    if (Test-WingetPackageInstalled -Id "EpicGames.EpicGamesLauncher") {
        Write-OK "Epic Games Launcher ja esta instalado."
    }
    else {
        & winget install `
            --id EpicGames.EpicGamesLauncher `
            --exact `
            --source winget `
            --location $EpicLauncherRoot `
            --silent `
            --accept-package-agreements `
            --accept-source-agreements

        if ($LASTEXITCODE -ne 0) {
            throw "Falha ao instalar Epic Games Launcher."
        }

        Write-OK "Epic Games Launcher instalado."
    }

    Write-Warn "O launcher pode manter pequenos componentes/metadados no C:, mesmo com o destino solicitado no D:."
    Write-Warn "A Unreal Engine NAO sera instalada automaticamente."
    Write-Warn "No Epic Games Launcher, instale Unreal Engine 5.8 em: $UnrealRoot"

    # -----------------------------------------------------------------------
    # OPENSSH SERVER
    # -----------------------------------------------------------------------

    Write-Step "Instalando e configurando OpenSSH Server"

    $sshCapability = Get-WindowsCapability -Online -Name "OpenSSH.Server~~~~0.0.1.0"

    if ($sshCapability.State -ne "Installed") {
        Add-WindowsCapability -Online -Name "OpenSSH.Server~~~~0.0.1.0" | Out-Null
    }

    Set-Service -Name sshd -StartupType Automatic

    if ((Get-Service sshd).Status -ne "Running") {
        Start-Service sshd
    }

    $firewallRule = Get-NetFirewallRule -Name "OpenSSH-Server-In-TCP" -ErrorAction SilentlyContinue

    if (-not $firewallRule) {
        New-NetFirewallRule `
            -Name "OpenSSH-Server-In-TCP" `
            -DisplayName "OpenSSH Server (sshd)" `
            -Enabled True `
            -Direction Inbound `
            -Protocol TCP `
            -Action Allow `
            -LocalPort 22 | Out-Null
    }
    else {
        Enable-NetFirewallRule -Name "OpenSSH-Server-In-TCP" | Out-Null
    }

    Write-OK "OpenSSH Server configurado e iniciado."

    # -----------------------------------------------------------------------
    # VARIAVEIS DE AMBIENTE
    # -----------------------------------------------------------------------

    Write-Step "Criando variaveis de ambiente"

    [Environment]::SetEnvironmentVariable("DEV_ROOT", $DevRoot, "Machine")
    [Environment]::SetEnvironmentVariable("UE_ROOT", $UnrealRoot, "Machine")
    [Environment]::SetEnvironmentVariable("RDA_UNREAL_ROOT", $ProjectRoot, "Machine")

    $env:DEV_ROOT = $DevRoot
    $env:UE_ROOT = $UnrealRoot
    $env:RDA_UNREAL_ROOT = $ProjectRoot

    Write-OK "DEV_ROOT=$DevRoot"
    Write-OK "UE_ROOT=$UnrealRoot"
    Write-OK "RDA_UNREAL_ROOT=$ProjectRoot"

    # -----------------------------------------------------------------------
    # RESUMO
    # -----------------------------------------------------------------------

    Write-Host ""
    Write-Host "============================================================" -ForegroundColor Green
    Write-Host " PREPARACAO DO RUA DE ACO UNREAL CONCLUIDA" -ForegroundColor Green
    Write-Host "============================================================" -ForegroundColor Green
    Write-Host ""
    Write-Host "Projeto novo:"
    Write-Host "  $ProjectRoot"
    Write-Host ""
    Write-Host "Visual Studio:"
    Write-Host "  $VSRoot"
    Write-Host ""
    Write-Host "Git:"
    Write-Host "  $GitRoot"
    Write-Host ""
    Write-Host "Unreal Engine (instalacao manual pendente):"
    Write-Host "  $UnrealRoot"
    Write-Host ""
    Write-Host "Log:"
    Write-Host "  $LogPath"
    Write-Host ""

    Write-Host "ETAPAS MANUAIS RESTANTES:" -ForegroundColor Yellow
    Write-Host "1. Abra o Epic Games Launcher."
    Write-Host "2. Entre na sua conta Epic."
    Write-Host "3. Acesse Unreal Engine > Library."
    Write-Host "4. Instale Unreal Engine 5.8 em:"
    Write-Host "   $UnrealRoot"
    Write-Host "5. Reinicie o PC se o Visual Studio/Windows solicitar."
    Write-Host "6. Abra um novo PowerShell para carregar o PATH atualizado."
    Write-Host ""
    Write-Host "Depois disso, valide:"
    Write-Host "  Test-Path `"$UnrealRoot\Engine\Binaries\Win64\UnrealEditor.exe`""
    Write-Host "  git --version"
    Write-Host "  git lfs version"
    Write-Host "  Get-Service sshd"
    Write-Host ""
}
finally {
    Stop-Transcript | Out-Null
}
