$ErrorActionPreference = "Stop"
$Root = Resolve-Path (Join-Path $PSScriptRoot "..")
$BuildPath = Join-Path $Root "build"

if (Test-Path $BuildPath) {
    Remove-Item -LiteralPath $BuildPath -Recurse -Force
    Write-Host "Removed $BuildPath"
} else {
    Write-Host "No build directory to remove."
}

