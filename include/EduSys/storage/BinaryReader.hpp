#pragma once

#include <cstdint>
#include <fstream>
#include <string>

namespace EduSys {

// 显式逐字段二进制读取器：与 BinaryWriter 对称。
// 任何读失败（EOF / IO / 长度非法）都抛 StorageException，不吞错。
class BinaryReader {
public:
    explicit BinaryReader(const std::string& path);
    ~BinaryReader();

    BinaryReader(const BinaryReader&)            = delete;
    BinaryReader& operator=(const BinaryReader&) = delete;

    std::uint8_t  readUint8();
    std::int32_t  readInt32();
    std::uint32_t readUint32();
    double        readDouble();
    void          readBytes(char* out, std::size_t n);
    std::string   readString();

    bool eof() const;

private:
    std::string  path_;
    std::ifstream in_;
};

} // namespace EduSys
