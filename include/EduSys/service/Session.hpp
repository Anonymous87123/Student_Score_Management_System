#pragma once

#include <string>

#include "EduSys/common/Types.hpp"

namespace EduSys {

// 会话上下文：描述"当前是谁、什么角色、关联哪个业务实体"。
// 纯值对象，不持有任何仓储 / 服务引用；由调用方按需构造并传入 Service。
// 未登录态：loggedIn_ = false，其余字段空白。
class Session {
public:
    Session() = default;

    bool isLoggedIn() const noexcept { return loggedIn_; }
    const std::string& getUsername() const noexcept { return username_; }
    RoleType getRole() const noexcept { return role_; }
    const std::string& getOwnerId() const noexcept { return ownerId_; }

    bool isAdmin()   const noexcept { return loggedIn_ && role_ == RoleType::Admin; }
    bool isTeacher() const noexcept { return loggedIn_ && role_ == RoleType::Teacher; }
    bool isStudent() const noexcept { return loggedIn_ && role_ == RoleType::Student; }

    void login(std::string username, RoleType role, std::string ownerId) {
        username_ = std::move(username);
        role_     = role;
        ownerId_  = std::move(ownerId);
        loggedIn_ = true;
    }

    void logout() noexcept {
        username_.clear();
        ownerId_.clear();
        role_     = RoleType::Student;
        loggedIn_ = false;
    }

private:
    bool        loggedIn_ = false;
    std::string username_;
    RoleType    role_ = RoleType::Student;
    std::string ownerId_;
};

} // namespace EduSys
