#include "EduSys/common/PasswordHasher.hpp"

#include <cstdint>
#include <sstream>
#include <iomanip>

namespace EduSys {

namespace {

// 固定盐：与项目同寿命，单平台课程项目可接受硬编码。
constexpr const char* kSalt = "EduSys::salt::v1";

// FNV-1a 64-bit 标准初值与质数。
constexpr std::uint64_t kFnvOffset = 1469598103934665603ULL;
constexpr std::uint64_t kFnvPrime  = 1099511628211ULL;

std::uint64_t fnv1a64(const std::string& data) {
    std::uint64_t h = kFnvOffset;
    for (unsigned char c : data) {
        h ^= static_cast<std::uint64_t>(c);
        h *= kFnvPrime;
    }
    return h;
}

std::string toHex16(std::uint64_t v) {
    std::ostringstream oss;
    oss << std::hex << std::setw(16) << std::setfill('0') << v;
    return oss.str();
}

} // namespace

std::string PasswordHasher::hash(const std::string& plaintext) {
    return toHex16(fnv1a64(std::string(kSalt) + plaintext));
}

bool PasswordHasher::verify(const std::string& plaintext, const std::string& storedHash) {
    if (storedHash.empty()) {
        return false;
    }
    return hash(plaintext) == storedHash;
}

} // namespace EduSys
