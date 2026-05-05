#pragma once

#include <string>
#include <utility>

namespace EduSys {

// 人员抽象基类：只抽取编号、姓名、联系方式等人共性字段。
// 不承担密码存储、菜单展示、成绩计算等职责。
class Person {
public:
    Person() = default;
    Person(std::string id, std::string name, std::string contact)
        : id_(std::move(id)), name_(std::move(name)), contact_(std::move(contact)) {}

    virtual ~Person() = default;

    const std::string& getId() const noexcept { return id_; }
    const std::string& getName() const noexcept { return name_; }
    const std::string& getContact() const noexcept { return contact_; }

    void setName(std::string name) { name_ = std::move(name); }
    void setContact(std::string contact) { contact_ = std::move(contact); }

    // 角色标签，派生类必须实现。
    virtual std::string roleLabel() const = 0;

protected:
    std::string id_;
    std::string name_;
    std::string contact_;
};

} // namespace EduSys
