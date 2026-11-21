<#
.SYNOPSIS
Installs what the Bazel build of nsound needs on Windows: Bazelisk (as bazel)
and the Visual Studio C++ Build Tools.

.DESCRIPTION
Run it from a PowerShell window in the nsound folder:

    .\setup-windows.ps1

Each step is skipped when the tool is already installed, so it is safe to run
again.  Bazelisk goes to %LOCALAPPDATA%\bazelisk\bazel.exe and that folder is
added to your user PATH.  The Build Tools are installed with winget and need
an administrator prompt (Windows asks for it).  BAZEL_VC is set for your user
so Bazel finds them.

.PARAMETER Test
After installing, run "bazel build //..." and "bazel test //...".

.EXAMPLE
.\setup-windows.ps1

.EXAMPLE
.\setup-windows.ps1 -Test
#>
[CmdletBinding()]
param(
    [switch]$Test
)

$ErrorActionPreference = 'Stop'

$bazeliskVersion = 'v1.30.0'
$bazeliskUrl = "https://github.com/bazelbuild/bazelisk/releases/download/$bazeliskVersion/bazelisk-windows-amd64.exe"
$bazeliskDir = Join-Path $env:LOCALAPPDATA 'bazelisk'
$bazelExe = Join-Path $bazeliskDir 'bazel.exe'

#------------------------------------------------------------------------------
# Bazelisk, installed as bazel.exe: it reads .bazelversion and downloads that
# Bazel the first time it runs.

if (Get-Command bazel -ErrorAction SilentlyContinue) {
    Write-Host "bazel found: $((Get-Command bazel).Source)" -ForegroundColor Green
} else {
    Write-Host "Installing Bazelisk $bazeliskVersion to $bazelExe ..." -ForegroundColor Cyan

    New-Item -ItemType Directory -Force -Path $bazeliskDir | Out-Null

    # The .sha256 file is served as binary, so save it and read it as text.
    $shaFile = "$bazelExe.sha256"
    Invoke-WebRequest -Uri $bazeliskUrl -OutFile $bazelExe -UseBasicParsing
    Invoke-WebRequest -Uri "$bazeliskUrl.sha256" -OutFile $shaFile -UseBasicParsing
    $expected = ((Get-Content -Raw $shaFile).Trim() -split '\s+')[0]
    Remove-Item $shaFile
    $actual = (Get-FileHash -Algorithm SHA256 $bazelExe).Hash

    if ($actual -ne $expected) {
        Remove-Item $bazelExe
        Write-Error "SHA256 of $bazelExe is $actual, expected $expected."
        exit 1
    }

    $userPath = [Environment]::GetEnvironmentVariable('Path', 'User')
    if (($userPath -split ';') -notcontains $bazeliskDir) {
        [Environment]::SetEnvironmentVariable('Path', "$userPath;$bazeliskDir", 'User')
        Write-Host "Added $bazeliskDir to your user PATH, new windows will see it." -ForegroundColor Cyan
    }
    $env:Path = "$env:Path;$bazeliskDir"
}

#------------------------------------------------------------------------------
# Visual Studio C++ Build Tools (MSVC), which Bazel finds by itself.

$vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio\Installer\vswhere.exe'

function Find-Msvc {
    if (-not (Test-Path $vswhere)) { return $null }
    & $vswhere -latest -products * `
        -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 `
        -property installationPath
}

$msvc = Find-Msvc

if ($msvc) {
    Write-Host "MSVC found: $msvc" -ForegroundColor Green
} else {
    if (-not (Get-Command winget -ErrorAction SilentlyContinue)) {
        Write-Error "winget was not found. Install 'App Installer' from the Microsoft Store, or install the Visual Studio 2022 Build Tools with the 'Desktop development with C++' workload by hand."
        exit 1
    }

    Write-Host "Installing the Visual Studio 2022 C++ Build Tools (this takes a while) ..." -ForegroundColor Cyan

    & winget install --id Microsoft.VisualStudio.2022.BuildTools --exact `
        --accept-package-agreements --accept-source-agreements `
        --override '--wait --passive --add Microsoft.VisualStudio.Workload.VCTools --includeRecommended'

    $msvc = Find-Msvc
    if (-not $msvc) {
        Write-Error "The C++ Build Tools are still not found after winget (exit code $LASTEXITCODE)."
        exit 1
    }
    Write-Host "MSVC installed: $msvc" -ForegroundColor Green
}

# Bazel's own search runs vswhere without "-products *", so it misses a
# Build Tools only install; point it at the VC folder instead.
$vcDir = Join-Path $msvc 'VC'
if ($env:BAZEL_VC -ne $vcDir) {
    [Environment]::SetEnvironmentVariable('BAZEL_VC', $vcDir, 'User')
    $env:BAZEL_VC = $vcDir
    Write-Host "Set BAZEL_VC=$vcDir for your user, new windows will see it." -ForegroundColor Cyan
}

#------------------------------------------------------------------------------
# Check, and optionally build and test.

& bazel version
if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

if ($Test) {
    & bazel build //...
    if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }

    & bazel test //...
    exit $LASTEXITCODE
}

Write-Host "Ready. Try:  bazel build //...   and   bazel test //..." -ForegroundColor Green
