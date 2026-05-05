#pragma once

#include <string>

namespace EduSys {

// 业务对象编号统一使用字符串（学号、工号、课程号等）。
using Id = std::string;

enum class RoleType {
    Admin,
    Teacher,
    Student
};

} // namespace EduSys
