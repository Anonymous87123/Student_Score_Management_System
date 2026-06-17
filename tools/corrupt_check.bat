@echo off
setlocal EnableExtensions EnableDelayedExpansion

rem ========================================================================
rem  Week 13 F Group -- file corruption scenarios (external, not in exe).
rem  ------------------------------------------------------------------------
rem  Why external: per user's constraint for Week 13, --self-test never
rem  modifies .dat files. F group verifies that malformed or missing data
rem  files are rejected by Storage / Service layers, so we corrupt from the
rem  outside, run exe, capture exit code, then restore the snapshot.
rem
rem  Preconditions:
rem    * edusys.exe built at repo root (run build.bat first)
rem    * data/*.dat already seeded (run "edusys.exe --self-test" once)
rem    * PowerShell available (default on Win10/11)
rem
rem  Outputs:
rem    * data/__corrupt_out__/F{1,2,3}.out   -- captured stdout+stderr
rem    * data/__corrupt_out__/sanity.out     -- post-restore self-test run
rem
rem  Exit code:
rem    * 0 -- all three F cases behaved as expected AND restored data still
rem           passes --self-test
rem    * 1 -- any case diverged OR sanity failed (inspect %OUTDIR% for tail)
rem ========================================================================

rem --- anchor cwd to repo root (parent of this script's dir) so that
rem     relative paths work no matter how the script was invoked.
pushd "%~dp0.." >nul

set EXE=edusys.exe
set DATA=data
set BACKUP=%DATA%\__corrupt_backup__
set OUTDIR=%DATA%\__corrupt_out__

if not exist %EXE% (
    echo [ERR] %EXE% not found at repo root. Run build.bat first.
    popd >nul
    exit /b 1
)

rem ------------------------------------------------------------------------
rem Stale-exe guard (Week 14 alpha 收口)
rem    Refuse to run if root edusys.exe might be out-of-date relative to:
rem      (a) any file under src/ or include/  -> source rebuilt but exe stale
rem      (b) build\Release\edusys.exe         -> CMake build is fresher than
rem                                              g++ build at repo root
rem    Either case means tests would run against an old binary, risking
rem    false green. Force user to rebuild before proceeding.
rem    Guard exits with code 2 so downstream automation can tell "stale exe"
rem    apart from "F group failure" (exit /b 1).
rem ------------------------------------------------------------------------
powershell -NoProfile -Command ^
  "$exe=Get-Item '%EXE%' -ErrorAction Stop;" ^
  "$srcLatest=(Get-ChildItem -Recurse -File -Path 'src','include' -ErrorAction SilentlyContinue | Measure-Object LastWriteTime -Maximum).Maximum;" ^
  "$cmakeExe=Get-Item 'build\Release\edusys.exe' -ErrorAction SilentlyContinue;" ^
  "$stale=$false;" ^
  "if ($srcLatest -and $srcLatest -gt $exe.LastWriteTime) { Write-Host '  - src/ or include/ has files newer than edusys.exe'; $stale=$true };" ^
  "if ($cmakeExe -and $cmakeExe.LastWriteTime -gt $exe.LastWriteTime) { Write-Host '  - build\Release\edusys.exe is newer than root edusys.exe (CMake build is fresher)'; $stale=$true };" ^
  "if ($stale) { exit 2 } else { exit 0 }"
if errorlevel 2 (
    echo [GUARD] %EXE% appears stale. Rebuild before running F-group tests:
    echo         build.bat                                                                 ^(g++ tooling^)
    echo         OR  cmake --build build --config Release ^&^& copy /y build\Release\edusys.exe .  ^(MSVC tooling^)
    popd >nul
    exit /b 2
)
echo [guard] mtime check OK: %EXE% is up-to-date.

for %%f in (users students teachers courses scores) do (
    if not exist %DATA%\%%f.dat (
        echo [ERR] %DATA%\%%f.dat missing. Run %EXE% --self-test once to seed.
        exit /b 1
    )
)

rem --- snapshot all .dat files before any tampering ---
if exist %BACKUP% rmdir /s /q %BACKUP%
if exist %OUTDIR% rmdir /s /q %OUTDIR%
mkdir %BACKUP% >nul
mkdir %OUTDIR% >nul

for %%f in (users students teachers courses scores) do (
    copy /y %DATA%\%%f.dat %BACKUP%\%%f.dat >nul
)

echo ==============================================
echo  EduSys Week 13 - F Group (file corruption)
echo  Snapshot        : %BACKUP%
echo  Captured logs   : %OUTDIR%\F{1,2,3}.out
echo ==============================================

set PASS=0
set FAIL=0

rem ------------------------------------------------------------------------
rem F1: scores.dat missing entirely
rem    -- loadAll() returns empty vector (by design, missing file == empty),
rem       but Week 11 FINAL ASSERT expects S003 score to exist, so self-check
rem       should throw and exit 1 with "score for S003 should exist" in Fatal.
rem ------------------------------------------------------------------------
echo.
echo [F1] scores.dat removed  ^(expect rc=1 and 'S003' error in Fatal^)
del /q %DATA%\scores.dat
.\%EXE% --self-test > %OUTDIR%\F1.out 2>&1
set RC_F1=!errorlevel!
copy /y %BACKUP%\scores.dat %DATA%\scores.dat >nul
findstr /c:"score for S003" %OUTDIR%\F1.out >nul
set FOUND_F1=!errorlevel!
set OK_F1=0
if !RC_F1! equ 1 if !FOUND_F1! equ 0 set OK_F1=1
if !OK_F1! equ 1 (
    echo   PASS  rc=!RC_F1!, marker 'score for S003' found
    set /a PASS+=1
) else (
    echo   FAIL  rc=!RC_F1! marker-found=!FOUND_F1!  ^(want rc=1, marker=0^)
    set /a FAIL+=1
)

rem ------------------------------------------------------------------------
rem F2: users.dat magic bytes clobbered (EDSY -> XXXX)
rem    -- BinaryRepository::checkHeader throws StorageException("Bad magic ...").
rem       First call happens in authSvc.authenticate() inside Week 11 check.
rem ------------------------------------------------------------------------
echo.
echo [F2] users.dat magic -^> 'XXXX'  ^(expect rc=1 and 'Bad magic' in Fatal^)
powershell -NoProfile -Command "$p='%DATA%\users.dat'; $b=[IO.File]::ReadAllBytes($p); $b[0]=[byte]0x58; $b[1]=[byte]0x58; $b[2]=[byte]0x58; $b[3]=[byte]0x58; [IO.File]::WriteAllBytes($p,$b)"
.\%EXE% --self-test > %OUTDIR%\F2.out 2>&1
set RC_F2=!errorlevel!
copy /y %BACKUP%\users.dat %DATA%\users.dat >nul
findstr /c:"Bad magic" %OUTDIR%\F2.out >nul
set FOUND_F2=!errorlevel!
set OK_F2=0
if !RC_F2! equ 1 if !FOUND_F2! equ 0 set OK_F2=1
if !OK_F2! equ 1 (
    echo   PASS  rc=!RC_F2!, marker 'Bad magic' found
    set /a PASS+=1
) else (
    echo   FAIL  rc=!RC_F2! marker-found=!FOUND_F2!  ^(want rc=1, marker=0^)
    set /a FAIL+=1
)

rem ------------------------------------------------------------------------
rem F3: scores.dat count field inflated (actual count + 1000)
rem    -- header passes, then reading past the real record tail underflows
rem       BinaryReader, which raises StorageException.
rem    -- Offset 8..11 is count (uint32, little-endian host order on x64).
rem ------------------------------------------------------------------------
echo.
echo [F3] scores.dat count -^> current+1000  ^(expect rc=1 from underflow^)
powershell -NoProfile -Command "$p='%DATA%\scores.dat'; $b=[IO.File]::ReadAllBytes($p); $count=[BitConverter]::ToUInt32($b,8); $new=[BitConverter]::GetBytes([uint32]($count+1000)); [Array]::Copy($new,0,$b,8,4); [IO.File]::WriteAllBytes($p,$b)"
.\%EXE% --self-test > %OUTDIR%\F3.out 2>&1
set RC_F3=!errorlevel!
copy /y %BACKUP%\scores.dat %DATA%\scores.dat >nul
findstr /c:"Fatal:" %OUTDIR%\F3.out >nul
set FOUND_F3=!errorlevel!
set OK_F3=0
if !RC_F3! equ 1 if !FOUND_F3! equ 0 set OK_F3=1
if !OK_F3! equ 1 (
    echo   PASS  rc=!RC_F3!, Fatal marker present
    set /a PASS+=1
) else (
    echo   FAIL  rc=!RC_F3! fatal-found=!FOUND_F3!  ^(want rc=1, marker=0^)
    set /a FAIL+=1
)

rem ------------------------------------------------------------------------
rem Final restore + sanity re-run
rem ------------------------------------------------------------------------
echo.
echo ==============================================
echo  F Group Summary: PASS=!PASS!  FAIL=!FAIL!
echo ==============================================

for %%f in (users students teachers courses scores) do (
    if exist %BACKUP%\%%f.dat copy /y %BACKUP%\%%f.dat %DATA%\%%f.dat >nul
)
rmdir /s /q %BACKUP%

echo [sanity] re-running --self-test on restored data ...
.\%EXE% --self-test > %OUTDIR%\sanity.out 2>&1
set RC_SANITY=!errorlevel!
if !RC_SANITY! neq 0 (
    echo [ERR] sanity --self-test FAILED after restore ^(rc=!RC_SANITY!^).
    echo       Inspect %OUTDIR%\sanity.out for details.
    popd >nul
    exit /b 1
)
echo [sanity] OK ^(self-test PASSED on restored data^)

if !FAIL! gtr 0 (
    echo F group had failures. See %OUTDIR%\ for captured output.
    popd >nul
    exit /b 1
)
echo All F-group cases behaved as expected.
popd >nul
exit /b 0

endlocal
