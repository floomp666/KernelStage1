@echo off
setlocal
pushd "%~dp0"
"%WINDIR%\Microsoft.NET\Framework64\v4.0.30319\csc.exe" /nologo /target:winexe /platform:x64 /optimize+ /win32manifest:app.manifest /out:..\..\Launcher.exe /reference:System.Windows.Forms.dll /reference:System.Drawing.dll Core.cs Launcher.cs
set "RESULT=%ERRORLEVEL%"
popd
exit /b %RESULT%
