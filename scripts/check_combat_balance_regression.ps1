$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$Source = Join-Path $Root "src\main_win32.cpp"
$Exe = Join-Path $Root "build\InkBlade.exe"
$sourceText = Get-Content -LiteralPath $Source -Raw

foreach ($needle in @(
    "HudLabelPanelColor",
    "PassiveEnergyRegen",
    "HitEnergyGain",
    "DodgeStaminaCost",
    "SkillStaminaCost",
    "runCombatBalanceSelfTest",
    "--self-test-combat-balance",
    "\u7cbe\u529b"
)) {
    if ($sourceText -notmatch [regex]::Escape($needle)) {
        throw "Missing combat balance marker: $needle"
    }
}

if ($sourceText -match [regex]::Escape('L"\u4f53\u529b"')) {
    throw "HUD should use 精力 instead of 体力."
}

if (!(Test-Path -LiteralPath $Exe)) {
    throw "Missing exe: $Exe"
}

$p = Start-Process -FilePath $Exe -ArgumentList @("--self-test-combat-balance") -WorkingDirectory (Split-Path $Exe) -PassThru
try {
    if (-not $p.WaitForExit(12000)) {
        throw "Combat balance self-test did not exit."
    }
    if ($p.ExitCode -ne 0) {
        throw "Combat balance self-test failed with exit code $($p.ExitCode)."
    }
} finally {
    if (-not $p.HasExited) {
        $p.Kill()
        [void]$p.WaitForExit(2000)
    }
}

Write-Host "OK combat balance regression checks passed."
