#include "EduSys/storage/BinaryWriter.hpp"

#include "EduSys/common/Exception.hpp"

namespace EduSys {

BinaryWriter::BinaryWriter(const std::string& path)
    : path_(path),
      out_(path, std::ios::out | std::ios::binary | std::ios::trunc) {
    if (!out_.is_open()) {
        throw StorageException("Failed to open for write: " + path_);
    }
}

BinaryWriter::~BinaryWriter() {
    if (out_.is_open()) {
        out_.flush();
        out_.close();
    }
}

void BinaryWriter::writeBytes(const char* data, std::size_t n) {
    out_.write(data, static_cast<std::streamsize>(n));
    if (!out_) {
        throw StorageException("Write failed: " + path_);
    }
}

void BinaryWriter::writeUint8(std::uint8_t v) {
    writeBytes(reinterpret_cast<const char*>(&v), sizeof(v));
}

void BinaryWriter::writeInt32(std::int32_t v) {
    writeBytes(reinterpret_cast<const char*>(&v), sizeof(v));
}

void BinaryWriter::writeUint32(std::uint32_t v) {
    writeBytes(reinterpret_cast<const char*>(&v), sizeof(v));
}

void BinaryWriter::writeDouble(double v) {
    writeBytes(reinterpret_cast<const char*>(&v), sizeof(v));
}

void BinaryWriter::writeString(const std::string& s) {
    writeUint32(static_cast<std::uint32_t>(s.size()));
    if (!s.empty()) {
        writeBytes(s.data(), s.size());
    }
}

} // namespace EduSys
