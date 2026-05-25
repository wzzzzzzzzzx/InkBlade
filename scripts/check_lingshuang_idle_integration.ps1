$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$Source = Join-Path $Root "src\main_win32.cpp"
$CMake = Join-Path $Root "CMakeLists.txt"
$Asset = Join-Path $Root "assets\art\characters\lingshuang\sprites\body\idle_01.png"

if (!(Test-Path $Asset)) {
    throw "Missing Ling Shuang idle sprite: $Asset"
}

$sourceText = Get-Content -LiteralPath $Source -Raw
$cmakeText = Get-Content -LiteralPath $CMake -Raw

if ($sourceText -notmatch "idle_01\.png") {
    throw "Win32 renderer does not reference Ling Shuang idle_01.png."
}

if ($sourceText -notmatch "AlphaBlend") {
    throw "Win32 renderer does not alpha-blend transparent character PNGs."
}

if ($sourceText -notmatch "drawBitmapAlpha") {
    throw "Win32 renderer does not have a reusable alpha bitmap drawing path."
}

if ($cmakeText -notmatch "msimg32") {
    throw "CMake does not link msimg32 for AlphaBlend."
}

Write-Host "OK Ling Shuang idle PNG integration checks passed."

