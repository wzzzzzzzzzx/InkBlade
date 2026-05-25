param(
    [string]$Exe = (Join-Path $PSScriptRoot '..\dist\InkBlade\InkBlade.exe'),
    [string]$ShotDir = (Join-Path $PSScriptRoot '..\build\visual-smoke')
)

$ErrorActionPreference = 'Stop'

Add-Type -AssemblyName System.Drawing
Add-Type @'
using System;
using System.Runtime.InteropServices;
public static class NativeInkBladeSmoke {
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
$WM_LBUTTONDOWN = 0x0201
$WM_LBUTTONUP = 0x0202
$VK_RETURN = 0x0D
$VK_DOWN = 0x28
$VK_ESCAPE = 0x1B

function Make-LParam([int]$x, [int]$y) {
    return [IntPtr](($y -shl 16) -bor ($x -band 0xFFFF))
}

function Get-Title([IntPtr]$hwnd) {
    $sb = New-Object System.Text.StringBuilder 256
    [void][NativeInkBladeSmoke]::GetWindowText($hwnd, $sb, $sb.Capacity)
    return $sb.ToString()
}

function Title-EndsWithCodepoints([IntPtr]$hwnd, [int[]]$codes) {
    $title = Get-Title $hwnd
    if ($title.Length -lt $codes.Count) { return $false }
    for ($i = 0; $i -lt $codes.Count; $i++) {
        if ([int][char]$title[$title.Length - $codes.Count + $i] -ne $codes[$i]) { return $false }
    }
    return $true
}

function Send-MenuKey([IntPtr]$hwnd, [int]$vk) {
    [void][NativeInkBladeSmoke]::PostMessage($hwnd, $WM_KEYDOWN, [IntPtr]$vk, [IntPtr]0)
    Start-Sleep -Milliseconds 45
    [void][NativeInkBladeSmoke]::PostMessage($hwnd, $WM_KEYUP, [IntPtr]$vk, [IntPtr]0)
    Start-Sleep -Milliseconds 90
}

function Send-RealKey([byte]$vk, [int]$holdMs = 70) {
    [NativeInkBladeSmoke]::keybd_event($vk, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds $holdMs
    [NativeInkBladeSmoke]::keybd_event($vk, 0, 2, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 100
}

function Save-MovingShot([IntPtr]$hwnd, [string]$path) {
    [void][NativeInkBladeSmoke]::SetForegroundWindow($hwnd)
    Start-Sleep -Milliseconds 120
    [NativeInkBladeSmoke]::keybd_event(0x44, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 260
    Save-WindowShot $hwnd $path
    [NativeInkBladeSmoke]::keybd_event(0x44, 0, 2, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 120
}

function Save-JumpShot([IntPtr]$hwnd, [string]$path) {
    [void][NativeInkBladeSmoke]::SetForegroundWindow($hwnd)
    Start-Sleep -Milliseconds 120
    [NativeInkBladeSmoke]::keybd_event(0x20, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 140
    Save-WindowShot $hwnd $path
    [NativeInkBladeSmoke]::keybd_event(0x20, 0, 2, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 360
}

function Save-DodgeShot([IntPtr]$hwnd, [string]$path) {
    [void][NativeInkBladeSmoke]::SetForegroundWindow($hwnd)
    Start-Sleep -Milliseconds 120
    [NativeInkBladeSmoke]::keybd_event(0x10, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 70
    Save-WindowShot $hwnd $path
    [NativeInkBladeSmoke]::keybd_event(0x10, 0, 2, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 180
}

function Save-ParryShot([IntPtr]$hwnd, [string]$path) {
    [void][NativeInkBladeSmoke]::SetForegroundWindow($hwnd)
    Start-Sleep -Milliseconds 120
    [NativeInkBladeSmoke]::keybd_event(0x47, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 70
    Save-WindowShot $hwnd $path
    [NativeInkBladeSmoke]::keybd_event(0x47, 0, 2, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 180
}

function Save-NormalShot([IntPtr]$hwnd, [string]$path) {
    [void][NativeInkBladeSmoke]::SetForegroundWindow($hwnd)
    Start-Sleep -Milliseconds 120
    Send-Mouse $hwnd $true 520 430
    Start-Sleep -Milliseconds 70
    Save-WindowShot $hwnd $path
    Send-Mouse $hwnd $false 520 430
    Start-Sleep -Milliseconds 220
}

function Save-ChargeShots([IntPtr]$hwnd, [string]$chargingPath, [string]$releasePath) {
    [void][NativeInkBladeSmoke]::SetForegroundWindow($hwnd)
    Start-Sleep -Milliseconds 120
    Send-Mouse $hwnd $true 520 430
    Start-Sleep -Milliseconds 420
    Save-WindowShot $hwnd $chargingPath
    Start-Sleep -Milliseconds 360
    Send-Mouse $hwnd $false 520 430
    Start-Sleep -Milliseconds 70
    Save-WindowShot $hwnd $releasePath
    Start-Sleep -Milliseconds 240
}

function Save-SkillShot([IntPtr]$hwnd, [string]$path) {
    [void][NativeInkBladeSmoke]::SetForegroundWindow($hwnd)
    Start-Sleep -Milliseconds 120
    [NativeInkBladeSmoke]::keybd_event(0x46, 0, 0, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 70
    Save-WindowShot $hwnd $path
    [NativeInkBladeSmoke]::keybd_event(0x46, 0, 2, [UIntPtr]::Zero)
    Start-Sleep -Milliseconds 260
}

function Send-Mouse([IntPtr]$hwnd, [bool]$down, [int]$x, [int]$y) {
    $msg = if ($down) { $WM_LBUTTONDOWN } else { $WM_LBUTTONUP }
    $w = if ($down) { [IntPtr]1 } else { [IntPtr]0 }
    [void][NativeInkBladeSmoke]::PostMessage($hwnd, $msg, $w, (Make-LParam $x $y))
}

function Save-WindowShot([IntPtr]$hwnd, [string]$path) {
    $rect = New-Object NativeInkBladeSmoke+RECT
    [void][NativeInkBladeSmoke]::GetWindowRect($hwnd, [ref]$rect)
    $width = [Math]::Max(1, $rect.Right - $rect.Left)
    $height = [Math]::Max(1, $rect.Bottom - $rect.Top)
    $bmp = New-Object System.Drawing.Bitmap $width, $height
    $gfx = [System.Drawing.Graphics]::FromImage($bmp)
    $hdc = $gfx.GetHdc()
    $printed = [NativeInkBladeSmoke]::PrintWindow($hwnd, $hdc, 2)
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

function Enter-Battle([IntPtr]$hwnd, [int]$charIndex, [int]$weaponIndex) {
    Send-MenuKey $hwnd $VK_RETURN
    for ($i = 0; $i -lt $charIndex; $i++) { Send-MenuKey $hwnd $VK_DOWN }
    Send-MenuKey $hwnd $VK_RETURN
    for ($i = 0; $i -lt $weaponIndex; $i++) { Send-MenuKey $hwnd $VK_DOWN }
    Send-MenuKey $hwnd $VK_RETURN
    Send-MenuKey $hwnd $VK_RETURN
    for ($i = 0; $i -lt 50; $i++) {
        if (Title-EndsWithCodepoints $hwnd @(25112, 26007)) { return }
        Start-Sleep -Milliseconds 100
    }
    throw "Did not reach battle screen. Last title: $(Get-Title $hwnd)"
}

function Enter-PracticeBattle([IntPtr]$hwnd) {
    Send-Mouse $hwnd $true 640 540
    Start-Sleep -Milliseconds 60
    Send-Mouse $hwnd $false 640 540
    Start-Sleep -Milliseconds 200
    Send-Mouse $hwnd $true 640 470
    Start-Sleep -Milliseconds 60
    Send-Mouse $hwnd $false 640 470
    for ($i = 0; $i -lt 50; $i++) {
        if (Title-EndsWithCodepoints $hwnd @(25112, 26007)) { return }
        Start-Sleep -Milliseconds 100
    }
    throw "Did not reach practice battle screen. Last title: $(Get-Title $hwnd)"
}

function Exercise-Combat([IntPtr]$hwnd) {
    [void][NativeInkBladeSmoke]::SetForegroundWindow($hwnd)
    Start-Sleep -Milliseconds 160
    Send-Mouse $hwnd $true 520 430
    Start-Sleep -Milliseconds 120
    Send-Mouse $hwnd $false 520 430
    Start-Sleep -Milliseconds 220
    Send-Mouse $hwnd $true 520 430
    Start-Sleep -Milliseconds 720
    Send-Mouse $hwnd $false 520 430
    Start-Sleep -Milliseconds 220
    Send-RealKey 0x47
    Send-RealKey 0x10
    Send-RealKey 0x20
    Send-RealKey 0x46
}

if (-not (Test-Path $Exe)) { throw "Missing exe: $Exe" }
New-Item -ItemType Directory -Force -Path $ShotDir | Out-Null

$cases = @()
for ($c = 0; $c -lt 3; $c++) {
    for ($w = 0; $w -lt 3; $w++) {
        $cases += [pscustomobject]@{ Character = $c; Weapon = $w }
    }
}

foreach ($case in $cases) {
    $p = Start-Process -FilePath $Exe -WorkingDirectory (Split-Path $Exe) -PassThru
    try {
        $hwnd = Wait-ForWindow $p
        [void][NativeInkBladeSmoke]::SetForegroundWindow($hwnd)
        Start-Sleep -Milliseconds 250
        if ($case.Character -eq 0 -and $case.Weapon -eq 0) {
            Save-WindowShot $hwnd (Join-Path $ShotDir "main_menu.png")
        }
        Enter-Battle $hwnd $case.Character $case.Weapon
        if ($case.Character -eq 0 -and $case.Weapon -eq 0) {
            Start-Sleep -Milliseconds 250
            $idleShot = Join-Path $ShotDir "char0_weapon0_idle.png"
            Save-WindowShot $hwnd $idleShot
            $runShot = Join-Path $ShotDir "char0_weapon0_run.png"
            Save-MovingShot $hwnd $runShot
            $jumpShot = Join-Path $ShotDir "char0_weapon0_jump.png"
            Save-JumpShot $hwnd $jumpShot
            $dodgeShot = Join-Path $ShotDir "char0_weapon0_dodge.png"
            Save-DodgeShot $hwnd $dodgeShot
            $parryShot = Join-Path $ShotDir "char0_weapon0_parry.png"
            Save-ParryShot $hwnd $parryShot
            $normalShot = Join-Path $ShotDir "char0_weapon0_normal.png"
            Save-NormalShot $hwnd $normalShot
            $chargingShot = Join-Path $ShotDir "char0_weapon0_charging.png"
            $chargeReleaseShot = Join-Path $ShotDir "char0_weapon0_charge_release.png"
            Save-ChargeShots $hwnd $chargingShot $chargeReleaseShot
            $skillShot = Join-Path $ShotDir "char0_weapon0_skill.png"
            Save-SkillShot $hwnd $skillShot
        }
        Exercise-Combat $hwnd
        if (($case.Character -eq $case.Weapon) -or ($case.Character -eq 2 -and $case.Weapon -eq 0)) {
            $shot = Join-Path $ShotDir ("char{0}_weapon{1}.png" -f $case.Character, $case.Weapon)
            Save-WindowShot $hwnd $shot
        }
        Write-Host ("Case character={0} weapon={1} reached battle and accepted combat inputs." -f $case.Character, $case.Weapon)
    } finally {
        if (-not $p.HasExited) {
            [void][NativeInkBladeSmoke]::PostMessage($p.MainWindowHandle, $WM_KEYDOWN, [IntPtr]$VK_ESCAPE, [IntPtr]0)
            Start-Sleep -Milliseconds 120
            $p.CloseMainWindow() | Out-Null
            Start-Sleep -Milliseconds 120
            if (-not $p.HasExited) { $p.Kill() }
            [void]$p.WaitForExit(2000)
        }
    }
}

$p = Start-Process -FilePath $Exe -WorkingDirectory (Split-Path $Exe) -PassThru
try {
    $hwnd = Wait-ForWindow $p
    [void][NativeInkBladeSmoke]::SetForegroundWindow($hwnd)
    Start-Sleep -Milliseconds 250
    Enter-PracticeBattle $hwnd
    Send-RealKey 0x56
    $practiceShot = Join-Path $ShotDir "practice_default_ultimate.png"
    Save-WindowShot $hwnd $practiceShot
    Write-Host "Practice mode reached battle, default full-energy ultimate input accepted."
} finally {
    if (-not $p.HasExited) {
        [void][NativeInkBladeSmoke]::PostMessage($p.MainWindowHandle, $WM_KEYDOWN, [IntPtr]$VK_ESCAPE, [IntPtr]0)
        Start-Sleep -Milliseconds 120
        $p.CloseMainWindow() | Out-Null
        Start-Sleep -Milliseconds 120
        if (-not $p.HasExited) { $p.Kill() }
        [void]$p.WaitForExit(2000)
    }
}

Write-Host "Smoke interaction test passed for 9 character/weapon combinations."
