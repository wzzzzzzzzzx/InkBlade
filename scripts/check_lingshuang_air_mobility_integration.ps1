$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$Source = Join-Path $Root "src\main_win32.cpp"
$CMake = Join-Path $Root "CMakeLists.txt"
$Jump = Join-Path $Root "assets\art\characters\lingshuang\sprites\body\jump_01.png"
$Dodge = Join-Path $Root "assets\art\characters\lingshuang\sprites\body\dodge_01.png"

if (!(Test-Path $Jump)) {
    throw "Missing Ling Shuang jump sprite: $Jump"
}

if (!(Test-Path $Dodge)) {
    throw "Missing Ling Shuang dodge sprite: $Dodge"
}

$sourceText = Get-Content -LiteralPath $Source -Raw
$cmakeText = Get-Content -LiteralPath $CMake -Raw

if ($sourceText -notmatch "jump_01\.png" -or $sourceText -notmatch "dodge_01\.png") {
    throw "Win32 renderer does not reference Ling Shuang jump_01.png and dodge_01.png."
}

if ($sourceText -notmatch "lingshuangJump" -or $sourceText -notmatch "lingshuangDodge") {
    throw "Win32 renderer does not cache Ling Shuang jump and dodge sprites."
}

if ($sourceText -notmatch "f\.airborne" -or $sourceText -notmatch "Action::Dodge") {
    throw "Win32 renderer does not select Ling Shuang sprites from airborne and dodge states."
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

Write-Host "OK Ling Shuang jump/dodge PNG integration checks passed."

