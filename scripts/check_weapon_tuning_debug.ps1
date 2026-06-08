$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$Source = Join-Path $Root "src\main_win32.cpp"
$Exe = Join-Path $Root "build\InkBlade.exe"
$sourceText = Get-Content -LiteralPath $Source -Raw

foreach ($needle in @(
    "WeaponActionTuning",
    "WeaponTunings",
    "attackProfile",
    "practiceDebugHitboxes",
    "drawCombatDebug",
    "outlineRect",
    "rangeCenterY",
    "weaponLayoutsDistinct",
    "--self-test-weapon-tuning"
)) {
    if ($sourceText -notmatch [regex]::Escape($needle)) {
        throw "Missing weapon tuning/debug marker: $needle"
    }
}

if (!(Test-Path -LiteralPath $Exe)) {
    throw "Missing exe: $Exe"
}

$p = Start-Process -FilePath $Exe -ArgumentList @("--self-test-weapon-tuning") -WorkingDirectory (Split-Path $Exe) -PassThru
try {
    if (-not $p.WaitForExit(12000)) {
        throw "Weapon tuning self-test did not exit."
    }
    if ($p.ExitCode -ne 0) {
        throw "Weapon tuning self-test failed with exit code $($p.ExitCode)."
    }
} finally {
    if (-not $p.HasExited) {
        $p.Kill()
        [void]$p.WaitForExit(2000)
    }
}

Write-Host "OK weapon tuning/debug checks passed."
