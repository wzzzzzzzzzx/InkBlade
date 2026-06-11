param(
    [string]$Exe = (Join-Path $PSScriptRoot '..\build\InkBlade.exe'),
    [string]$ShotDir = (Join-Path $PSScriptRoot '..\build\weapon-combo-visual')
)

$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class NativeInkBladeComboVisual {
    [StructLayout(LayoutKind.Sequential)]
    public struct RECT { public int Left; public int Top; public int Right; public int Bottom; }
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr hWnd, int msg, IntPtr wParam, IntPtr lParam);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetWindowText(IntPtr hWnd, System.Text.StringBuilder text, int count);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);
    [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr hWnd, IntPtr hdcBlt, uint flags);
}
'@

$WM_KEYDOWN = 0x0100
$WM_KEYUP = 0x0101
$VK_ESCAPE = 0x1B
$VK_NORMAL_TEST = 0x35

function Get-Title([IntPtr]$hwnd) {
    $text = New-Object System.Text.StringBuilder 256
    [void][NativeInkBladeComboVisual]::GetWindowText($hwnd, $text, $text.Capacity)
    return $text.ToString()
}

function Wait-ForWindow($process) {
    for ($i = 0; $i -lt 80; $i++) {
        $process.Refresh()
        if ($process.MainWindowHandle -ne [IntPtr]::Zero) {
            if ((Get-Title $process.MainWindowHandle).Length -gt 0) {
                return $process.MainWindowHandle
            }
        }
        Start-Sleep -Milliseconds 100
    }
    throw "Game window did not appear."
}

function Send-Key([IntPtr]$hwnd, [int]$vk) {
    [void][NativeInkBladeComboVisual]::PostMessage($hwnd, $WM_KEYDOWN, [IntPtr]$vk, [IntPtr]0)
    Start-Sleep -Milliseconds 45
    [void][NativeInkBladeComboVisual]::PostMessage($hwnd, $WM_KEYUP, [IntPtr]$vk, [IntPtr]0)
}

function Save-WindowShot([IntPtr]$hwnd, [string]$path) {
    $rect = New-Object NativeInkBladeComboVisual+RECT
    [void][NativeInkBladeComboVisual]::GetWindowRect($hwnd, [ref]$rect)
    $width = [Math]::Max(1, $rect.Right - $rect.Left)
    $height = [Math]::Max(1, $rect.Bottom - $rect.Top)
    $bitmap = New-Object System.Drawing.Bitmap $width, $height
    $graphics = [System.Drawing.Graphics]::FromImage($bitmap)
    $hdc = $graphics.GetHdc()
    $printed = [NativeInkBladeComboVisual]::PrintWindow($hwnd, $hdc, 2)
    $graphics.ReleaseHdc($hdc)
    if (-not $printed) {
        $graphics.CopyFromScreen($rect.Left, $rect.Top, 0, 0, $bitmap.Size)
    }
    $bitmap.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
    $graphics.Dispose()
    $bitmap.Dispose()
}

function Stop-Game($process) {
    if ($process.HasExited) { return }
    [void][NativeInkBladeComboVisual]::PostMessage($process.MainWindowHandle, $WM_KEYDOWN, [IntPtr]$VK_ESCAPE, [IntPtr]0)
    Start-Sleep -Milliseconds 80
    $process.CloseMainWindow() | Out-Null
    Start-Sleep -Milliseconds 80
    if (-not $process.HasExited) { $process.Kill() }
    [void]$process.WaitForExit(2000)
}

function Build-ContactSheet([System.Collections.Generic.List[string]]$paths, [string]$output) {
    $cellWidth = 426
    $cellHeight = 240
    $sheet = New-Object System.Drawing.Bitmap ($cellWidth * 3), ($cellHeight * 9)
    $graphics = [System.Drawing.Graphics]::FromImage($sheet)
    $graphics.Clear([System.Drawing.Color]::FromArgb(24, 28, 30))
    $graphics.InterpolationMode = [System.Drawing.Drawing2D.InterpolationMode]::HighQualityBicubic
    for ($index = 0; $index -lt $paths.Count; $index++) {
        $image = [System.Drawing.Image]::FromFile($paths[$index])
        try {
            $row = [Math]::Floor($index / 3)
            $column = $index % 3
            $target = New-Object System.Drawing.Rectangle ($column * $cellWidth), ($row * $cellHeight), $cellWidth, $cellHeight
            $graphics.DrawImage($image, $target)
        } finally {
            $image.Dispose()
        }
    }
    $sheet.Save($output, [System.Drawing.Imaging.ImageFormat]::Png)
    $graphics.Dispose()
    $sheet.Dispose()
}

if (-not (Test-Path -LiteralPath $Exe)) { throw "Missing exe: $Exe" }
New-Item -ItemType Directory -Force -Path $ShotDir | Out-Null
$shots = [System.Collections.Generic.List[string]]::new()

for ($character = 0; $character -lt 3; $character++) {
    for ($weapon = 0; $weapon -lt 3; $weapon++) {
        for ($stage = 0; $stage -lt 3; $stage++) {
            $args = @(
                '--visual-lab',
                "--visual-lab-character=$character",
                "--visual-lab-weapon=$weapon",
                "--visual-lab-normal-stage=$stage"
            )
            $process = Start-Process -FilePath $Exe -ArgumentList $args -WorkingDirectory (Split-Path $Exe) -PassThru
            try {
                $hwnd = Wait-ForWindow $process
                [void][NativeInkBladeComboVisual]::SetForegroundWindow($hwnd)
                Start-Sleep -Milliseconds 260
                Send-Key $hwnd $VK_NORMAL_TEST
                Start-Sleep -Milliseconds 90
                $shot = Join-Path $ShotDir ("character{0}_weapon{1}_stage{2}.png" -f $character, $weapon, ($stage + 1))
                Save-WindowShot $hwnd $shot
                if ((Get-Item -LiteralPath $shot).Length -lt 10000) {
                    throw "Screenshot is unexpectedly small: $shot"
                }
                $shots.Add($shot)
                Write-Host ("Captured character={0} weapon={1} stage={2}" -f $character, $weapon, ($stage + 1))
            } finally {
                Stop-Game $process
            }
        }
    }
}

if ($shots.Count -ne 27) { throw "Expected 27 screenshots, got $($shots.Count)." }
$contactSheet = Join-Path $ShotDir 'weapon_combo_contact_sheet.png'
Build-ContactSheet $shots $contactSheet
Write-Host "OK captured 27 weapon combo screenshots: $contactSheet"
