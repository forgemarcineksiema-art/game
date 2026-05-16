param(
    [string]$Preset = "windows-vs2022-debug-jolt"
)

$ErrorActionPreference = "Stop"

$root = Resolve-Path (Join-Path $PSScriptRoot "..")
Push-Location $root
try {
    Write-Host "Configuring Jolt preset: $Preset"
    cmake --preset $Preset

    Write-Host "Building Jolt preset: $Preset"
    cmake --build --preset $Preset

    Write-Host "Running Jolt CTest gate"
    ctest --preset $Preset --output-on-failure

    $exe = Join-Path $root "build\$Preset\Debug\EngineApp.exe"
    if (-not (Test-Path $exe)) {
        throw "Jolt EngineApp executable was not found: $exe"
    }

    $playthroughReport = Join-Path $root "build\playthroughs\ferry-office-service-call-jolt-report.json"
    $runtimeReport = Join-Path $root "build\physics\ferry-office-vehicle-runtime-comparison-report.json"

    Write-Host "Running explicit Jolt playthrough QA"
    python tools\playthrough_qa.py `
        --exe $exe `
        --vehicle-runtime jolt `
        --report-json $playthroughReport

    Write-Host "Running Jolt vehicle runtime QA with authored road-edge evidence"
    python tools\vehicle_runtime_qa.py `
        --exe $exe `
        --report-json $runtimeReport

    Write-Host "Jolt gameplay verification completed."
} finally {
    Pop-Location
}
