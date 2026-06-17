#pragma once

#include "EduSys/report/ReportExporter.hpp"
#include "EduSys/service/AuthService.hpp"
#include "EduSys/service/CourseService.hpp"
#include "EduSys/service/ScoreService.hpp"
#include "EduSys/service/StatsService.hpp"
#include "EduSys/service/StudentService.hpp"
#include "EduSys/service/TeacherService.hpp"
#include "EduSys/storage/CourseRepository.hpp"
#include "EduSys/storage/ScoreRepository.hpp"
#include "EduSys/storage/StudentRepository.hpp"
#include "EduSys/storage/TeacherRepository.hpp"
#include "EduSys/storage/UserRepository.hpp"

namespace EduSys {

class AppContext {
public:
    AppContext();

    bool initializeData();

    StudentRepository studentRepo;
    TeacherRepository teacherRepo;
    UserRepository    userRepo;
    CourseRepository  courseRepo;
    ScoreRepository   scoreRepo;

    AuthService       authService;
    StudentService    studentService;
    TeacherService    teacherService;
    CourseService     courseService;
    ScoreService      scoreService;
    StatsService      statsService;
    ReportExporter    reportExporter;
};

} // namespace EduSys
