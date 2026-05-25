$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$Source = Join-Path $Root "src\main_win32.cpp"
$sourceText = Get-Content -LiteralPath $Source -Raw

foreach ($needle in @(
    "LingShuangSpriteLayout",
    "lingShuangSpriteLayout",
    "mirrorFromSource",
    "xOffset",
    "yOffset",
    "drawBitmapAlpha"
)) {
    if ($sourceText -notmatch $needle) {
        throw "Missing Ling Shuang layout marker: $needle"
    }
}

if ($sourceText -notmatch "LingShuangSpriteSlot::Skill[\s\S]*560[\s\S]*320") {
    throw "Ling Shuang skill sprite does not use a wide action layout."
}

if ($sourceText -notmatch "LingShuangSpriteSlot::Ultimate[\s\S]*640[\s\S]*360") {
    throw "Ling Shuang ultimate sprite does not use a large action layout."
}

if ($sourceText -notmatch "LingShuangSpriteSlot::ChargeRelease[\s\S]*420[\s\S]*360") {
    throw "Ling Shuang charge release sprite does not use an enlarged action layout."
}

Write-Host "OK Ling Shuang action sprite layout checks passed."
