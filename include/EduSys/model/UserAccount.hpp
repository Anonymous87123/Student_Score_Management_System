#pragma once

#include <string>

#include "EduSys/common/Types.hpp"

namespace EduSys {

class BinaryReader;
class BinaryWriter;

// 登录账户：与 Person 解耦，管理员账号可不对应任何业务实体。
class UserAccount {
public:
    UserAccount() = default;
    UserAccount(std::string username,
                std::string passwordHash,
                RoleType role,
                std::string ownerId,
                bool enabled = true);

    const std::string& getUsername() const noexcept { return username_; }
    const std::string& getPasswordHash() const noexcept { return passwordHash_; }
    RoleType getRole() const noexcept { return role_; }
    const std::string& getOwnerId() const noexcept { return ownerId_; }
    bool isEnabled() const noexcept { return enabled_; }

    void setPasswordHash(std::string hash);
    void setRole(RoleType role) noexcept { role_ = role; }
    void setOwnerId(std::string ownerId);
    void setEnabled(bool enabled) noexcept { enabled_ = enabled; }

    void writeTo(BinaryWriter& w) const;
    static UserAccount readFrom(BinaryReader& r);

private:
    std::string username_;
    std::string passwordHash_;
    RoleType    role_    = RoleType::Student;
    std::string ownerId_;              // 关联的 Student/Teacher 编号；管理员可为空。
    bool        enabled_ = true;
};

} // namespace EduSys
