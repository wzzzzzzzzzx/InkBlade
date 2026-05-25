$ErrorActionPreference = "Stop"

Add-Type -AssemblyName System.Drawing

$Root = Resolve-Path "$PSScriptRoot\.."

$jobs = @(
    @{
        Character = "mohen"
        Sheet = Join-Path $Root "assets\art\characters\mohen\concept\mohen_action_pose_sheet.png"
        Outputs = @{
            "idle_01.png" = 0
            "hit_01.png" = 5
            "skill_01.png" = 6
            "ultimate_01.png" = 7
        }
    },
    @{
        Character = "suxin"
        Sheet = Join-Path $Root "assets\art\characters\suxin\concept\suxin_action_pose_sheet.png"
        Outputs = @{
            "idle_01.png" = 0
            "hit_01.png" = 5
            "skill_01.png" = 6
            "ultimate_01.png" = 7
        }
    }
)

function New-ArgbBitmap([int]$width, [int]$height) {
    return New-Object System.Drawing.Bitmap $width, $height, ([System.Drawing.Imaging.PixelFormat]::Format32bppArgb)
}

function Is-BackgroundCandidate([System.Drawing.Color]$c) {
    $max = [Math]::Max($c.R, [Math]::Max($c.G, $c.B))
    $min = [Math]::Min($c.R, [Math]::Min($c.G, $c.B))
    return ($c.R -ge 222 -and $c.G -ge 222 -and $c.B -ge 222 -and ($max - $min) -le 34)
}

function Copy-Crop([System.Drawing.Bitmap]$source, [System.Drawing.Rectangle]$rect) {
    $crop = New-ArgbBitmap $rect.Width $rect.Height
    $gfx = [System.Drawing.Graphics]::FromImage($crop)
    $gfx.DrawImage($source, [System.Drawing.Rectangle]::new(0, 0, $rect.Width, $rect.Height), $rect, [System.Drawing.GraphicsUnit]::Pixel)
    $gfx.Dispose()
    return $crop
}

function Remove-ConnectedWhiteBackground([System.Drawing.Bitmap]$bmp) {
    $w = $bmp.Width
    $h = $bmp.Height
    $visited = New-Object 'bool[,]' $w, $h
    $queue = New-Object System.Collections.Generic.Queue[object]

    function Try-Enqueue([int]$x, [int]$y) {
        if ($x -lt 0 -or $x -ge $w -or $y -lt 0 -or $y -ge $h) { return }
        if ($visited[$x, $y]) { return }
        $c = $bmp.GetPixel($x, $y)
        if (Is-BackgroundCandidate $c) {
            $visited[$x, $y] = $true
            $queue.Enqueue(@($x, $y))
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
        $x = [int]$p[0]
        $y = [int]$p[1]
        Try-Enqueue ($x + 1) $y
        Try-Enqueue ($x - 1) $y
        Try-Enqueue $x ($y + 1)
        Try-Enqueue $x ($y - 1)
    }

    for ($y = 0; $y -lt $h; $y++) {
        for ($x = 0; $x -lt $w; $x++) {
            if ($visited[$x, $y]) {
                $c = $bmp.GetPixel($x, $y)
                $bmp.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(0, $c.R, $c.G, $c.B))
            }
        }
    }
}

function Find-OpaqueBounds([System.Drawing.Bitmap]$bmp, [int]$padding) {
    $minX = $bmp.Width
    $minY = $bmp.Height
    $maxX = -1
    $maxY = -1
    for ($y = 0; $y -lt $bmp.Height; $y++) {
        for ($x = 0; $x -lt $bmp.Width; $x++) {
            if ($bmp.GetPixel($x, $y).A -gt 10) {
                $minX = [Math]::Min($minX, $x)
                $minY = [Math]::Min($minY, $y)
                $maxX = [Math]::Max($maxX, $x)
                $maxY = [Math]::Max($maxY, $y)
            }
        }
    }
    if ($maxX -lt $minX -or $maxY -lt $minY) {
        return [System.Drawing.Rectangle]::new(0, 0, $bmp.Width, $bmp.Height)
    }
    $x0 = [Math]::Max(0, $minX - $padding)
    $y0 = [Math]::Max(0, $minY - $padding)
    $x1 = [Math]::Min($bmp.Width - 1, $maxX + $padding)
    $y1 = [Math]::Min($bmp.Height - 1, $maxY + $padding)
    return [System.Drawing.Rectangle]::new($x0, $y0, $x1 - $x0 + 1, $y1 - $y0 + 1)
}

function Clear-CanvasEdgeArtifacts([System.Drawing.Bitmap]$bmp, [int]$edge) {
    for ($y = 0; $y -lt $bmp.Height; $y++) {
        for ($x = 0; $x -lt $bmp.Width; $x++) {
            if ($x -lt $edge -or $x -ge ($bmp.Width - $edge) -or $y -lt $edge -or $y -ge ($bmp.Height - $edge)) {
                $c = $bmp.GetPixel($x, $y)
                $bmp.SetPixel($x, $y, [System.Drawing.Color]::FromArgb(0, $c.R, $c.G, $c.B))
            }
        }
    }
}

foreach ($job in $jobs) {
    if (!(Test-Path $job.Sheet)) {
        throw "Missing action pose sheet: $($job.Sheet)"
    }
    $sheet = [System.Drawing.Bitmap]::FromFile($job.Sheet)
    try {
        $cellW = [int]($sheet.Width / 4)
        $cellH = [int]($sheet.Height / 2)
        $outDir = Join-Path $Root "assets\art\characters\$($job.Character)\sprites\body"
        New-Item -ItemType Directory -Force -Path $outDir | Out-Null

        foreach ($entry in $job.Outputs.GetEnumerator()) {
            $index = [int]$entry.Value
            $col = $index % 4
            $row = [int][Math]::Floor($index / 4)
            $cropRect = [System.Drawing.Rectangle]::new($col * $cellW + 56, $row * $cellH + 56, $cellW - 112, $cellH - 112)
            $crop = Copy-Crop $sheet $cropRect
            try {
                Remove-ConnectedWhiteBackground $crop
                $bounds = Find-OpaqueBounds $crop 24
                $trimmed = Copy-Crop $crop $bounds
                try {
                    Clear-CanvasEdgeArtifacts $trimmed 4
                    $path = Join-Path $outDir $entry.Key
                    $trimmed.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
                    Write-Host "Wrote $path"
                } finally {
                    $trimmed.Dispose()
                }
            } finally {
                $crop.Dispose()
            }
        }
    } finally {
        $sheet.Dispose()
    }
}

Write-Host "Extracted Mo Hen and Su Xin body sprite masters."
