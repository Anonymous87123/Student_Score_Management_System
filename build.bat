@echo off
setlocal EnableExtensions
set OUT=edusys.exe
set CXX=g++
set FLAGS=-std=c++17 -Wall -Wextra -Wpedantic -O2 -Iinclude

%CXX% %FLAGS% ^
    src\app\main.cpp ^
    src\common\Logger.cpp ^
    src\common\PasswordHasher.cpp ^
    src\model\Student.cpp ^
    src\model\Teacher.cpp ^
    src\model\UserAccount.cpp ^
    src\model\Course.cpp ^
    src\model\Score.cpp ^
    src\report\ReportExporter.cpp ^
    src\service\AuthService.cpp ^
    src\service\StudentService.cpp ^
    src\service\CourseService.cpp ^
    src\service\ScoreService.cpp ^
    src\service\StatsService.cpp ^
    src\storage\BinaryReader.cpp ^
    src\storage\BinaryWriter.cpp ^
    src\view\BaseMenu.cpp ^
    src\view\AdminMenu.cpp ^
    src\view\TeacherMenu.cpp ^
    src\view\StudentMenu.cpp ^
    -o %OUT%

if errorlevel 1 (
    echo Build FAILED.
    exit /b 1
)
echo Build OK -^> %OUT%
endlocal
