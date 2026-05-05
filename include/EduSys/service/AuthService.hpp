#pragma once

#include <string>

#include "EduSys/model/UserAccount.hpp"
#include "EduSys/storage/UserRepository.hpp"

namespace EduSys {

class Session;

// 认证服务：唯一处理用户名/密码校验的入口。
// 不直接修改 Session（由调用方根据返回的 UserAccount 自行 login），保持单一职责。
class AuthService {
public:
    explicit AuthService(UserRepository& userRepo) : userRepo_(userRepo) {}

    // 校验账号 + 密码 + enabled 状态；任一失败抛 AuthException 并写日志。
    UserAccount authenticate(const std::string& username,
                             const std::string& password);

    // 修改密码：仅允许已登录账号修改自身密码；旧密码必须正确。
    void changePassword(const Session& session,
                        const std::string& oldPassword,
                        const std::string& newPassword);

private:
    UserRepository& userRepo_;
};

} // namespace EduSys
