#include "EduSys/model/UserAccount.hpp"

#include <utility>

#include "EduSys/common/Exception.hpp"
#include "EduSys/storage/BinaryReader.hpp"
#include "EduSys/storage/BinaryWriter.hpp"

namespace EduSys {

UserAccount::UserAccount(std::string username,
                         std::string passwordHash,
                         RoleType role,
                         std::string ownerId,
                         bool enabled)
    : username_(std::move(username)),
      passwordHash_(std::move(passwordHash)),
      role_(role),
      ownerId_(std::move(ownerId)),
      enabled_(enabled) {}

void UserAccount::setPasswordHash(std::string hash) { passwordHash_ = std::move(hash); }
void UserAccount::setOwnerId(std::string ownerId)   { ownerId_ = std::move(ownerId); }

void UserAccount::writeTo(BinaryWriter& w) const {
    w.writeString(username_);
    w.writeString(passwordHash_);
    w.writeUint8(static_cast<std::uint8_t>(role_));
    w.writeString(ownerId_);
    w.writeUint8(enabled_ ? 1u : 0u);
}

UserAccount UserAccount::readFrom(BinaryReader& r) {
    UserAccount u;
    u.username_     = r.readString();
    u.passwordHash_ = r.readString();
    const std::uint8_t roleByte = r.readUint8();
    if (roleByte > static_cast<std::uint8_t>(RoleType::Student)) {
        throw StorageException("Invalid RoleType value in users.dat");
    }
    u.role_         = static_cast<RoleType>(roleByte);
    u.ownerId_      = r.readString();
    u.enabled_      = (r.readUint8() != 0);
    return u;
}

} // namespace EduSys
