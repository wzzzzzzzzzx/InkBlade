$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$Source = Join-Path $Root "src\main_win32.cpp"
$Script = Join-Path $Root "scripts\visual_combat_test.ps1"
$sourceText = Get-Content -LiteralPath $Source -Raw

foreach ($needle in @(
    "visualLabMode",
    "--visual-lab",
    "startVisualLab",
    "triggerVisualLabAction",
    "VisualLabAction::Ultimate",
    "VisualLabAction::ChargeRelease",
    "VisualLabAction::Hit",
    "VisualLabAction::Skill",
    "VisualLabAction::Normal",
    "VisualLabAction::Charging",
    "VisualLabAction::Jump",
    "VisualLabAction::Parry",
    "VisualLabAction::Dodge",
    "visualLabMode = true",
    "practiceEnemyAttacks = false"
)) {
    if ($sourceText -notmatch $needle) {
        throw "Missing visual lab source marker: $needle"
    }
}

if (!(Test-Path $Script)) {
    throw "Missing visual combat test script: $Script"
}

$scriptText = Get-Content -LiteralPath $Script -Raw
foreach ($needle in @(
    "--visual-lab",
    "visual_lab_idle.png",
    "visual_lab_run.png",
    "visual_lab_ultimate.png",
    "visual_lab_charge_release.png",
    "visual_lab_hit.png",
    "visual_lab_skill.png",
    "visual_lab_normal.png",
    "visual_lab_charging.png",
    "visual_lab_jump.png",
    "visual_lab_parry.png",
    "visual_lab_dodge.png"
)) {
    if ($scriptText -notmatch $needle) {
        throw "Missing visual combat test script marker: $needle"
    }
}

Write-Host "OK visual lab mode checks passed."
