#pragma once

#include "EduSys/common/Constants.hpp"
#include "EduSys/model/Course.hpp"
#include "EduSys/storage/BinaryRepository.hpp"

namespace EduSys {

class CourseRepository : public BinaryRepository<Course> {
public:
    CourseRepository() : BinaryRepository<Course>(COURSES_FILE) {}
};

} // namespace EduSys
