#pragma once

#include <string>
#include <vector>

namespace EduSys {

class BinaryReader;
class BinaryWriter;

// 课程实体：通过 teacherId 字符串与 Teacher 关联；多个教师用逗号分隔。
class Course {
public:
    Course() = default;
    Course(std::string courseId,
           std::string courseName,
           double credit,
           std::string teacherId,
           std::string semester);

    const std::string& getCourseId() const noexcept { return courseId_; }
    const std::string& getCourseName() const noexcept { return courseName_; }
    double getCredit() const noexcept { return credit_; }
    const std::string& getTeacherId() const noexcept { return teacherId_; }
    std::vector<std::string> getTeacherIds() const;
    const std::string& getSemester() const noexcept { return semester_; }
    bool hasTeacher(const std::string& teacherId) const;

    void setCourseName(std::string name);
    void setCredit(double credit) noexcept { credit_ = credit; }
    void setTeacherId(std::string teacherId);
    void removeTeacherId(const std::string& teacherId);
    void setSemester(std::string semester);

    void writeTo(BinaryWriter& w) const;
    static Course readFrom(BinaryReader& r);

private:
    std::string courseId_;
    std::string courseName_;
    double      credit_ = 0.0;
    std::string teacherId_;
    std::string semester_;
};

} // namespace EduSys
