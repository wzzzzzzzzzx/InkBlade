$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$Source = Join-Path $Root "src\main_win32.cpp"
$sourceText = Get-Content -LiteralPath $Source -Raw

foreach ($needle in @(
    "struct FloatingText",
    "floatingTexts",
    "hitStopTimer",
    "shakeTimer",
    "shakeStrength",
    "flashTimer",
    "addFloatingText",
    "addHitStop",
    "addHitFeedback",
    "triggerKoFeedback",
    "drawFloatingTexts",
    "drawHitFlash",
    "KO",
    "\u632f\u5200\u6210\u529f",
    "\u5e73A\u7834\u62db",
    "\u84c4\u529b\u547d\u4e2d"
)) {
    if ($sourceText -notmatch [regex]::Escape($needle)) {
        throw "Missing combat feedback marker: $needle"
    }
}

Write-Host "OK combat feedback checks passed."
