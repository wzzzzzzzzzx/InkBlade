param(
    [string]$Source = "assets/art/backgrounds/bamboo_moon/concept/bamboo_moon_background_concept.png",
    [string]$Out = "assets/art/backgrounds/bamboo_moon/layers/ground_strip.png"
)

Add-Type -AssemblyName System.Drawing

$root = Split-Path -Parent $PSScriptRoot
$sourcePath = Join-Path $root $Source
$outPath = Join-Path $root $Out
$outDir = Split-Path -Parent $outPath

if (!(Test-Path $sourcePath)) {
    throw "Missing source background: $sourcePath"
}

New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$src = [System.Drawing.Bitmap]::new($sourcePath)
try {
    $cropY = [Math]::Min(600, [Math]::Max(0, $src.Height - 1))
    $crop = [System.Drawing.Rectangle]::new(0, $cropY, $src.Width, $src.Height - $cropY)
    $dst = [System.Drawing.Bitmap]::new($crop.Width, $crop.Height)
    try {
        $g = [System.Drawing.Graphics]::FromImage($dst)
        try {
            $g.DrawImage($src, [System.Drawing.Rectangle]::new(0, 0, $dst.Width, $dst.Height), $crop, [System.Drawing.GraphicsUnit]::Pixel)
        }
        finally {
            $g.Dispose()
        }
        $dst.Save($outPath, [System.Drawing.Imaging.ImageFormat]::Png)
    }
    finally {
        $dst.Dispose()
    }
}
finally {
    $src.Dispose()
}

Write-Host "Wrote $outPath"
