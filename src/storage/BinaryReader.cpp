#include "EduSys/storage/BinaryReader.hpp"

#include "EduSys/common/Exception.hpp"

namespace EduSys {

namespace {
// 字符串长度上限：保护输入不合理时不至于 OOM。
// 对课程作业数据规模而言，1 MiB 远超实际需要。
constexpr std::uint32_t kMaxStringLen = 1u << 20;
} // namespace

BinaryReader::BinaryReader(const std::string& path)
    : path_(path),
      in_(path, std::ios::in | std::ios::binary) {
    if (!in_.is_open()) {
        throw StorageException("Failed to open for read: " + path_);
    }
}

BinaryReader::~BinaryReader() {
    if (in_.is_open()) {
        in_.close();
    }
}

void BinaryReader::readBytes(char* out, std::size_t n) {
    in_.read(out, static_cast<std::streamsize>(n));
    if (in_.gcount() != static_cast<std::streamsize>(n)) {
        throw StorageException("Unexpected EOF reading: " + path_);
    }
}

std::uint8_t BinaryReader::readUint8() {
    std::uint8_t v = 0;
    readBytes(reinterpret_cast<char*>(&v), sizeof(v));
    return v;
}

std::int32_t BinaryReader::readInt32() {
    std::int32_t v = 0;
    readBytes(reinterpret_cast<char*>(&v), sizeof(v));
    return v;
}

std::uint32_t BinaryReader::readUint32() {
    std::uint32_t v = 0;
    readBytes(reinterpret_cast<char*>(&v), sizeof(v));
    return v;
}

double BinaryReader::readDouble() {
    double v = 0.0;
    readBytes(reinterpret_cast<char*>(&v), sizeof(v));
    return v;
}

std::string BinaryReader::readString() {
    const std::uint32_t len = readUint32();
    if (len > kMaxStringLen) {
        throw StorageException("String length out of range in: " + path_);
    }
    std::string s;
    if (len > 0) {
        s.resize(len);
        readBytes(s.data(), len);
    }
    return s;
}

bool BinaryReader::eof() const {
    return in_.eof();
}

} // namespace EduSys
