param(
    [string]$Source = (Join-Path $PSScriptRoot '..\src\main_win32.cpp')
)

$ErrorActionPreference = 'Stop'
$text = Get-Content -Encoding UTF8 -Raw -Path $Source

$requiredTokens = @(
    'int characterIndex(const Fighter& f) const',
    'int weaponIndex(const Fighter& f) const',
    'void drawWeapon(HDC hdc, const Fighter& f, int x, int y, int role, int weapon)',
    'void drawRoleSlash',
    'void drawRoleCharge',
    'void drawRoleAura',
    'if (role == 0)',
    'else if (role == 1)',
    'else',
    'if (weapon == 0)',
    'else if (weapon == 1)'
)

$missing = @()
foreach ($token in $requiredTokens) {
    if (-not $text.Contains($token)) {
        $missing += $token
    }
}

if ($missing.Count -gt 0) {
    Write-Error ("Visual differentiation check failed. Missing markers: " + ($missing -join ', '))
}

Write-Host 'Visual differentiation static check passed.'
