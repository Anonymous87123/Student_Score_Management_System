#pragma once

#include <string>

namespace EduSys {

class BinaryReader;
class BinaryWriter;

// 成绩记录：独立关联实体，通过 studentId + courseId + semester 标识一条成绩。
// 不嵌套在 Student 内部，也不跨对象持有指针，便于级联删除和文件持久化。
class Score {
public:
    Score() = default;
    Score(std::string studentId,
          std::string courseId,
          std::string semester,
          double usualScore,
          double finalScore,
          double totalScore);

    const std::string& getStudentId() const noexcept { return studentId_; }
    const std::string& getCourseId() const noexcept { return courseId_; }
    const std::string& getSemester() const noexcept { return semester_; }
    double getUsualScore() const noexcept { return usualScore_; }
    double getFinalScore() const noexcept { return finalScore_; }
    double getTotalScore() const noexcept { return totalScore_; }

    void setUsualScore(double s) noexcept { usualScore_ = s; }
    void setFinalScore(double s) noexcept { finalScore_ = s; }
    void setTotalScore(double s) noexcept { totalScore_ = s; }

    void writeTo(BinaryWriter& w) const;
    static Score readFrom(BinaryReader& r);

private:
    std::string studentId_;
    std::string courseId_;
    std::string semester_;
    double      usualScore_ = 0.0;
    double      finalScore_ = 0.0;
    double      totalScore_ = 0.0;
};

} // namespace EduSys
