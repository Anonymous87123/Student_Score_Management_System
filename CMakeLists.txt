cmake_minimum_required(VERSION 3.15)
project(EduSys LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/W4 /utf-8 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(EDUSYS_SOURCES
    src/app/main.cpp
    src/common/Logger.cpp
    src/common/PasswordHasher.cpp
    src/model/Student.cpp
    src/model/Teacher.cpp
    src/model/UserAccount.cpp
    src/model/Course.cpp
    src/model/Score.cpp
    src/report/ReportExporter.cpp
    src/service/AuthService.cpp
    src/service/StudentService.cpp
    src/service/CourseService.cpp
    src/service/ScoreService.cpp
    src/service/StatsService.cpp
    src/storage/BinaryReader.cpp
    src/storage/BinaryWriter.cpp
    src/view/BaseMenu.cpp
    src/view/AdminMenu.cpp
    src/view/TeacherMenu.cpp
    src/view/StudentMenu.cpp
)

add_executable(edusys ${EDUSYS_SOURCES})
target_include_directories(edusys PRIVATE include)
set_target_properties(edusys PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)
