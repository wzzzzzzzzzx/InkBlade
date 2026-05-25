$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$Exe = "$Root\build\InkBlade.exe"
$Out = "$Root\dist\InkBlade"

if (!(Test-Path $Exe)) {
    & "$PSScriptRoot\build.ps1"
}

if (Test-Path $Out) {
    Remove-Item -LiteralPath $Out -Recurse -Force
}

New-Item -ItemType Directory -Path $Out | Out-Null
Copy-Item -LiteralPath $Exe -Destination "$Out\InkBlade.exe"
Copy-Item -LiteralPath "$Root\assets" -Destination "$Out\assets" -Recurse
Copy-Item -LiteralPath "$Root\config" -Destination "$Out\config" -Recurse
Copy-Item -LiteralPath "$Root\README.md" -Destination "$Out\README.md"
Write-Host "Packaged: $Out"
