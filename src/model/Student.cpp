#include "EduSys/model/Student.hpp"

#include <utility>

#include "EduSys/storage/BinaryReader.hpp"
#include "EduSys/storage/BinaryWriter.hpp"

namespace EduSys {

Student::Student(std::string id,
                 std::string name,
                 std::string contact,
                 std::string major,
                 std::string className,
                 int enrollYear)
    : Person(std::move(id), std::move(name), std::move(contact)),
      major_(std::move(major)),
      className_(std::move(className)),
      enrollYear_(enrollYear) {}

void Student::setMajor(std::string major)         { major_ = std::move(major); }
void Student::setClassName(std::string className) { className_ = std::move(className); }
void Student::setEnrollYear(int year)             { enrollYear_ = year; }

std::string Student::roleLabel() const {
    return "Student";
}

void Student::writeTo(BinaryWriter& w) const {
    w.writeString(id_);
    w.writeString(name_);
    w.writeString(contact_);
    w.writeString(major_);
    w.writeString(className_);
    w.writeInt32(static_cast<std::int32_t>(enrollYear_));
}

Student Student::readFrom(BinaryReader& r) {
    Student s;
    s.id_         = r.readString();
    s.name_       = r.readString();
    s.contact_    = r.readString();
    s.major_      = r.readString();
    s.className_  = r.readString();
    s.enrollYear_ = static_cast<int>(r.readInt32());
    return s;
}

} // namespace EduSys
