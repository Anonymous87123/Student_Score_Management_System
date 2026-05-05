#include "EduSys/model/Course.hpp"

#include <utility>

#include "EduSys/storage/BinaryReader.hpp"
#include "EduSys/storage/BinaryWriter.hpp"

namespace EduSys {

Course::Course(std::string courseId,
               std::string courseName,
               double credit,
               std::string teacherId,
               std::string semester)
    : courseId_(std::move(courseId)),
      courseName_(std::move(courseName)),
      credit_(credit),
      teacherId_(std::move(teacherId)),
      semester_(std::move(semester)) {}

void Course::setCourseName(std::string name)        { courseName_ = std::move(name); }
void Course::setTeacherId(std::string teacherId)    { teacherId_ = std::move(teacherId); }
void Course::setSemester(std::string semester)      { semester_ = std::move(semester); }

void Course::writeTo(BinaryWriter& w) const {
    w.writeString(courseId_);
    w.writeString(courseName_);
    w.writeDouble(credit_);
    w.writeString(teacherId_);
    w.writeString(semester_);
}

Course Course::readFrom(BinaryReader& r) {
    Course c;
    c.courseId_   = r.readString();
    c.courseName_ = r.readString();
    c.credit_     = r.readDouble();
    c.teacherId_  = r.readString();
    c.semester_   = r.readString();
    return c;
}

} // namespace EduSys
