#pragma once

#include "EduSys/common/Constants.hpp"
#include "EduSys/model/Teacher.hpp"
#include "EduSys/storage/BinaryRepository.hpp"

namespace EduSys {

class TeacherRepository : public BinaryRepository<Teacher> {
public:
    TeacherRepository() : BinaryRepository<Teacher>(TEACHERS_FILE) {}
};

} // namespace EduSys
