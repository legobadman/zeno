@echo off
setlocal EnableExtensions

REM Usage:
REM   run_gen_testdata.bat [GEN_SCRIPT] [HOUDINI_VERSION_HINT]
REM Examples:
REM   run_gen_testdata.bat
REM   run_gen_testdata.bat Extrude_gen_testdata.py
REM   run_gen_testdata.bat Extrude_gen_testdata.py 20.0

set "SCRIPT_DIR=%~dp0"
set "GEN_SCRIPT=%~1"
set "VERSION_HINT=%~2"

if "%GEN_SCRIPT%"=="" set "GEN_SCRIPT=Extrude_gen_testdata.py"

if exist "%GEN_SCRIPT%" (
    set "GEN_SCRIPT_PATH=%GEN_SCRIPT%"
) else (
    set "GEN_SCRIPT_PATH=%SCRIPT_DIR%%GEN_SCRIPT%"
)

if not exist "%GEN_SCRIPT_PATH%" (
    echo [ERROR] Testdata script not found:
    echo         "%GEN_SCRIPT_PATH%"
    exit /b 1
)

for /f "usebackq delims=" %%I in (`
    powershell -NoProfile -ExecutionPolicy Bypass -Command ^
    "$hint = '%VERSION_HINT%';" ^
    "$cands = New-Object System.Collections.Generic.List[string];" ^
    "if ($env:HOUDINI_HYTHON -and (Test-Path $env:HOUDINI_HYTHON)) { $cands.Add((Resolve-Path $env:HOUDINI_HYTHON).Path) };" ^
    "if ($env:HFS) { $p = Join-Path $env:HFS 'bin\hython.exe'; if (Test-Path $p) { $cands.Add((Resolve-Path $p).Path) } };" ^
    "$cmd = Get-Command hython.exe -ErrorAction SilentlyContinue; if ($cmd -and (Test-Path $cmd.Source)) { $cands.Add((Resolve-Path $cmd.Source).Path) };" ^
    "$roots = @(" ^
    "  'Registry::HKEY_LOCAL_MACHINE\SOFTWARE\Side Effects Software'," ^
    "  'Registry::HKEY_LOCAL_MACHINE\SOFTWARE\WOW6432Node\Side Effects Software'" ^
    ");" ^
    "foreach ($root in $roots) {" ^
    "  if (Test-Path $root) {" ^
    "    Get-ChildItem $root -ErrorAction SilentlyContinue | ForEach-Object {" ^
    "      $props = Get-ItemProperty $_.PSPath -ErrorAction SilentlyContinue;" ^
    "      $install = $props.InstallPath;" ^
    "      if (-not $install) { $install = $props.installPath };" ^
    "      if (-not $install) { $install = $props.Path };" ^
    "      if ($install) {" ^
    "        $hython = Join-Path $install 'bin\hython.exe';" ^
    "        if (Test-Path $hython) { $cands.Add((Resolve-Path $hython).Path) }" ^
    "      }" ^
    "    }" ^
    "  }" ^
    "};" ^
    "Get-ChildItem 'C:\Program Files\Side Effects Software' -Directory -ErrorAction SilentlyContinue | ForEach-Object {" ^
    "  $hython = Join-Path $_.FullName 'bin\hython.exe';" ^
    "  if (Test-Path $hython) { $cands.Add((Resolve-Path $hython).Path) }" ^
    "};" ^
    "$unique = $cands | Where-Object { $_ } | Sort-Object -Unique;" ^
    "if (-not $unique -or $unique.Count -eq 0) { exit 2 };" ^
    "$scored = $unique | ForEach-Object {" ^
    "  $path = $_;" ^
    "  $ver = [version]'0.0.0.0';" ^
    "  if ($path -match 'Houdini\\s+([0-9]+\\.[0-9]+\\.[0-9]+)') {" ^
    "    try { $ver = [version]$Matches[1] } catch {}" ^
    "  };" ^
    "  [pscustomobject]@{ Path = $path; Ver = $ver }" ^
    "};" ^
    "$pool = $scored;" ^
    "if ($hint) { $filtered = $scored | Where-Object { $_.Path -like ('*' + $hint + '*') }; if ($filtered) { $pool = $filtered } };" ^
    "$chosen = $pool | Sort-Object Ver -Descending | Select-Object -First 1;" ^
    "if (-not $chosen) { exit 3 };" ^
    "Write-Output $chosen.Path"
`) do (
    set "HYTHON_PATH=%%I"
)

if not defined HYTHON_PATH (
    echo [ERROR] Failed to locate hython.exe automatically.
    echo         You can set HOUDINI_HYTHON environment variable manually.
    exit /b 1
)

echo [INFO] Using hython:
echo        "%HYTHON_PATH%"
echo [INFO] Running:
echo        "%GEN_SCRIPT_PATH%"

"%HYTHON_PATH%" "%GEN_SCRIPT_PATH%"
set "EXIT_CODE=%ERRORLEVEL%"

if not "%EXIT_CODE%"=="0" (
    echo [ERROR] Generation failed with exit code %EXIT_CODE%.
    exit /b %EXIT_CODE%
)

echo [OK] Testdata generation finished.
exit /b 0

