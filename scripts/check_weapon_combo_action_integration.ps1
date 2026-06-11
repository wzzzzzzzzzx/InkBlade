$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$Source = Join-Path $Root "src\main_win32.cpp"
$sourceText = Get-Content -LiteralPath $Source -Raw

$characters = @("lingshuang", "mohen", "suxin")
$weapons = @("longsword", "broadsword", "dualblades")

Add-Type -AssemblyName System.Drawing

foreach ($character in $characters) {
    foreach ($weapon in $weapons) {
        $expectedSize = $null
        for ($stage = 1; $stage -le 3; $stage++) {
            $relative = "assets\art\characters\$character\sprites\attack\$weapon\normal_0$stage.png"
            $path = Join-Path $Root $relative
            if (!(Test-Path -LiteralPath $path)) {
                throw "Missing weapon combo action sprite: $relative"
            }

            $bitmap = [System.Drawing.Bitmap]::FromFile($path)
            try {
                if (($bitmap.PixelFormat -band [System.Drawing.Imaging.PixelFormat]::Alpha) -eq 0) {
                    throw "Weapon combo sprite has no alpha channel: $relative"
                }
                if ($bitmap.Width -lt 300 -or $bitmap.Height -lt 300) {
                    throw "Weapon combo sprite canvas is unexpectedly small: $relative ($($bitmap.Width)x$($bitmap.Height))"
                }
                if ($null -eq $expectedSize) {
                    $expectedSize = "$($bitmap.Width)x$($bitmap.Height)"
                } elseif ($expectedSize -ne "$($bitmap.Width)x$($bitmap.Height)") {
                    throw "Combo stages must share one stable canvas: $character/$weapon"
                }
                if ($bitmap.GetPixel(0, 0).A -ne 0) {
                    throw "Weapon combo sprite corner is not transparent: $relative"
                }
            } finally {
                $bitmap.Dispose()
            }
        }
    }
}

foreach ($needle in @(
    "weaponComboSprites",
    "loadWeaponComboSprites",
    "drawWeaponComboSprite",
    "normalStage",
    'L"normal_0"',
    "if (f.action == Action::Normal)",
    "const bool drewWeaponCombo"
)) {
    if ($sourceText -notmatch [regex]::Escape($needle)) {
        throw "Missing weapon combo integration marker: $needle"
    }
}

$weaponLines = Get-Content -LiteralPath $Source -Encoding UTF8 | Where-Object {
    $_ -match '^\s*\{L"[^"]+", L"[^"]+", \{([^}]*)\}, \{'
}
if ($weaponLines.Count -lt 3) {
    throw "Could not identify all three weapon definitions."
}

foreach ($definition in $weaponLines | Select-Object -First 3) {
    $combo = [regex]::Match($definition, '\{([^{}]+)\}, \{').Groups[1].Value
    if (($combo -split ',').Count -ne 3) {
        throw "Every weapon must expose exactly three normal-attack stages: $definition"
    }
}

Write-Host "OK weapon combo action integration checks passed."
