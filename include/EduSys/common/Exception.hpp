#pragma once

#include <stdexcept>
#include <string>

namespace EduSys {

// 项目统一异常基类，派生类表示不同错误域，便于上层分类捕获。
class EduException : public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

class StorageException : public EduException {
public:
    using EduException::EduException;
};

class AuthException : public EduException {
public:
    using EduException::EduException;
};

class ValidationException : public EduException {
public:
    using EduException::EduException;
};

class PermissionException : public EduException {
public:
    using EduException::EduException;
};

} // namespace EduSys
