$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$Source = Join-Path $Root "src\main_win32.cpp"
$CMake = Join-Path $Root "CMakeLists.txt"
$Parry = Join-Path $Root "assets\art\characters\lingshuang\sprites\body\parry_01.png"
$Hit = Join-Path $Root "assets\art\characters\lingshuang\sprites\body\hit_01.png"

if (!(Test-Path $Parry)) {
    throw "Missing Ling Shuang parry sprite: $Parry"
}

if (!(Test-Path $Hit)) {
    throw "Missing Ling Shuang hit sprite: $Hit"
}

$sourceText = Get-Content -LiteralPath $Source -Raw
$cmakeText = Get-Content -LiteralPath $CMake -Raw

if ($sourceText -notmatch "parry_01\.png" -or $sourceText -notmatch "hit_01\.png") {
    throw "Win32 renderer does not reference Ling Shuang parry_01.png and hit_01.png."
}

if ($sourceText -notmatch "lingshuangParry" -or $sourceText -notmatch "lingshuangHit") {
    throw "Win32 renderer does not cache Ling Shuang parry and hit sprites."
}

if ($sourceText -notmatch "Action::Parry" -or $sourceText -notmatch "Action::Hit") {
    throw "Win32 renderer does not select Ling Shuang sprites from parry and hit states."
}

if ($sourceText -notmatch "drawLingShuangSprite") {
    throw "Win32 renderer does not route Ling Shuang body sprites through drawLingShuangSprite."
}

if ($sourceText -notmatch "AlphaBlend") {
    throw "Win32 renderer does not alpha-blend transparent character PNGs."
}

if ($cmakeText -notmatch "msimg32") {
    throw "CMake does not link msimg32 for AlphaBlend."
}

Write-Host "OK Ling Shuang parry/hit PNG integration checks passed."
