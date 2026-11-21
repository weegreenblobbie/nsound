<#
.SYNOPSIS
Launches the nsound-claude Docker container and starts Claude Code in the current folder.

.DESCRIPTION
Mounts the current folder at /workspace and .\claude-cfg as Claude's config
directory, so logins and conversation history persist between runs.

Build the image first:  docker build -t nsound-claude .

.PARAMETER Resume
Session ID (UUID) of an earlier conversation to resume.

.PARAMETER Continue
Continue the most recent conversation in this folder.

.PARAMETER Image
Docker image to run. Defaults to nsound-claude.

.EXAMPLE
.\start-claude.ps1

.EXAMPLE
.\start-claude.ps1 -Resume 3f2b9c1e-8a4d-4e7b-9c2a-1d5e6f7a8b9c

.EXAMPLE
.\start-claude.ps1 -Continue

.EXAMPLE
# Anything else is passed straight to claude
.\start-claude.ps1 --model opus
#>
[CmdletBinding()]
param(
    [Alias('r')]
    [string]$Resume,

    [Alias('c')]
    [switch]$Continue,

    [string]$Image = 'nsound-claude',

    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$ClaudeArgs
)

$ErrorActionPreference = 'Stop'

if ($Resume -and $Continue) {
    Write-Error "Use either -Resume or -Continue, not both."
    exit 1
}

if ($Resume) {
    $parsed = [guid]::Empty
    if (-not [guid]::TryParse($Resume, [ref]$parsed)) {
        Write-Error "'$Resume' is not a valid session UUID."
        exit 1
    }
}

if (-not (Get-Command docker -ErrorAction SilentlyContinue)) {
    Write-Error "docker was not found on PATH. Is Docker Desktop installed and running?"
    exit 1
}

$workspace = (Get-Location).Path
$configDir = Join-Path $workspace 'claude-cfg'

# Create the config folder ourselves so Docker doesn't create it with odd permissions
if (-not (Test-Path $configDir)) {
    New-Item -ItemType Directory -Path $configDir | Out-Null
}

# Arguments for docker itself
$dockerArgs = @(
    'run', '-it', '--rm',
    '--name', 'nsound-claude',
    '-v', "${workspace}:/workspace",
    '-v', "${configDir}:/home/ubuntu/.claude",
    # Named volume for Bazel, Bazelisk and ccache caches. It lives inside Docker
    # (fast) instead of on the Windows mount (slow), and survives --rm.
    # Wipe it with:  docker volume rm nsound-cache
    '-v', 'nsound-cache:/home/ubuntu/.cache',
    '-e', 'TERM=xterm-256color',
    '-e', 'COLORTERM=truecolor'
)

# Share your global git config (name, email) read-only, so commits made in the
# container have the right author.
$gitConfig = Join-Path $HOME '.gitconfig'
if (Test-Path $gitConfig) {
    $dockerArgs += @('-v', "${gitConfig}:/home/ubuntu/.gitconfig:ro")
} else {
    Write-Warning "No $gitConfig found, git in the container has no user name or email."
}

# Pass an API key through if one is set (otherwise you log in inside the container)
if ($env:ANTHROPIC_API_KEY) {
    $dockerArgs += @('-e', 'ANTHROPIC_API_KEY')
}

$dockerArgs += $Image

# Arguments appended to "claude --dangerously-skip-permissions" (the image's ENTRYPOINT)
if ($Resume)   { $dockerArgs += @('--resume', $Resume) }
if ($Continue) { $dockerArgs += '--continue' }
if ($ClaudeArgs) { $dockerArgs += $ClaudeArgs }

if ($Resume) {
    Write-Host "Starting Claude Agent container (resuming $Resume)..." -ForegroundColor Cyan
} elseif ($Continue) {
    Write-Host "Starting Claude Agent container (continuing last session)..." -ForegroundColor Cyan
} else {
    Write-Host "Starting Claude Agent container..." -ForegroundColor Cyan
}

& docker @dockerArgs
exit $LASTEXITCODE