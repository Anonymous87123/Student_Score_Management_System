#pragma once

#include <cerrno>
#include <cstdint>
#include <cstdio>
#include <string>
#include <utility>
#include <vector>

#include "EduSys/common/Exception.hpp"
#include "EduSys/storage/BinaryReader.hpp"
#include "EduSys/storage/BinaryWriter.hpp"

namespace EduSys {

// 模板仓储基类：只承担三件事——
//   1. 写入/校验统一文件头（magic + version + count）；
//   2. 整批加载（读头后循环调 T::readFrom）；
//   3. 整批覆写（写头后循环调 item.writeTo）。
// 实体字段布局完全由 T::writeTo / T::readFrom 决定，模板内部对 T 的字段一无所知。
//
// 文件头（12 字节）：
//   magic[4]   = 'E' 'D' 'S' 'Y'
//   version    = uint32  （当前 = 1）
//   count      = uint32  （后续记录条数）
template <typename T>
class BinaryRepository {
public:
    static constexpr char          kMagic[4]        = {'E', 'D', 'S', 'Y'};
    static constexpr std::uint32_t kFormatVersion   = 1u;

    explicit BinaryRepository(std::string path) : path_(std::move(path)) {}
    virtual ~BinaryRepository() = default;

    const std::string& getPath() const noexcept { return path_; }

    // 数据文件不存在时视作“空仓库”，返回空 vector；交由调用方决定是否注入初始样例。
    std::vector<T> loadAll() {
        if (!fileExists()) {
            return {};
        }
        BinaryReader r(path_);
        checkHeader(r);
        const std::uint32_t count = r.readUint32();
        std::vector<T> items;
        items.reserve(count);
        for (std::uint32_t i = 0; i < count; ++i) {
            items.push_back(T::readFrom(r));
        }
        return items;
    }

    // 整批覆写：每次都重写 header + 所有记录，不做增量写入。
    void saveAll(const std::vector<T>& items) {
        BinaryWriter w(path_);
        writeHeader(w, static_cast<std::uint32_t>(items.size()));
        for (const auto& item : items) {
            item.writeTo(w);
        }
    }

private:
    static void writeHeader(BinaryWriter& w, std::uint32_t count) {
        w.writeBytes(kMagic, sizeof(kMagic));
        w.writeUint32(kFormatVersion);
        w.writeUint32(count);
    }

    bool fileExists() const {
        std::FILE* file = nullptr;
#ifdef _MSC_VER
        const int openErr = ::fopen_s(&file, path_.c_str(), "rb");
#else
        errno = 0;
        file = std::fopen(path_.c_str(), "rb");
        const int openErr = errno;
#endif
        if (file != nullptr) {
            std::fclose(file);
            return true;
        }
        if (openErr == ENOENT) {
            return false;
        }
        throw StorageException("Failed to probe repository file: " + path_);
    }

    void checkHeader(BinaryReader& r) const {
        char magic[4] = {0};
        r.readBytes(magic, sizeof(magic));
        for (int i = 0; i < 4; ++i) {
            if (magic[i] != kMagic[i]) {
                throw StorageException("Bad magic in " + path_);
            }
        }
        const std::uint32_t ver = r.readUint32();
        if (ver != kFormatVersion) {
            throw StorageException("Unsupported version in " + path_);
        }
    }

    std::string path_;
};

template <typename T>
constexpr char BinaryRepository<T>::kMagic[4];

template <typename T>
constexpr std::uint32_t BinaryRepository<T>::kFormatVersion;

} // namespace EduSys
