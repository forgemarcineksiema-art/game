param(
    [string]$Preset = "auto"
)

$ErrorActionPreference = "Stop"
$Root = Resolve-Path (Join-Path $PSScriptRoot "..")
$LastPresetPath = Join-Path $Root "build\.last_preset"

if ($Preset -eq "auto") {
    if (!(Test-Path $LastPresetPath)) {
        & (Join-Path $PSScriptRoot "configure.ps1") -Preset auto
        if ($LASTEXITCODE -ne 0) {
            exit $LASTEXITCODE
        }
    }
    $Preset = (Get-Content $LastPresetPath -Raw).Trim()
}

Push-Location $Root
try {
    cmake --build --preset $Preset
    exit $LASTEXITCODE
} finally {
    Pop-Location
}

