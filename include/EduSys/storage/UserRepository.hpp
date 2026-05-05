#pragma once

#include "EduSys/common/Constants.hpp"
#include "EduSys/model/UserAccount.hpp"
#include "EduSys/storage/BinaryRepository.hpp"

namespace EduSys {

// Week 10: 仅绑定文件路径与实体类型；面向 username/role 的具体查询推迟到 Week 11 Service 层使用时再扩展。
class UserRepository : public BinaryRepository<UserAccount> {
public:
    UserRepository() : BinaryRepository<UserAccount>(USERS_FILE) {}
};

} // namespace EduSys
