param(
    [string]$Exe = (Join-Path $PSScriptRoot "..\build\InkBlade.exe")
)

$ErrorActionPreference = "Stop"

if (!(Test-Path $Exe)) {
    throw "Missing exe: $Exe"
}

$p = Start-Process -FilePath $Exe -ArgumentList "--self-test-death" -WorkingDirectory (Split-Path $Exe) -PassThru
try {
    if (-not $p.WaitForExit(4000)) {
        throw "Death self-test did not exit. The executable likely does not support --self-test-death."
    }
    if ($p.ExitCode -ne 0) {
        throw "Death self-test failed with exit code $($p.ExitCode)."
    }
} finally {
    if (-not $p.HasExited) {
        $p.Kill()
        [void]$p.WaitForExit(2000)
    }
}

Write-Host "OK Suxin death resolution checks passed."
