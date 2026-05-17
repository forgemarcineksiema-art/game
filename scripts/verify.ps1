param(
    [string]$Preset = "auto",
    [switch]$SkipSmokeRun
)

$ErrorActionPreference = "Stop"
$Root = Resolve-Path (Join-Path $PSScriptRoot "..")
$LastPresetPath = Join-Path $Root "build\.last_preset"

& (Join-Path $PSScriptRoot "doctor.ps1")
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

& (Join-Path $PSScriptRoot "configure.ps1") -Preset $Preset
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

if ($Preset -eq "auto") {
    $Preset = (Get-Content $LastPresetPath -Raw).Trim()
}

& (Join-Path $PSScriptRoot "build.ps1") -Preset $Preset
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}

Push-Location $Root
try {
    ctest --preset $Preset
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }

    python tools\validate_scene.py
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }

    python tools\validate_scene.py data\scenes\veyra_reach_pilot.scene.json
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }

    python tools\validate_assets.py
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }

    python tools\world_slice_report.py
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }

    python tools\mesh_report.py
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }

    if (!$SkipSmokeRun) {
        $Candidates = @(
            "build\$Preset\Debug\EngineApp.exe",
            "build\$Preset\EngineApp.exe"
        )
        $Exe = $Candidates | Where-Object { Test-Path (Join-Path $Root $_) } | Select-Object -First 1
        if ($null -eq $Exe) {
            Write-Host "EngineApp executable was not found after build."
            exit 1
        }

        $ExePath = Join-Path $Root $Exe
        & $ExePath --smoke-test --frames 3
        if ($LASTEXITCODE -ne 0) {
            exit $LASTEXITCODE
        }
    }
} finally {
    Pop-Location
}

Write-Host "Verification completed."
exit 0
