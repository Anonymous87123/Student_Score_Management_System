cmake_minimum_required(VERSION 3.15)
project(EduSys LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(EDUSYS_BUILD_GUI "Build the Qt Widgets GUI target" ON)

if(MSVC)
    add_compile_options(/W4 /utf-8 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

set(EDUSYS_CORE_SOURCES
    src/app/AppContext.cpp
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
)

set(EDUSYS_CLI_SOURCES
    src/app/main.cpp
    src/view/BaseMenu.cpp
    src/view/AdminMenu.cpp
    src/view/TeacherMenu.cpp
    src/view/StudentMenu.cpp
)

set(EDUSYS_GUI_SOURCES
    src/app/gui_main.cpp
    src/gui/LoginDialog.cpp
    src/gui/AdminWindow.cpp
    src/gui/StudentEditDialog.cpp
    src/gui/CourseEditDialog.cpp
    src/gui/ScoreEditDialog.cpp
    src/gui/ChangePasswordDialog.cpp
    src/gui/TeacherWindow.cpp
    src/gui/StudentWindow.cpp
)

add_library(edusys_core STATIC ${EDUSYS_CORE_SOURCES})
target_include_directories(edusys_core PUBLIC include)

add_executable(edusys ${EDUSYS_CLI_SOURCES})
target_link_libraries(edusys PRIVATE edusys_core)
set_target_properties(edusys PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

if(EDUSYS_BUILD_GUI)
    find_package(Qt6 REQUIRED COMPONENTS Widgets CoreTools)

    add_executable(edusys_gui WIN32 ${EDUSYS_GUI_SOURCES})
    target_link_libraries(edusys_gui PRIVATE edusys_core Qt6::Widgets)
    set_target_properties(edusys_gui PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
    )

    add_custom_command(TARGET edusys_gui POST_BUILD
        COMMAND $<TARGET_FILE:Qt6::windeployqt>
                --dir $<TARGET_FILE_DIR:edusys_gui>
                --compiler-runtime
                --no-translations
                $<TARGET_FILE:edusys_gui>
        COMMENT "Deploying Qt runtime dependencies for edusys_gui"
        VERBATIM
    )
endif()
