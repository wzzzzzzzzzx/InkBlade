param(
    [string]$Exe = (Join-Path $PSScriptRoot '..\dist\InkBlade\InkBlade.exe'),
    [string]$ShotDir = (Join-Path $PSScriptRoot '..\build\visual-lab')
)

$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class NativeInkBladeVisualLab {
    [StructLayout(LayoutKind.Sequential)]
    public struct RECT { public int Left; public int Top; public int Right; public int Bottom; }
    [DllImport("user32.dll")] public static extern bool SetForegroundWindow(IntPtr hWnd);
    [DllImport("user32.dll")] public static extern bool PostMessage(IntPtr hWnd, int msg, IntPtr wParam, IntPtr lParam);
    [DllImport("user32.dll")] public static extern void keybd_event(byte vk, byte scan, int flags, UIntPtr extra);
    [DllImport("user32.dll", CharSet=CharSet.Unicode)] public static extern int GetWindowText(IntPtr hWnd, System.Text.StringBuilder text, int count);
    [DllImport("user32.dll")] public static extern bool GetWindowRect(IntPtr hWnd, out RECT rect);
    [DllImport("user32.dll")] public static extern bool PrintWindow(IntPtr hWnd, IntPtr hdcBlt, uint flags);
}
'@

$WM_KEYDOWN = 0x0100
$WM_KEYUP = 0x0101
$VK_ESCAPE = 0x1B
$VK_D = 0x44

function Get-Title([IntPtr]$hwnd) {
    $sb = New-Object System.Text.StringBuilder 256
    [void][NativeInkBladeVisualLab]::GetWindowText($hwnd, $sb, $sb.Capacity)
    return $sb.ToString()
}

function Send-Key([IntPtr]$hwnd, [int]$vk, [int]$holdMs = 45) {
    [void][NativeInkBladeVisualLab]::PostMessage($hwnd, $WM_KEYDOWN, [IntPtr]$vk, [IntPtr]0)
    Start-Sleep -Milliseconds $holdMs
    [void][NativeInkBladeVisualLab]::PostMessage($hwnd, $WM_KEYUP, [IntPtr]$vk, [IntPtr]0)
}

function Save-WindowShot([IntPtr]$hwnd, [string]$path) {
    $rect = New-Object NativeInkBladeVisualLab+RECT
    [void][NativeInkBladeVisualLab]::GetWindowRect($hwnd, [ref]$rect)
    $width = [Math]::Max(1, $rect.Right - $rect.Left)
    $height = [Math]::Max(1, $rect.Bottom - $rect.Top)
    $bmp = New-Object System.Drawing.Bitmap $width, $height
    $gfx = [System.Drawing.Graphics]::FromImage($bmp)
    $hdc = $gfx.GetHdc()
    $printed = [NativeInkBladeVisualLab]::PrintWindow($hwnd, $hdc, 2)
    $gfx.ReleaseHdc($hdc)
    if (-not $printed) {
        $gfx.CopyFromScreen($rect.Left, $rect.Top, 0, 0, $bmp.Size)
    }
    $bmp.Save($path, [System.Drawing.Imaging.ImageFormat]::Png)
    $gfx.Dispose()
    $bmp.Dispose()
}

function Wait-ForWindow($process) {
    for ($i = 0; $i -lt 80; $i++) {
        $process.Refresh()
        if ($process.MainWindowHandle -ne [IntPtr]::Zero) {
            $title = Get-Title $process.MainWindowHandle
            if ($title.Length -gt 0) { return $process.MainWindowHandle }
        }
        Start-Sleep -Milliseconds 100
    }
    throw "Game window did not appear."
}

function Capture-Action([IntPtr]$hwnd, [int]$vk, [string]$fileName, [int]$delayMs) {
    Send-Key $hwnd $vk
    Start-Sleep -Milliseconds $delayMs
    Save-WindowShot $hwnd (Join-Path $ShotDir $fileName)
    Start-Sleep -Milliseconds 260
}

function Capture-HeldRealKey([IntPtr]$hwnd, [byte]$vk, [string]$fileName, [int]$holdMs = 260) {
    [void][NativeInkBladeVisualLab]::SetForegroundWindow($hwnd)
    Start-Sleep -Milliseconds 120
    [NativeInkBladeVisualLab]::keybd_event($vk, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds $holdMs
    Save-WindowShot $hwnd (Join-Path $ShotDir $fileName)
    [NativeInkBladeVisualLab]::keybd_event($vk, 0, 2, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 240
}

function Copy-LegacyChar0Shot([string]$sourceName, [string]$legacyName) {
    $source = Join-Path $ShotDir $sourceName
    $legacy = Join-Path $ShotDir $legacyName
    if (Test-Path $source) {
        Copy-Item -LiteralPath $source -Destination $legacy -Force
    }
}

function Run-VisualLabCase([int]$characterIndex) {
    $args = @('--visual-lab', ('--visual-lab-character={0}' -f $characterIndex))
    $p = Start-Process -FilePath $Exe -ArgumentList $args -WorkingDirectory (Split-Path $Exe) -PassThru
    try {
        $hwnd = Wait-ForWindow $p
        [void][NativeInkBladeVisualLab]::SetForegroundWindow($hwnd)
        Start-Sleep -Milliseconds 500

        Save-WindowShot $hwnd (Join-Path $ShotDir ("visual_lab_char{0}_idle.png" -f $characterIndex))
        Capture-HeldRealKey $hwnd $VK_D ("visual_lab_char{0}_run.png" -f $characterIndex)
        Capture-Action $hwnd 0x31 ("visual_lab_char{0}_ultimate.png" -f $characterIndex) 110
        Capture-Action $hwnd 0x32 ("visual_lab_char{0}_charge_release.png" -f $characterIndex) 80
        Capture-Action $hwnd 0x33 ("visual_lab_char{0}_hit.png" -f $characterIndex) 90
        Capture-Action $hwnd 0x34 ("visual_lab_char{0}_skill.png" -f $characterIndex) 90
        Capture-Action $hwnd 0x35 ("visual_lab_char{0}_normal.png" -f $characterIndex) 90
        Capture-Action $hwnd 0x36 ("visual_lab_char{0}_charging.png" -f $characterIndex) 120
        Capture-Action $hwnd 0x37 ("visual_lab_char{0}_jump.png" -f $characterIndex) 90
        Capture-Action $hwnd 0x38 ("visual_lab_char{0}_parry.png" -f $characterIndex) 90

        if ($characterIndex -eq 0) {
            Copy-LegacyChar0Shot 'visual_lab_char0_idle.png' 'visual_lab_idle.png'
            Copy-LegacyChar0Shot 'visual_lab_char0_run.png' 'visual_lab_run.png'
            Copy-LegacyChar0Shot 'visual_lab_char0_ultimate.png' 'visual_lab_ultimate.png'
            Copy-LegacyChar0Shot 'visual_lab_char0_charge_release.png' 'visual_lab_charge_release.png'
            Copy-LegacyChar0Shot 'visual_lab_char0_hit.png' 'visual_lab_hit.png'
            Copy-LegacyChar0Shot 'visual_lab_char0_skill.png' 'visual_lab_skill.png'
            Copy-LegacyChar0Shot 'visual_lab_char0_normal.png' 'visual_lab_normal.png'
            Copy-LegacyChar0Shot 'visual_lab_char0_charging.png' 'visual_lab_charging.png'
            Copy-LegacyChar0Shot 'visual_lab_char0_jump.png' 'visual_lab_jump.png'
            Copy-LegacyChar0Shot 'visual_lab_char0_parry.png' 'visual_lab_parry.png'
        }
    } finally {
        if (-not $p.HasExited) {
            [void][NativeInkBladeVisualLab]::PostMessage($p.MainWindowHandle, $WM_KEYDOWN, [IntPtr]$VK_ESCAPE, [IntPtr]0)
            Start-Sleep -Milliseconds 120
            $p.CloseMainWindow() | Out-Null
            Start-Sleep -Milliseconds 120
            if (-not $p.HasExited) { $p.Kill() }
            [void]$p.WaitForExit(2000)
        }
    }
}

if (-not (Test-Path $Exe)) { throw "Missing exe: $Exe" }
New-Item -ItemType Directory -Force -Path $ShotDir | Out-Null

for ($characterIndex = 0; $characterIndex -lt 3; $characterIndex++) {
    Run-VisualLabCase $characterIndex
}

$expected = @(
    'visual_lab_idle.png',
    'visual_lab_run.png',
    'visual_lab_ultimate.png',
    'visual_lab_charge_release.png',
    'visual_lab_hit.png',
    'visual_lab_skill.png',
    'visual_lab_normal.png',
    'visual_lab_charging.png',
    'visual_lab_jump.png',
    'visual_lab_parry.png'
)

for ($characterIndex = 0; $characterIndex -lt 3; $characterIndex++) {
    $expected += "visual_lab_char${characterIndex}_idle.png"
    $expected += "visual_lab_char${characterIndex}_run.png"
    $expected += "visual_lab_char${characterIndex}_ultimate.png"
    $expected += "visual_lab_char${characterIndex}_charge_release.png"
    $expected += "visual_lab_char${characterIndex}_hit.png"
    $expected += "visual_lab_char${characterIndex}_skill.png"
    $expected += "visual_lab_char${characterIndex}_normal.png"
    $expected += "visual_lab_char${characterIndex}_charging.png"
    $expected += "visual_lab_char${characterIndex}_jump.png"
    $expected += "visual_lab_char${characterIndex}_parry.png"
}

foreach ($name in $expected) {
    $path = Join-Path $ShotDir $name
    if (-not (Test-Path $path)) { throw "Missing screenshot: $path" }
    $length = (Get-Item $path).Length
    if ($length -lt 10000) { throw "Screenshot too small: $path ($length bytes)" }
}

Write-Host "Visual combat lab screenshots captured for 3 characters."
