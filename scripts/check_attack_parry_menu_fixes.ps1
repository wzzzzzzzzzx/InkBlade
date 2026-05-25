$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$Source = Join-Path $Root "src\main_win32.cpp"
$MenuBg = Join-Path $Root "assets\art\backgrounds\main_menu\main_menu_heroes.png"
$sourceText = Get-Content -LiteralPath $Source -Raw

if (!(Test-Path $MenuBg)) {
    throw "Missing main menu hero background image: $MenuBg"
}

foreach ($needle in @(
    "menuBackground",
    "main_menu_heroes\.png",
    "drawMainMenuBackgroundImage",
    "QuickAttackThreshold",
    "f\.chargeTimer < QuickAttackThreshold",
    "normal\(f\)",
    "lastMoveFacing",
    "f\.facing = f\.lastMoveFacing",
    "ActionSpriteSlot::Parry"
)) {
    if ($sourceText -notmatch $needle) {
        throw "Missing attack/parry/menu fix marker: $needle"
    }
}

Write-Host "OK quick attack, parry facing, and menu background checks passed."
