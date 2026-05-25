param(
    [string]$Source,
    [string]$Out = "assets/art/backgrounds/main_menu/main_menu_heroes.png"
)

$ErrorActionPreference = "Stop"
if ([string]::IsNullOrWhiteSpace($Source)) {
    throw "Source image path is required."
}

Add-Type -AssemblyName System.Drawing

$root = Split-Path -Parent $PSScriptRoot
$outPath = Join-Path $root $Out
$outDir = Split-Path -Parent $outPath
New-Item -ItemType Directory -Force -Path $outDir | Out-Null

$src = [System.Drawing.Bitmap]::new($Source)
try {
    $canvas = [System.Drawing.Bitmap]::new($src.Width, $src.Height)
    try {
        $g = [System.Drawing.Graphics]::FromImage($canvas)
        try {
            $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::AntiAlias
            $g.TextRenderingHint = [System.Drawing.Text.TextRenderingHint]::AntiAliasGridFit
            $g.DrawImage($src, 0, 0, $src.Width, $src.Height)

            $titleFont = [System.Drawing.Font]::new("Microsoft YaHei UI", 88, [System.Drawing.FontStyle]::Bold, [System.Drawing.GraphicsUnit]::Pixel)
            $subFont = [System.Drawing.Font]::new("Microsoft YaHei UI", 28, [System.Drawing.FontStyle]::Regular, [System.Drawing.GraphicsUnit]::Pixel)
            $titleBrush = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(245, 232, 204))
            $subBrush = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(230, 218, 196))
            $glowBrush = [System.Drawing.SolidBrush]::new([System.Drawing.Color]::FromArgb(110, 20, 20, 20))
            $fmt = [System.Drawing.StringFormat]::new()
            $fmt.Alignment = [System.Drawing.StringAlignment]::Center
            $fmt.LineAlignment = [System.Drawing.StringAlignment]::Center
            try {
                $titleRect = [System.Drawing.RectangleF]::new(0, [single]($src.Height * 0.30), $src.Width, 120)
                $subRect = [System.Drawing.RectangleF]::new(0, [single]($src.Height * 0.415), $src.Width, 50)
                $titleText = [string]::Concat([char]0x58A8, " ", [char]0x5203)
                $subText = [string]::Concat("2D ", [char]0x6B66, [char]0x4FA0, [char]0x683C, [char]0x6597)
                $g.DrawString($titleText, $titleFont, $glowBrush, [System.Drawing.RectangleF]::new($titleRect.X + 4, $titleRect.Y + 5, $titleRect.Width, $titleRect.Height), $fmt)
                $g.DrawString($titleText, $titleFont, $titleBrush, $titleRect, $fmt)
                $g.DrawString($subText, $subFont, $glowBrush, [System.Drawing.RectangleF]::new($subRect.X + 2, $subRect.Y + 3, $subRect.Width, $subRect.Height), $fmt)
                $g.DrawString($subText, $subFont, $subBrush, $subRect, $fmt)
            }
            finally {
                $fmt.Dispose()
                $glowBrush.Dispose()
                $subBrush.Dispose()
                $titleBrush.Dispose()
                $subFont.Dispose()
                $titleFont.Dispose()
            }
        }
        finally {
            $g.Dispose()
        }
        $canvas.Save($outPath, [System.Drawing.Imaging.ImageFormat]::Png)
    }
    finally {
        $canvas.Dispose()
    }
}
finally {
    $src.Dispose()
}

Write-Host "Wrote $outPath"
