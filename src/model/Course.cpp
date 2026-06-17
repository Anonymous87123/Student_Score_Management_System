#include "EduSys/model/Course.hpp"

#include <algorithm>
#include <cctype>
#include <utility>

#include "EduSys/storage/BinaryReader.hpp"
#include "EduSys/storage/BinaryWriter.hpp"

namespace EduSys {

namespace {

std::string trim(std::string value) {
    auto notSpace = [](unsigned char ch) { return !std::isspace(ch); };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), notSpace));
    value.erase(std::find_if(value.rbegin(), value.rend(), notSpace).base(), value.end());
    return value;
}

std::vector<std::string> splitTeacherIds(const std::string& teacherIds) {
    std::vector<std::string> out;
    std::string current;
    for (char ch : teacherIds) {
        if (ch == ',' || ch == ';') {
            const std::string id = trim(current);
            if (!id.empty() && std::find(out.begin(), out.end(), id) == out.end()) {
                out.push_back(id);
            }
            current.clear();
        } else {
            current.push_back(ch);
        }
    }
    const std::string id = trim(current);
    if (!id.empty() && std::find(out.begin(), out.end(), id) == out.end()) {
        out.push_back(id);
    }
    return out;
}

std::string joinTeacherIds(const std::vector<std::string>& teacherIds) {
    std::string out;
    for (const auto& id : teacherIds) {
        if (!out.empty()) {
            out += ",";
        }
        out += id;
    }
    return out;
}

} // namespace

Course::Course(std::string courseId,
               std::string courseName,
               double credit,
               std::string teacherId,
               std::string semester)
    : courseId_(std::move(courseId)),
      courseName_(std::move(courseName)),
      credit_(credit),
      teacherId_(std::move(teacherId)),
      semester_(std::move(semester)) {
    teacherId_ = joinTeacherIds(splitTeacherIds(teacherId_));
}

void Course::setCourseName(std::string name)        { courseName_ = std::move(name); }
void Course::setTeacherId(std::string teacherId)    { teacherId_ = joinTeacherIds(splitTeacherIds(teacherId)); }
void Course::setSemester(std::string semester)      { semester_ = std::move(semester); }

std::vector<std::string> Course::getTeacherIds() const {
    return splitTeacherIds(teacherId_);
}

bool Course::hasTeacher(const std::string& teacherId) const {
    const auto teacherIds = getTeacherIds();
    return std::find(teacherIds.begin(), teacherIds.end(), teacherId) != teacherIds.end();
}

void Course::removeTeacherId(const std::string& teacherId) {
    auto teacherIds = getTeacherIds();
    teacherIds.erase(std::remove(teacherIds.begin(), teacherIds.end(), teacherId), teacherIds.end());
    teacherId_ = joinTeacherIds(teacherIds);
}

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
    c.teacherId_  = joinTeacherIds(splitTeacherIds(r.readString()));
    c.semester_   = r.readString();
    return c;
}

} // namespace EduSys
