$ErrorActionPreference = "Stop"

Add-Type -AssemblyName System.Drawing

$Root = Resolve-Path "$PSScriptRoot\.."
$Source = Join-Path $Root "assets\art\backgrounds\bamboo_moon\concept\bamboo_moon_background_concept.png"
$OutDir = Join-Path $Root "assets\art\backgrounds\bamboo_moon\layers"
$Out = Join-Path $OutDir "ground_strip.png"

if (!(Test-Path $Source)) {
    throw "Missing battle background: $Source"
}

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$src = [System.Drawing.Bitmap]::new($Source)
try {
    $cropY = [Math]::Max(0, $src.Height - 245)
    $crop = [System.Drawing.Rectangle]::new(0, $cropY, $src.Width, $src.Height - $cropY)
    $canvas = [System.Drawing.Bitmap]::new($src.Width, 320, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    try {
        $g = [System.Drawing.Graphics]::FromImage($canvas)
        try {
            $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
            $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
            $g.DrawImage($src, [System.Drawing.Rectangle]::new(0, 0, $canvas.Width, $canvas.Height), $crop, [System.Drawing.GraphicsUnit]::Pixel)
        } finally {
            $g.Dispose()
        }
        $canvas.Save($Out, [System.Drawing.Imaging.ImageFormat]::Png)
        Write-Host "Extracted ground strip: $Out"
    } finally {
        $canvas.Dispose()
    }
} finally {
    $src.Dispose()
}
