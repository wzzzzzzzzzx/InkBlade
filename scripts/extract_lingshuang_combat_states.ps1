$ErrorActionPreference = "Stop"

Add-Type -AssemblyName System.Drawing

$Root = Resolve-Path "$PSScriptRoot\.."
$Source = Join-Path $Root "assets\art\characters\lingshuang\concept\lingshuang_action_pose_sheet.png"
$OutDir = Join-Path $Root "assets\art\characters\lingshuang\sprites\body"

if (!(Test-Path $Source)) {
    throw "Missing source pose sheet: $Source"
}

New-Item -ItemType Directory -Force -Path $OutDir | Out-Null

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
        $out = Join-Path $OutDir $name
        $canvas.Save($out, [System.Drawing.Imaging.ImageFormat]::Png)
        Write-Host "Extracted: $out"
    } finally {
        $canvas.Dispose()
    }
}

$src = [System.Drawing.Bitmap]::new($Source)
try {
    Export-Sprite $src "parry_01.png" `
        ([System.Drawing.Rectangle]::new(0, 500, 360, 500)) `
        ([System.Drawing.Rectangle]::new(92, 32, 584, 704))

    Export-Sprite $src "hit_01.png" `
        ([System.Drawing.Rectangle]::new(386, 500, 312, 500)) `
        ([System.Drawing.Rectangle]::new(132, 36, 504, 700))
} finally {
    $src.Dispose()
}
