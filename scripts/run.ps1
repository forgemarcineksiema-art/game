param(
    [string]$Preset = "auto",
    [Alias("Args")]
    [string[]]$AppArgs = @()
)

$ErrorActionPreference = "Stop"
$Root = Resolve-Path (Join-Path $PSScriptRoot "..")
$LastPresetPath = Join-Path $Root "build\.last_preset"

if ($Preset -eq "auto") {
    if (!(Test-Path $LastPresetPath)) {
        & (Join-Path $PSScriptRoot "build.ps1") -Preset auto
        if ($LASTEXITCODE -ne 0) {
            exit $LASTEXITCODE
        }
    }
    $Preset = (Get-Content $LastPresetPath -Raw).Trim()
}

$Candidates = @(
    "build\$Preset\Debug\EngineApp.exe",
    "build\$Preset\EngineApp.exe"
)
$Exe = $Candidates | Where-Object { Test-Path (Join-Path $Root $_) } | Select-Object -First 1
if ($null -eq $Exe) {
    Write-Host "EngineApp executable was not found. Run scripts/build.ps1 first."
    exit 1
}

& (Join-Path $Root $Exe) @AppArgs
exit $LASTEXITCODE
