#pragma once

#include <string>

namespace EduSys {

// 课程项目级密码哈希：FNV-1a 64-bit + 固定盐 -> 16 字符十六进制摘要。
// 目的仅是避免明文落盘 / 保证跨平台稳定，不宣称达到密码学安全级别。
// 真实生产应改用 bcrypt / Argon2 等慢哈希；这里的取舍受 C++17 标准库约束。
class PasswordHasher {
public:
    // 计算 plaintext 的摘要；返回 16 字符小写 hex。
    static std::string hash(const std::string& plaintext);

    // 验证 plaintext 与已存哈希是否一致；空摘要直接判失败。
    static bool verify(const std::string& plaintext, const std::string& storedHash);
};

} // namespace EduSys
