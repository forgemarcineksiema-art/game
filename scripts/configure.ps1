param(
    [string]$Preset = "auto"
)

$ErrorActionPreference = "Stop"
$Root = Resolve-Path (Join-Path $PSScriptRoot "..")

function Invoke-Preset($Name) {
    Write-Host "Configuring preset: $Name"
    Push-Location $Root
    try {
        cmake --preset $Name
        if ($LASTEXITCODE -eq 0) {
            return $true
        }
        return $false
    } finally {
        Pop-Location
    }
}

function Save-LastPreset($Name) {
    $BuildDir = Join-Path $Root "build"
    New-Item -ItemType Directory -Force -Path $BuildDir | Out-Null
    Set-Content -Path (Join-Path $BuildDir ".last_preset") -Value $Name
}

if ($Preset -ne "auto") {
    if (Invoke-Preset $Preset) {
        Save-LastPreset $Preset
        exit 0
    }
    exit 1
}

$Candidates = @(
    "windows-vs2022-debug",
    "windows-vs2026-debug",
    "ninja-debug"
)

foreach ($Candidate in $Candidates) {
    if (Invoke-Preset $Candidate) {
        Write-Host "Configured with preset: $Candidate"
        Save-LastPreset $Candidate
        exit 0
    }
    Write-Host "Preset failed: $Candidate"
}

Write-Host "No configure preset succeeded."
exit 1
