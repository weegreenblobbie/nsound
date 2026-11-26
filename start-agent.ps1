<#
.SYNOPSIS
Launches the nsound-agent Docker container and starts Claude Code or Antigravity CLI (agy) in the current folder.

.DESCRIPTION
Mounts the current folder at /workspace and the agent's config directory, so
logins and conversation history persist between runs:

  claude   .\claude-cfg is mounted as ~/.claude
  gemini   .\agy-cfg is mounted as ~/.gemini

Build the image first:  docker build -t nsound-agent .

.PARAMETER Agent
The agent to start: claude or gemini.

.PARAMETER Resume
ID of an earlier conversation to resume: a session UUID for claude, a
conversation ID for gemini.

.PARAMETER Continue
Continue the most recent conversation in this folder.

.PARAMETER Image
Docker image to run. Defaults to nsound-agent.

.EXAMPLE
.\start-agent.ps1 claude

.EXAMPLE
.\start-agent.ps1 claude 3f2b9c1e-8a4d-4e7b-9c2a-1d5e6f7a8b9c

.EXAMPLE
.\start-agent.ps1 gemini -Continue

.EXAMPLE
# Anything else is passed straight to the agent
.\start-agent.ps1 claude --model opus
#>
[CmdletBinding()]
param(
    [Parameter(Mandatory = $true, Position = 0)]
    [ValidateSet('claude', 'gemini')]
    [string]$Agent,

    [Parameter(Position = 1)]
    [Alias('r')]
    [string]$Resume,

    [Alias('c')]
    [switch]$Continue,

    [string]$Image = 'nsound-agent',

    [Parameter(ValueFromRemainingArguments = $true)]
    [string[]]$AgentArgs
)

$ErrorActionPreference = 'Stop'

# An option in the second position, e.g. --model, belongs to the agent
if ($Resume.StartsWith('-')) {
    $AgentArgs = @($Resume) + @($AgentArgs | Where-Object { $_ })
    $Resume = ''
}

if ($Resume -and $Continue) {
    Write-Error "Use either a resume ID or -Continue, not both."
    exit 1
}

# What differs between the two agents
switch ($Agent) {
    'claude' {
        $title        = 'Claude'
        $command      = 'claude'
        $resumeFlag   = '--resume'
        $continueFlag = '--continue'
        $configName   = 'claude-cfg'
        $configMount  = '/home/ubuntu/.claude'
        $envNames     = @('ANTHROPIC_API_KEY')
    }
    'gemini' {
        $title        = 'Antigravity'
        $command      = 'agy'
        $resumeFlag   = '--conversation'
        $continueFlag = '-c'
        $configName   = 'agy-cfg'
        $configMount  = '/home/ubuntu/.gemini'
        $envNames     = @(
            'GEMINI_API_KEY',
            'AGY_ADC_AUTH',
            'GOOGLE_CLOUD_QUOTA_PROJECT',
            'GOOGLE_CLOUD_LOCATION'
        )
    }
}

if ($Resume -and $Agent -eq 'claude') {
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
$configDir = Join-Path $workspace $configName

# Create the config folder ourselves so Docker doesn't create it with odd permissions
if (-not (Test-Path $configDir)) {
    New-Item -ItemType Directory -Path $configDir | Out-Null
}

# Arguments for docker itself
$dockerArgs = @(
    'run', '-it', '--rm',
    '--name', "nsound-$Agent",
    '-v', "${workspace}:/workspace",
    '-v', "${configDir}:${configMount}",
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

# Pass the agent's API key and auth variables through if they are set
# (otherwise you log in inside the container)
foreach ($envName in $envNames) {
    if ([Environment]::GetEnvironmentVariable($envName)) {
        $dockerArgs += @('-e', $envName)
    }
}

$dockerArgs += @($Image, $command, '--dangerously-skip-permissions')

# Arguments for the agent
if ($Resume)    { $dockerArgs += @($resumeFlag, $Resume) }
if ($Continue)  { $dockerArgs += $continueFlag }
if ($AgentArgs) { $dockerArgs += $AgentArgs }

if ($Resume) {
    Write-Host "Starting $title Agent container (resuming $Resume)..." -ForegroundColor Cyan
} elseif ($Continue) {
    Write-Host "Starting $title Agent container (continuing last session)..." -ForegroundColor Cyan
} else {
    Write-Host "Starting $title Agent container..." -ForegroundColor Cyan
}

& docker @dockerArgs
exit $LASTEXITCODE
