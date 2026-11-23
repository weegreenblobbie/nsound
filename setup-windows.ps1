<#
.SYNOPSIS
Installs what the Bazel build of nsound needs on Windows: Bazelisk (as bazel)
and the Visual Studio C++ Build Tools, and turns on Developer Mode and long
paths for the Python tests.

.DESCRIPTION
Run it from a PowerShell window in the nsound folder:

    .\setup-windows.ps1

Each step is skipped when the tool is already installed, so it is safe to run
again.  Bazelisk goes to %LOCALAPPDATA%\bazelisk\bazel.exe and that folder is
added to your user PATH.  The Build Tools are installed with winget and need
an administrator prompt (Windows asks for it).  BAZEL_VC is set for your user
so Bazel finds them.

Bazel's Python tests need two Windows settings: Developer Mode, to create
symlinks without being an administrator, and long paths, for paths longer
than 260 characters.  The script explains each one that is off and turns
them on from one administrator prompt.

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
# The release's bazelisk-windows-amd64.exe.sha256, pinned here: a hash from
# the same place as the download would only catch a broken download.
$bazeliskSha256 = '5d14a31e9d8a363c48ba1900dbd56764d730835232eb0d113cce255856ffe3ef'
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

    Invoke-WebRequest -Uri $bazeliskUrl -OutFile $bazelExe -UseBasicParsing
    $actual = (Get-FileHash -Algorithm SHA256 $bazelExe).Hash

    # -ne ignores case, Get-FileHash prints upper case.
    if ($actual -ne $bazeliskSha256) {
        Remove-Item $bazelExe
        Write-Error "SHA256 of $bazelExe is $actual, expected $bazeliskSha256."
        exit 1
    }

    Write-Host "Bazelisk $bazeliskVersion SHA256 $actual passed." -ForegroundColor Green

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

    # winget checks the installer against the SHA256 in its package manifest
    # and prints "Successfully verified installer hash".

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
# Two Windows settings Bazel's Python tests (bazel test //src/test:py_test_*)
# need.  Both are machine wide, so the ones that are off are turned on
# together from one administrator prompt.

$windowsSettings = @(
    @{
        Name  = 'Developer Mode'
        Key   = 'HKLM:\SOFTWARE\Microsoft\Windows\CurrentVersion\AppModelUnlock'
        Value = 'AllowDevelopmentWithoutDevLicense'
        On    = "Bazel's Python tests can create their symlinks."
        Why   = @(
            "rules_python starts each Python test in a venv it makes at run time, with a"
            "symlink to python.exe, and Windows only lets administrators create symlinks"
            "unless Developer Mode is on.  Without it the tests fail with 'WinError 1314:"
            "A required privilege is not held by the client'.  It is the same switch as"
            "Settings > System > For developers > Developer Mode."
        )
    },
    @{
        Name  = 'Long paths'
        Key   = 'HKLM:\SYSTEM\CurrentControlSet\Control\FileSystem'
        Value = 'LongPathsEnabled'
        On    = "Python can load files from Bazel's deep folders."
        Why   = @(
            "Windows paths stop at 260 characters unless long paths are on, and the paths"
            "of the Python packages in Bazel's output folder are longer: numpy fails to"
            "import with 'No module named numpy._core._multiarray_umath'.  With long paths"
            "on, programs that support them, python.exe among them, can use longer paths."
        )
    }
)

function Test-WindowsSetting($setting) {
    $item = Get-ItemProperty -Path $setting.Key -Name $setting.Value -ErrorAction SilentlyContinue
    return ($null -ne $item) -and ($item.($setting.Value) -eq 1)
}

$settingsOff = @($windowsSettings | Where-Object { -not (Test-WindowsSetting $_) })

foreach ($setting in $windowsSettings) {
    if (Test-WindowsSetting $setting) {
        Write-Host "$($setting.Name) is on: $($setting.On)" -ForegroundColor Green
    } else {
        Write-Host "$($setting.Name) is off." -ForegroundColor Yellow
        $setting.Why | ForEach-Object { Write-Host "  $_" }
        Write-Host "  Turning it on sets $($setting.Value) = 1 under"
        Write-Host "  $($setting.Key)."
    }
}

if ($settingsOff.Count -gt 0) {
    $names = ($settingsOff | ForEach-Object { $_.Name }) -join ' and '
    Write-Host "Turning on $names, Windows asks for an administrator prompt ..." -ForegroundColor Cyan

    # The key is made only when it is missing: New-Item -Force on an existing
    # key would recreate it empty, losing its other values.
    $command = ($settingsOff | ForEach-Object {
        "if (-not (Test-Path '$($_.Key)')) { New-Item -Path '$($_.Key)' | Out-Null }; " +
        "Set-ItemProperty -Path '$($_.Key)' -Name '$($_.Value)' -Value 1 -Type DWord"
    }) -join '; '

    try {
        $process = Start-Process powershell -Verb RunAs -Wait -PassThru `
            -ArgumentList '-NoProfile', '-Command', $command
        $exitCode = $process.ExitCode
    } catch {
        # Start-Process throws when the administrator prompt is declined.
        $exitCode = -1
    }

    foreach ($setting in $settingsOff) {
        if (Test-WindowsSetting $setting) {
            Write-Host "$($setting.Name) is on, for programs started from now on." -ForegroundColor Green
        } else {
            Write-Warning ("$($setting.Name) is still off (exit code $exitCode).  The C++ build " +
                "and tests work without it, the Python tests don't.  Run this script again, or " +
                "set $($setting.Value) = 1 under $($setting.Key) from an administrator PowerShell.")
        }
    }
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
