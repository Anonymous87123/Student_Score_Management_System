#include "EduSys/model/Score.hpp"

#include <utility>

#include "EduSys/storage/BinaryReader.hpp"
#include "EduSys/storage/BinaryWriter.hpp"

namespace EduSys {

Score::Score(std::string studentId,
             std::string courseId,
             std::string semester,
             double usualScore,
             double finalScore,
             double totalScore)
    : studentId_(std::move(studentId)),
      courseId_(std::move(courseId)),
      semester_(std::move(semester)),
      usualScore_(usualScore),
      finalScore_(finalScore),
      totalScore_(totalScore) {}

void Score::writeTo(BinaryWriter& w) const {
    w.writeString(studentId_);
    w.writeString(courseId_);
    w.writeString(semester_);
    w.writeDouble(usualScore_);
    w.writeDouble(finalScore_);
    w.writeDouble(totalScore_);
}

Score Score::readFrom(BinaryReader& r) {
    Score s;
    s.studentId_  = r.readString();
    s.courseId_   = r.readString();
    s.semester_   = r.readString();
    s.usualScore_ = r.readDouble();
    s.finalScore_ = r.readDouble();
    s.totalScore_ = r.readDouble();
    return s;
}

} // namespace EduSys
