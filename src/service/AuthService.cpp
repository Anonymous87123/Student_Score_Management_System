#include "EduSys/service/AuthService.hpp"

#include <algorithm>

#include "EduSys/common/Exception.hpp"
#include "EduSys/common/Logger.hpp"
#include "EduSys/common/PasswordHasher.hpp"
#include "EduSys/service/Session.hpp"

namespace EduSys {

namespace {

// 在仓储 vector 中按 username 线性查找；课程项目数据量小，O(n) 足够。
std::vector<UserAccount>::iterator
findByUsername(std::vector<UserAccount>& users, const std::string& username) {
    return std::find_if(users.begin(), users.end(),
        [&](const UserAccount& u) { return u.getUsername() == username; });
}

} // namespace

UserAccount AuthService::authenticate(const std::string& username,
                                      const std::string& password) {
    auto users = userRepo_.loadAll();
    auto it = findByUsername(users, username);

    if (it == users.end()) {
        Logger::instance().warn("Auth failed: unknown user '" + username + "'");
        throw AuthException("Invalid username or password");
    }
    if (!it->isEnabled()) {
        Logger::instance().warn("Auth failed: account disabled '" + username + "'");
        throw AuthException("Account is disabled");
    }
    if (!PasswordHasher::verify(password, it->getPasswordHash())) {
        Logger::instance().warn("Auth failed: bad password for '" + username + "'");
        throw AuthException("Invalid username or password");
    }

    Logger::instance().info("Auth OK: '" + username + "'");
    return *it;
}

void AuthService::changePassword(const Session& session,
                                 const std::string& oldPassword,
                                 const std::string& newPassword) {
    if (!session.isLoggedIn()) {
        throw AuthException("Not logged in");
    }
    if (newPassword.empty()) {
        throw ValidationException("New password must not be empty");
    }

    auto users = userRepo_.loadAll();
    auto it = findByUsername(users, session.getUsername());
    if (it == users.end()) {
        throw AuthException("Session user no longer exists: " + session.getUsername());
    }
    if (!PasswordHasher::verify(oldPassword, it->getPasswordHash())) {
        Logger::instance().warn("ChangePassword failed: bad old password for '"
                                + session.getUsername() + "'");
        throw AuthException("Old password is incorrect");
    }

    it->setPasswordHash(PasswordHasher::hash(newPassword));
    userRepo_.saveAll(users);
    Logger::instance().info("Password changed for '" + session.getUsername() + "'");
}

} // namespace EduSys
