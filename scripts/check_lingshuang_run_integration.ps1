$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$Source = Join-Path $Root "src\main_win32.cpp"
$CMake = Join-Path $Root "CMakeLists.txt"
$Run1 = Join-Path $Root "assets\art\characters\lingshuang\sprites\body\run_01.png"
$Run2 = Join-Path $Root "assets\art\characters\lingshuang\sprites\body\run_02.png"

if (!(Test-Path $Run1)) {
    throw "Missing Ling Shuang run sprite: $Run1"
}

if (!(Test-Path $Run2)) {
    throw "Missing Ling Shuang run sprite: $Run2"
}

$sourceText = Get-Content -LiteralPath $Source -Raw
$cmakeText = Get-Content -LiteralPath $CMake -Raw

if ($sourceText -notmatch "run_01\.png" -or $sourceText -notmatch "run_02\.png") {
    throw "Win32 renderer does not reference Ling Shuang run_01.png and run_02.png."
}

if ($sourceText -notmatch "drawLingShuangSprite") {
    throw "Win32 renderer does not route Ling Shuang body sprites through a reusable drawLingShuangSprite path."
}

if ($sourceText -notmatch "mirrorLingShuangSprite") {
    throw "Win32 renderer does not mirror left-facing Ling Shuang run sprites when the fighter faces right."
}

if ($sourceText -notmatch "input\.move\.x") {
    throw "Win32 renderer does not select run sprites based on movement input."
}

if ($sourceText -notmatch "AlphaBlend") {
    throw "Win32 renderer does not alpha-blend transparent character PNGs."
}

if ($cmakeText -notmatch "msimg32") {
    throw "CMake does not link msimg32 for AlphaBlend."
}

Write-Host "OK Ling Shuang run PNG integration checks passed."
