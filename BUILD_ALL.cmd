@echo off
setlocal
pushd "%~dp0"
set "MSBUILD=C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe"
if not exist "%MSBUILD%" (echo Edit MSBUILD path for the installed Visual Studio.& exit /b 2)
if not exist binaries mkdir binaries
if not exist build-logs mkdir build-logs
set "_CL_=/Od /GL- /MP2 /MT"
"%MSBUILD%" project\internal\internal.vcxproj /t:Rebuild /p:Configuration=Release /p:Platform=x64 /p:WholeProgramOptimization=false "/p:OutDir=%CD%\binaries\" "/p:IntDir=%CD%\build-obj\dll\" /p:TargetName=RustInternal /m:2 /nologo /verbosity:minimal > build-logs\dll.log 2>&1
if errorlevel 1 goto fail
set "_CL_="
"%MSBUILD%" project\kernel\bridge.vcxproj /t:Rebuild /p:Configuration=Release /p:Platform=x64 "/p:OutDir=%CD%\binaries\" "/p:IntDir=%CD%\build-obj\driver\" /m:2 /nologo /verbosity:minimal > build-logs\driver.log 2>&1
if errorlevel 1 goto fail
call project\launcher\build.cmd > build-logs\launcher.log 2>&1
if errorlevel 1 goto fail
copy /y Launcher.exe binaries\Launcher.exe >nul
echo Built binaries\Launcher.exe, RustInternal.dll, RustInternalBridge.sys
popd
exit /b 0
:fail
echo Build failed. See build-logs.
popd
exit /b 1
