@echo off
setlocal
cd /d "%~dp0"
where mingw32-make >nul 2>nul
if errorlevel 1 (echo mingw32-make was not found in PATH.& exit /b 1)
mingw32-make config=release_x64 game-engine
if errorlevel 1 exit /b %errorlevel%
if not exist "build/standalone" mkdir "build/standalone"
copy /Y "bin\Release\game-engine.exe" "build/standalone\game-engine.exe"
if errorlevel 1 exit /b %errorlevel%
xcopy /E /I /Y "resources" "build/standalone\resources"
if errorlevel 1 exit /b %errorlevel%
echo Standalone build is ready in build/standalone
endlocal
