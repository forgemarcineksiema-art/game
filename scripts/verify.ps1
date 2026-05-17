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

    python tools\world_author.py --check
    if ($LASTEXITCODE -ne 0) {
        exit $LASTEXITCODE
    }

    python tools\world_author.py --preview-html build\world_preview\veyra_reach_preview.html --report-json build\world_preview\veyra_reach_report.json
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
        python tools\runtime_scene_smoke.py --exe $ExePath --scene data\scenes\veyra_reach_pilot.scene.json --renderer gdi --ui-mode playtest --capture-frame build\captures\veyra-reach-pilot-runtime-smoke.bmp --report-json build\runtime\veyra-reach-pilot-smoke-report.json
        if ($LASTEXITCODE -ne 0) {
            exit $LASTEXITCODE
        }

        python tools\runtime_scene_smoke.py --exe $ExePath --scene data\scenes\veyra_reach_pilot.scene.json --ui-mode debug --report-json build\runtime\veyra-reach-pilot-debug-smoke-report.json
        if ($LASTEXITCODE -ne 0) {
            exit $LASTEXITCODE
        }

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
