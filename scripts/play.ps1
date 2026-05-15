param(
    [Alias("Args")]
    [string[]]$AppArgs = @(),
    [ValidateSet("gdi", "dx11", "auto", "null")]
    [string]$Renderer = "gdi",
    [ValidateSet("playtest", "debug", "minimal")]
    [string]$UiMode = "playtest",
    [switch]$DebugUi,
    [switch]$MinimalUi,
    [switch]$Dx11,
    [switch]$FreeCursor,
    [int]$Frames = 0,
    [string]$CaptureFrame = "",
    [string]$CaptureDir = "",
    [string]$Scene = "data\scenes\ferry_office.scene.json",
    [string]$ExecutablePath = "",
    [switch]$DryRun
)

$ErrorActionPreference = "Stop"
$Root = (Resolve-Path (Join-Path $PSScriptRoot "..")).Path

function Resolve-RepoPath([string]$Path) {
    if ([System.IO.Path]::IsPathRooted($Path)) {
        return $Path
    }
    return (Join-Path $Root $Path)
}

function Test-OptionPresent([string[]]$Items, [string[]]$Names) {
    foreach ($Item in $Items) {
        $Lower = $Item.ToLowerInvariant()
        foreach ($Name in $Names) {
            $Option = $Name.ToLowerInvariant()
            if ($Lower -eq $Option -or $Lower.StartsWith("$Option=")) {
                return $true
            }
        }
    }
    return $false
}

function Format-CommandLine([string]$Executable, [string[]]$Items) {
    $Parts = @($Executable) + $Items
    $Quoted = foreach ($Part in $Parts) {
        if ($Part -match '[\s"]') {
            '"' + ($Part -replace '"', '\"') + '"'
        } else {
            $Part
        }
    }
    return ($Quoted -join " ")
}

if ($DebugUi) {
    $UiMode = "debug"
}
if ($MinimalUi) {
    $UiMode = "minimal"
}
if ($Dx11) {
    $Renderer = "dx11"
}
if (![string]::IsNullOrWhiteSpace($CaptureFrame) -and ![string]::IsNullOrWhiteSpace($CaptureDir)) {
    Write-Host "Use either -CaptureFrame or -CaptureDir, not both."
    exit 1
}

if ([string]::IsNullOrWhiteSpace($ExecutablePath)) {
    $ExecutablePath = "build\windows-vs2022-debug\Debug\EngineApp.exe"
}

$Executable = Resolve-RepoPath $ExecutablePath
if (!(Test-Path $Executable)) {
    Write-Host "Playable executable was not found: $Executable"
    Write-Host "Run these first from the repository root:"
    Write-Host "  powershell -ExecutionPolicy Bypass -File scripts\configure.ps1"
    Write-Host "  powershell -ExecutionPolicy Bypass -File scripts\build.ps1"
    exit 1
}

$LaunchArgs = New-Object System.Collections.Generic.List[string]

if (!(Test-OptionPresent $AppArgs @("--renderer"))) {
    $LaunchArgs.Add("--renderer")
    $LaunchArgs.Add($Renderer)
}

if (!(Test-OptionPresent $AppArgs @("--ui-mode", "--playtest-ui", "--debug-ui"))) {
    $LaunchArgs.Add("--ui-mode")
    $LaunchArgs.Add($UiMode)
}

if (!(Test-OptionPresent $AppArgs @("--scene"))) {
    $LaunchArgs.Add("--scene")
    $LaunchArgs.Add($Scene)
}

if ($Frames -gt 0 -and !(Test-OptionPresent $AppArgs @("--frames"))) {
    $LaunchArgs.Add("--frames")
    $LaunchArgs.Add($Frames.ToString())
}

if (![string]::IsNullOrWhiteSpace($CaptureFrame) -and !(Test-OptionPresent $AppArgs @("--capture-frame"))) {
    $LaunchArgs.Add("--capture-frame")
    $LaunchArgs.Add($CaptureFrame)
}

if (![string]::IsNullOrWhiteSpace($CaptureDir) -and !(Test-OptionPresent $AppArgs @("--capture-dir"))) {
    $LaunchArgs.Add("--capture-dir")
    $LaunchArgs.Add($CaptureDir)
}

if ($FreeCursor -and !(Test-OptionPresent $AppArgs @("--free-cursor", "--show-cursor"))) {
    $LaunchArgs.Add("--free-cursor")
}

foreach ($Arg in $AppArgs) {
    $LaunchArgs.Add($Arg)
}

$FinalArgs = $LaunchArgs.ToArray()
Write-Host "Tidebreak playable build launch:"
Write-Host (Format-CommandLine $Executable $FinalArgs)

if ($DryRun) {
    exit 0
}

& $Executable @FinalArgs
exit $LASTEXITCODE
