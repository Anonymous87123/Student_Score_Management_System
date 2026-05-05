#pragma once

#include <string>

#include "EduSys/model/Person.hpp"

namespace EduSys {

class BinaryReader;
class BinaryWriter;

class Student : public Person {
public:
    Student() = default;
    Student(std::string id,
            std::string name,
            std::string contact,
            std::string major,
            std::string className,
            int enrollYear);

    const std::string& getMajor() const noexcept { return major_; }
    const std::string& getClassName() const noexcept { return className_; }
    int getEnrollYear() const noexcept { return enrollYear_; }

    void setMajor(std::string major);
    void setClassName(std::string className);
    void setEnrollYear(int year);

    std::string roleLabel() const override;

    void writeTo(BinaryWriter& w) const;
    static Student readFrom(BinaryReader& r);

private:
    std::string major_;
    std::string className_;
    int enrollYear_ = 0;
};

} // namespace EduSys
