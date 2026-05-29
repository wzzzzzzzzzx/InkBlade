$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$Source = Join-Path $Root "src\main_win32.cpp"
$Exe = Join-Path $Root "build\InkBlade.exe"
$sourceText = Get-Content -LiteralPath $Source -Raw

foreach ($needle in @(
    "playerRounds",
    "aiRounds",
    "currentRound",
    "matchComplete",
    "resetMatchState",
    "continueAfterResult",
    "runMatchFlowSelfTest",
    "--self-test-match-flow"
)) {
    if ($sourceText -notmatch [regex]::Escape($needle)) {
        throw "Missing match flow marker: $needle"
    }
}

if (!(Test-Path -LiteralPath $Exe)) {
    throw "Missing exe: $Exe"
}

$p = Start-Process -FilePath $Exe -ArgumentList "--self-test-match-flow" -WorkingDirectory (Split-Path $Exe) -PassThru
try {
    if (-not $p.WaitForExit(4000)) {
        throw "Match flow self-test did not exit."
    }
    if ($p.ExitCode -ne 0) {
        throw "Match flow self-test failed with exit code $($p.ExitCode)."
    }
} finally {
    if (-not $p.HasExited) {
        $p.Kill()
        [void]$p.WaitForExit(2000)
    }
}

Write-Host "OK match round flow checks passed."
