$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$Spec = Join-Path $Root "docs\character-battle-art-spec.md"
$Source = Join-Path $Root "src\main_win32.cpp"

if (!(Test-Path $Spec)) {
    throw "Missing character battle art spec: $Spec"
}

$specText = Get-Content -LiteralPath $Spec -Raw
$sourceText = Get-Content -LiteralPath $Source -Raw

foreach ($needle in @(
    "Ling Shuang is the current quality baseline",
    "Runtime filenames are the only body sprites loaded by the game",
    "Delete disposable chroma-key sources",
    "jump_01_concept_01.png",
    "dodge_01_concept_01.png",
    "run_01_concept_01.png",
    "run_02_concept_01.png",
    "skill_01_concept_01.png",
    "ultimate_01_concept_01.png",
    "concept/*_action_pose_sheet.png",
    "ultimate_vfx_01.png",
    "assets/art/characters/mohen/sprites/body/",
    "visual_combat_test.ps1",
    "#00ff00"
)) {
    if ($specText -notmatch [regex]::Escape($needle)) {
        throw "Missing character battle art spec marker: $needle"
    }
}

foreach ($needle in @(
    "CharacterSpriteFolder",
    "CharacterVfxFolder",
    "loadCharacterActionSprites",
    "drawCharacterSprite",
    "drawUltimateVfxSprite",
    "ActionSpriteSlot::Idle",
    "ActionSpriteSlot::Ultimate"
)) {
    if ($sourceText -notmatch [regex]::Escape($needle)) {
        throw "Missing character art runtime marker: $needle"
    }
}

Write-Host "OK character battle art spec checks passed."
