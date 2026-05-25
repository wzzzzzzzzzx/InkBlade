$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$Source = Join-Path $Root "src\main_win32.cpp"
$BodyDir = Join-Path $Root "assets\art\characters\lingshuang\sprites\body"

$requiredSprites = @(
    "normal_01.png",
    "charging_01.png",
    "charge_release_01.png",
    "skill_01.png",
    "ultimate_01.png"
)

foreach ($sprite in $requiredSprites) {
    $path = Join-Path $BodyDir $sprite
    if (!(Test-Path $path)) {
        throw "Missing Ling Shuang action sprite: $path"
    }
}

$sourceText = Get-Content -LiteralPath $Source -Raw

foreach ($needle in @(
    "ActionSpriteSlot",
    "characterSprites",
    "loadCharacterActionSprites",
    "drawCharacterSprite",
    "normal_01\.png",
    "charging_01\.png",
    "charge_release_01\.png",
    "skill_01\.png",
    "ultimate_01\.png"
)) {
    if ($sourceText -notmatch $needle) {
        throw "Missing renderer integration marker: $needle"
    }
}

foreach ($action in @(
    "Action::Normal",
    "Action::Charging",
    "Action::ChargeRelease",
    "Action::Skill",
    "Action::Ultimate"
)) {
    if ($sourceText -notmatch $action) {
        throw "Ling Shuang sprite selector does not reference $action."
    }
}

Write-Host "OK Ling Shuang action sprite table checks passed."
