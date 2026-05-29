$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$Source = Join-Path $Root "src\main_win32.cpp"
$sourceText = Get-Content -LiteralPath $Source -Raw

$weaponModels = @(
    "assets\art\weapons\longsword\sprites\model\model_01.png",
    "assets\art\weapons\broadsword\sprites\model\model_01.png",
    "assets\art\weapons\dualblades\sprites\model\model_01.png"
)

foreach ($relative in $weaponModels) {
    $path = Join-Path $Root $relative
    if (!(Test-Path -LiteralPath $path)) {
        throw "Missing weapon model sprite: $relative"
    }
}

foreach ($needle in @(
    "weaponModelSprites",
    "loadWeaponModelSprites",
    "drawWeaponSprite",
    "model_01.png",
    "if (drawWeaponSprite"
)) {
    if ($sourceText -notmatch [regex]::Escape($needle)) {
        throw "Missing weapon sprite integration marker: $needle"
    }
}

Write-Host "OK weapon sprite integration checks passed."
