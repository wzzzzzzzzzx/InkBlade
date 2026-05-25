$ErrorActionPreference = "Stop"

Add-Type -AssemblyName System.Drawing

$Root = Resolve-Path "$PSScriptRoot\.."
$ActionSource = Join-Path $Root "assets\art\characters\lingshuang\concept\lingshuang_action_pose_sheet.png"
$VfxSource = Join-Path $Root "assets\art\characters\lingshuang\concept\lingshuang_skill_vfx_sheet.png"
$OutDir = Join-Path $Root "assets\art\characters\lingshuang\sprites\body"

foreach ($source in @($ActionSource, $VfxSource)) {
    if (!(Test-Path $source)) {
        throw "Missing source sheet: $source"
    }
}

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

$onlyNames = @()
if ($env:INKBLADE_EXTRACT_ONLY) {
    $onlyNames = $env:INKBLADE_EXTRACT_ONLY.Split(",") | ForEach-Object { $_.Trim() } | Where-Object { $_.Length -gt 0 }
}

function Test-BackgroundPixel([System.Drawing.Color] $c) {
    $max = [Math]::Max($c.R, [Math]::Max($c.G, $c.B))
    $min = [Math]::Min($c.R, [Math]::Min($c.G, $c.B))
    return ($c.A -eq 0 -or ($c.R -ge 220 -and $c.G -ge 220 -and $c.B -ge 220 -and ($max - $min) -le 45))
}

function Remove-ConnectedBackground([System.Drawing.Bitmap] $canvas) {
    $w = $canvas.Width
    $h = $canvas.Height
    $visited = New-Object 'bool[,]' $w, $h
    $queue = [System.Collections.Generic.Queue[System.Drawing.Point]]::new()

    function Try-Enqueue([int] $x, [int] $y) {
        if ($x -lt 0 -or $y -lt 0 -or $x -ge $w -or $y -ge $h -or $visited[$x, $y]) {
            return
        }
        if (Test-BackgroundPixel $canvas.GetPixel($x, $y)) {
            $visited[$x, $y] = $true
            $queue.Enqueue([System.Drawing.Point]::new($x, $y))
        }
    }

    for ($x = 0; $x -lt $w; $x++) {
        Try-Enqueue $x 0
        Try-Enqueue $x ($h - 1)
    }
    for ($y = 0; $y -lt $h; $y++) {
        Try-Enqueue 0 $y
        Try-Enqueue ($w - 1) $y
    }

    while ($queue.Count -gt 0) {
        $p = $queue.Dequeue()
        $canvas.SetPixel($p.X, $p.Y, [System.Drawing.Color]::FromArgb(0, 255, 255, 255))
        Try-Enqueue ($p.X + 1) $p.Y
        Try-Enqueue ($p.X - 1) $p.Y
        Try-Enqueue $p.X ($p.Y + 1)
        Try-Enqueue $p.X ($p.Y - 1)
    }
}

function Export-Sprite(
    [System.Drawing.Bitmap] $src,
    [string] $name,
    [System.Drawing.Rectangle] $crop,
    [System.Drawing.Rectangle] $dest
) {
    if ($onlyNames.Count -gt 0 -and $onlyNames -notcontains $name) {
        return
    }

    $out = Join-Path $OutDir $name
    if ((Test-Path $out) -and $env:INKBLADE_FORCE_EXTRACT -ne "1") {
        Write-Host "Skipped existing: $out"
        return
    }

    $canvasSize = 768
    $canvas = [System.Drawing.Bitmap]::new($canvasSize, $canvasSize, [System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
    try {
        $g = [System.Drawing.Graphics]::FromImage($canvas)
        try {
            $g.Clear([System.Drawing.Color]::FromArgb(0, 255, 255, 255))
            $g.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
            $g.SmoothingMode = [System.Drawing.Drawing2D.SmoothingMode]::HighQuality
            $g.PixelOffsetMode = [System.Drawing.Drawing2D.PixelOffsetMode]::HighQuality
            $g.DrawImage($src, $dest, $crop, [System.Drawing.GraphicsUnit]::Pixel)
        } finally {
            $g.Dispose()
        }

        Remove-ConnectedBackground $canvas
        $canvas.Save($out, [System.Drawing.Imaging.ImageFormat]::Png)
        Write-Host "Extracted: $out"
    } finally {
        $canvas.Dispose()
    }
}

$action = [System.Drawing.Bitmap]::new($ActionSource)
$vfx = [System.Drawing.Bitmap]::new($VfxSource)
try {
    Export-Sprite $action "normal_01.png" `
        ([System.Drawing.Rectangle]::new(404, 76, 330, 405)) `
        ([System.Drawing.Rectangle]::new(100, 106, 568, 568))

    Export-Sprite $action "charging_01.png" `
        ([System.Drawing.Rectangle]::new(0, 500, 360, 500)) `
        ([System.Drawing.Rectangle]::new(92, 32, 584, 704))

    Export-Sprite $action "charge_release_01.png" `
        ([System.Drawing.Rectangle]::new(706, 510, 318, 500)) `
        ([System.Drawing.Rectangle]::new(124, 32, 520, 704))

    Export-Sprite $vfx "skill_01.png" `
        ([System.Drawing.Rectangle]::new(410, 42, 455, 305)) `
        ([System.Drawing.Rectangle]::new(82, 158, 604, 404))

    Export-Sprite $vfx "ultimate_01.png" `
        ([System.Drawing.Rectangle]::new(600, 660, 842, 360)) `
        ([System.Drawing.Rectangle]::new(42, 204, 684, 292))
} finally {
    $action.Dispose()
    $vfx.Dispose()
}
