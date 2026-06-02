$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$Source = Join-Path $Root "src\main_win32.cpp"
$Exe = Join-Path $Root "build\InkBlade.exe"
$sourceText = Get-Content -LiteralPath $Source -Raw

foreach ($needle in @(
    "UltimateEnergyColor",
    "StaminaBarColor",
    "runUltimateEnergySelfTest",
    "--self-test-ultimate-energy",
    "\u5927\u62db\u80fd\u91cf",
    "\u4f53\u529b"
)) {
    if ($sourceText -notmatch [regex]::Escape($needle)) {
        throw "Missing ultimate energy marker: $needle"
    }
}

if (!(Test-Path -LiteralPath $Exe)) {
    throw "Missing exe: $Exe"
}

$p = Start-Process -FilePath $Exe -ArgumentList @("--self-test-ultimate-energy") -WorkingDirectory (Split-Path $Exe) -PassThru
try {
    if (-not $p.WaitForExit(4000)) {
        throw "Ultimate energy self-test did not exit."
    }
    if ($p.ExitCode -ne 0) {
        throw "Ultimate energy self-test failed with exit code $($p.ExitCode)."
    }
} finally {
    if (-not $p.HasExited) {
        $p.Kill()
        [void]$p.WaitForExit(2000)
    }
}

Write-Host "OK ultimate energy checks passed for all characters."
