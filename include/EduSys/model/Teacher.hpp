#pragma once

#include <string>

#include "EduSys/model/Person.hpp"

namespace EduSys {

class BinaryReader;
class BinaryWriter;

class Teacher : public Person {
public:
    Teacher() = default;
    Teacher(std::string id,
            std::string name,
            std::string contact,
            std::string department,
            std::string title);

    const std::string& getDepartment() const noexcept { return department_; }
    const std::string& getTitle() const noexcept { return title_; }

    void setDepartment(std::string department);
    void setTitle(std::string title);

    std::string roleLabel() const override;

    void writeTo(BinaryWriter& w) const;
    static Teacher readFrom(BinaryReader& r);

private:
    std::string department_;
    std::string title_;
};

} // namespace EduSys
