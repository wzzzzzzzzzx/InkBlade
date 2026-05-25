$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$Source = Join-Path $Root "src\main_win32.cpp"
$Ground = Join-Path $Root "assets\art\backgrounds\bamboo_moon\layers\ground_strip.png"
$sourceText = Get-Content -LiteralPath $Source -Raw

if (!(Test-Path $Ground)) {
    throw "Missing extracted arena ground image: $Ground"
}

foreach ($needle in @(
    "ArenaLeft",
    "ArenaRight",
    "ArenaTop",
    "ArenaBottom",
    "clampFighterToArena",
    "drawArenaGround",
    "battleGround",
    "ground_strip\.png"
)) {
    if ($sourceText -notmatch $needle) {
        throw "Missing arena boundary or ground marker: $needle"
    }
}

if ($sourceText -notmatch "Screen::Practice") {
    throw "Practice settings screen is not defined."
}

foreach ($needle in @(
    "practiceMode",
    "practiceEnemyAttacks",
    "practiceFullEnergy",
    "drawPracticeSettings",
    "if \(!practiceMode \|\| practiceEnemyAttacks\)",
    "player\.energy = 100\.f",
    "ai\.energy = 100\.f"
)) {
    if ($sourceText -notmatch $needle) {
        throw "Missing practice-mode marker: $needle"
    }
}

Write-Host "OK arena boundary, ground, and practice-mode checks passed."
