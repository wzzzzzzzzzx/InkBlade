$ErrorActionPreference = "Stop"

$Root = Resolve-Path "$PSScriptRoot\.."
$VsDevCmd = "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat"
$CMake = "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$Ninja = "C:\Program Files\Microsoft Visual Studio\18\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja\ninja.exe"

if (!(Test-Path $VsDevCmd)) { throw "VsDevCmd.bat not found: $VsDevCmd" }
if (!(Test-Path $CMake)) { throw "CMake not found: $CMake" }
if (!(Test-Path $Ninja)) { throw "Ninja not found: $Ninja" }

$Cmd = "`"$VsDevCmd`" -arch=x64 && `"$CMake`" -S `"$Root`" -B `"$Root\build`" -G Ninja -DCMAKE_MAKE_PROGRAM=`"$Ninja`" -DCMAKE_BUILD_TYPE=Release && `"$CMake`" --build `"$Root\build`" --config Release"
cmd.exe /c $Cmd
