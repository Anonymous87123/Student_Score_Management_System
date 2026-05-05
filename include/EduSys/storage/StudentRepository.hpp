#pragma once

#include "EduSys/common/Constants.hpp"
#include "EduSys/model/Student.hpp"
#include "EduSys/storage/BinaryRepository.hpp"

namespace EduSys {

class StudentRepository : public BinaryRepository<Student> {
public:
    StudentRepository() : BinaryRepository<Student>(STUDENTS_FILE) {}
};

} // namespace EduSys
