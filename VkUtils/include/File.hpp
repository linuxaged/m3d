#pragma once

#include <cstdio>
#include <type_traits>

namespace m3d {
namespace file {
    template <class T>
    bool readBinary(const char* path, T& container)
    {
        static_assert(std::is_same<T, std::vector<uint8_t>>::value || std::is_same<T, std::string>::value, "T must be std::vector<uint8_t> or std::string");
        std::FILE* fp = std::fopen(path, "rb");
        if (fp) {
            std::fseek(fp, 0, SEEK_END);
            container.resize(std::ftell(fp));
            std::rewind(fp);
            std::fread(&container[0], 1, container.size(), fp);
            std::fclose(fp);

            return true;
        } else {
            return false;
        }
    }
}
}
