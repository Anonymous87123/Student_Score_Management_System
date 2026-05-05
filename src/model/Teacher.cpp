#include "EduSys/model/Teacher.hpp"

#include <utility>

#include "EduSys/storage/BinaryReader.hpp"
#include "EduSys/storage/BinaryWriter.hpp"

namespace EduSys {

Teacher::Teacher(std::string id,
                 std::string name,
                 std::string contact,
                 std::string department,
                 std::string title)
    : Person(std::move(id), std::move(name), std::move(contact)),
      department_(std::move(department)),
      title_(std::move(title)) {}

void Teacher::setDepartment(std::string department) { department_ = std::move(department); }
void Teacher::setTitle(std::string title)           { title_ = std::move(title); }

std::string Teacher::roleLabel() const {
    return "Teacher";
}

void Teacher::writeTo(BinaryWriter& w) const {
    w.writeString(id_);
    w.writeString(name_);
    w.writeString(contact_);
    w.writeString(department_);
    w.writeString(title_);
}

Teacher Teacher::readFrom(BinaryReader& r) {
    Teacher t;
    t.id_         = r.readString();
    t.name_       = r.readString();
    t.contact_    = r.readString();
    t.department_ = r.readString();
    t.title_      = r.readString();
    return t;
}

} // namespace EduSys
