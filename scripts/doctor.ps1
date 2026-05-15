param(
    [switch]$Strict
)

$ErrorActionPreference = "Stop"
$Root = Resolve-Path (Join-Path $PSScriptRoot "..")
$Failures = New-Object System.Collections.Generic.List[string]
$Warnings = New-Object System.Collections.Generic.List[string]

function Test-Command($Name, [switch]$Required) {
    $Command = Get-Command $Name -ErrorAction SilentlyContinue
    if ($null -eq $Command) {
        if ($Required) {
            $Failures.Add("$Name was not found in PATH.")
        } else {
            $Warnings.Add("$Name was not found in PATH.")
        }
        return $false
    }

    Write-Host "[ok] $Name -> $($Command.Source)"
    return $true
}

function Test-PathRequired($Path) {
    $FullPath = Join-Path $Root $Path
    if (!(Test-Path $FullPath)) {
        $Failures.Add("Missing expected path: $Path")
    } else {
        Write-Host "[ok] $Path"
    }
}

Write-Host "AI-Native Engine Foundation doctor"
Write-Host "Root: $Root"
Write-Host ""

Test-Command "git" -Required | Out-Null
Test-Command "cmake" -Required | Out-Null
Test-Command "python" -Required | Out-Null

Test-Command "cl" | Out-Null
Test-Command "clang++" | Out-Null
Test-Command "g++" | Out-Null
Test-Command "msbuild" | Out-Null
Test-Command "ninja" | Out-Null
Test-Command "vcpkg" | Out-Null

Write-Host ""
foreach ($Path in @(
    "AGENTS.md",
    "CMakeLists.txt",
    "CMakePresets.json",
    "docs\RUNBOOK.md",
    "docs\ARCHITECTURE.md",
    "docs\ROADMAP.md",
    "docs\STATUS.md",
    "docs\DECISIONS.md",
    "docs\AI_WORKFLOW.md",
    "docs\GAME_DIRECTION.md",
    "docs\VERTICAL_SLICE.md",
    "docs\TECH_DEBT.md",
    "docs\MANUAL_TEST_CHECKLIST.md",
    "docs\PHYSICS_DECISION.md",
    "docs\ASSET_GUIDE.md",
    "docs\SCENE_AUTHORING.md",
    "docs\ART_DIRECTION.md",
    "docs\MESH_RENDERING.md",
    "docs\ASSET_PIPELINE_DECISION.md",
    "docs\BLENDER_WORKFLOW.md",
    "data\scenes\ferry_office.scene.json",
    "assets\models\unit_box.gltf",
    "assets\models\ferry_notice_board.gltf",
    "src\engine",
    "src\game",
    "tests",
    "assets",
    "tools\status_report.py",
    "tools\scene_data.py",
    "tools\scene_report.py",
    "tools\validate_scene.py",
    "tools\validate_assets.py",
    "tools\scale_audit.py",
    "tools\mesh_report.py",
    "tools\check_blender.py",
    "tools\create_simple_prop_gltf.py"
)) {
    Test-PathRequired $Path
}

Write-Host ""
Write-Host "CMake version:"
cmake --version

Write-Host ""
Write-Host "CMake presets:"
cmake --list-presets

if ($Warnings.Count -gt 0) {
    Write-Host ""
    Write-Host "Warnings:"
    foreach ($Warning in $Warnings) {
        Write-Host "  - $Warning"
    }
}

if ($Strict -and $Warnings.Count -gt 0) {
    foreach ($Warning in $Warnings) {
        $Failures.Add($Warning)
    }
}

if ($Failures.Count -gt 0) {
    Write-Host ""
    Write-Host "Doctor failed:"
    foreach ($Failure in $Failures) {
        Write-Host "  - $Failure"
    }
    exit 1
}

Write-Host ""
Write-Host "Doctor completed."
exit 0
