$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$Source = Join-Path $Root "src\main_win32.cpp"
$CMake = Join-Path $Root "CMakeLists.txt"
$Config = Join-Path $Root "config\art_assets.json"
$Asset = Join-Path $Root "assets\art\backgrounds\bamboo_moon\concept\bamboo_moon_background_concept.png"

if (!(Test-Path $Asset)) {
    throw "Missing bamboo moon background asset: $Asset"
}

$sourceText = Get-Content -LiteralPath $Source -Raw
$cmakeText = Get-Content -LiteralPath $CMake -Raw
$config = Get-Content -LiteralPath $Config -Raw | ConvertFrom-Json

if ($sourceText -notmatch "wincodec\.h") {
    throw "Win32 renderer does not include WIC PNG loading support."
}

if ($sourceText -notmatch "bamboo_moon_background_concept\.png") {
    throw "Win32 renderer does not reference the bamboo moon PNG asset."
}

if ($sourceText -notmatch "StretchBlt") {
    throw "Win32 renderer does not draw the loaded background bitmap."
}

if ($cmakeText -notmatch "windowscodecs" -or $cmakeText -notmatch "ole32") {
    throw "CMake does not link WIC dependencies windowscodecs and ole32."
}

if ($config.backgrounds.bamboo_moon.concept -ne "assets/art/backgrounds/bamboo_moon/concept/") {
    throw "art_assets.json bamboo_moon concept path is not configured."
}

Write-Host "OK background PNG integration checks passed."

