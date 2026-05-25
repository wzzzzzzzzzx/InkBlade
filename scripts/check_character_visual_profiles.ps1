$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$Source = Join-Path $Root "src\main_win32.cpp"
$VisualLab = Join-Path $Root "scripts\visual_combat_test.ps1"
$ConceptExtractor = Join-Path $Root "scripts\extract_concept_action_sprites.py"
$SourceText = Get-Content -LiteralPath $Source -Raw
$VisualLabText = Get-Content -LiteralPath $VisualLab -Raw
$RequiredBodySprites = @(
    "idle_01.png",
    "run_01.png",
    "run_02.png",
    "jump_01.png",
    "dodge_01.png",
    "parry_01.png",
    "normal_01.png",
    "charging_01.png",
    "charge_release_01.png",
    "hit_01.png",
    "skill_01.png",
    "ultimate_01.png"
)
$RequiredConceptSprites = @(
    "idle_01_concept_01.png",
    "run_01_concept_01.png",
    "run_02_concept_01.png",
    "jump_01_concept_01.png",
    "dodge_01_concept_01.png",
    "normal_01_concept_01.png",
    "charging_01_concept_01.png",
    "charge_release_01_concept_01.png",
    "hit_01_concept_01.png",
    "skill_01_concept_01.png",
    "ultimate_01_concept_01.png"
)
$RequiredVfxSprites = @("ultimate_vfx_01.png")

if (!(Test-Path $ConceptExtractor)) {
    throw "Missing concept action sprite extractor: $ConceptExtractor"
}

foreach ($needle in @(
    "enum class ActionSpriteSlot",
    "struct ActionSpriteLayout",
    "characterSprites",
    "characterUltimateVfx",
    "CharacterSpriteFolder",
    "CharacterVfxFolder",
    "loadCharacterActionSprites",
    "loadAllCharacterActionSprites",
    "drawCharacterSprite",
    "drawUltimateVfxSprite",
    "actionSpriteFor",
    "spriteLayoutFor"
)) {
    if ($SourceText -notmatch $needle) {
        throw "Missing character visual profile marker: $needle"
    }
}

foreach ($legacy in @(
    "LingShuangSpriteSlot",
    "lingshuangSprites",
    "loadLingShuangActionSprites",
    "drawLingShuangSprite"
)) {
    if ($SourceText -match $legacy) {
        throw "Legacy Ling Shuang-only sprite marker remains: $legacy"
    }
}

foreach ($folder in @(
    "characters\\lingshuang\\sprites\\body\\",
    "characters\\mohen\\sprites\\body\\",
    "characters\\suxin\\sprites\\body\\"
)) {
    if ($SourceText -notmatch [regex]::Escape($folder)) {
        throw "Missing character sprite folder reference: $folder"
    }
}

foreach ($character in @("mohen", "suxin")) {
    $bodyDir = Join-Path $Root "assets\art\characters\$character\sprites\body"
    foreach ($sprite in $RequiredBodySprites) {
        $path = Join-Path $bodyDir $sprite
        if (!(Test-Path $path)) {
            throw "Missing $character body sprite: $path"
        }
    }
    foreach ($sprite in $RequiredConceptSprites) {
        $path = Join-Path $bodyDir $sprite
        if (!(Test-Path $path)) {
            throw "Missing $character concept-extracted sprite: $path"
        }
    }
    $vfxDir = Join-Path $Root "assets\art\characters\$character\sprites\vfx"
    foreach ($sprite in $RequiredVfxSprites) {
        $path = Join-Path $vfxDir $sprite
        if (!(Test-Path $path)) {
            throw "Missing $character vfx sprite: $path"
        }
    }
}

foreach ($needle in @(
    "--visual-lab-character",
    "Run-VisualLabCase",
    "characterIndex -lt 3",
    "visual_lab_char{0}_idle.png",
    "visual_lab_char{0}_run.png",
    "visual_lab_char{0}_ultimate.png",
    "visual_lab_char{0}_charge_release.png",
    "visual_lab_char{0}_hit.png",
    "visual_lab_char{0}_skill.png",
    "visual_lab_char{0}_normal.png",
    "visual_lab_char{0}_charging.png",
    "visual_lab_char{0}_jump.png",
    "visual_lab_char{0}_parry.png"
)) {
    if ($VisualLabText -notmatch [regex]::Escape($needle)) {
        throw "Missing visual lab multi-character marker: $needle"
    }
}

Write-Host "OK character visual profile checks passed."
