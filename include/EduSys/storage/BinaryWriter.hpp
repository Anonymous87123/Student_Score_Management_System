#pragma once

#include <cstdint>
#include <fstream>
#include <string>

namespace EduSys {

// 显式逐字段二进制写入器：禁止 reinterpret_cast 整块写对象。
// 数值类型按主机字节序写入（项目限定 Windows x86_64 单平台）；
// 字符串以 uint32_t 长度前缀 + 原始字节编码。
class BinaryWriter {
public:
    explicit BinaryWriter(const std::string& path);
    ~BinaryWriter();

    BinaryWriter(const BinaryWriter&)            = delete;
    BinaryWriter& operator=(const BinaryWriter&) = delete;

    void writeUint8(std::uint8_t v);
    void writeInt32(std::int32_t v);
    void writeUint32(std::uint32_t v);
    void writeDouble(double v);
    void writeBytes(const char* data, std::size_t n);
    void writeString(const std::string& s);

private:
    std::string  path_;
    std::ofstream out_;
};

} // namespace EduSys
