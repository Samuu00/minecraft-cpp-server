#pragma once

#include <cstdint>
#include <string>
#include <array>

namespace mc {
    using VarInt        = int32_t;
    using VarLong       = int64_t;
    using Byte          = int8_t;
    using UnsignedByte  = uint8_t;
    using Short         = int16_t;
    using UnsignedShort = uint16_t;
    using Int           = int32_t;
    using Long          = int64_t;
    using Float         = float;
    using Double        = double;
    using Boolean       = bool;

    struct BlockPosition{
        int32_t x{0};
        int32_t y{0};
        int32_t z{0};

        [[nodiscard]] uint64_t encode() const {
            return ((static_cast<uint64_t>(x) & 0x3FFFFFF) << 38) |
               ((static_cast<uint64_t>(z) & 0x3FFFFFF) << 12) |
                (static_cast<uint64_t>(y) & 0xFFF);
        }

        static BlockPosition decode(uint64_t val){
            int32_t x = static_cast<int32_t>(val >> 38);
            int32_t y = static_cast<int32_t>(val & 0xFFF);
            int32_t z = static_cast<int32_t>((val >> 12) & 0x3FFFFFF);

            if (x >= (1 << 25)) x -= (1 << 26);
            if (y >= (1 << 11)) y -= (1 << 12);
            if (z >= (1 << 25)) z -= (1 << 26);

            return {x, y, z};
        }
    };

    struct chunkPosition{
        int32_t chunkX{0};
        int32_t chunkZ{0};

        bool operator==(const chunkPosition& other) const {
            return chunkX == other.chunkX && chunkZ == other.chunkZ;
        }
    };

    struct UUID{
        std::array<uint8_t, 16> bytes{};

        [[nodiscard]] std::string toString() const {
            char buf[37];
            snprintf(buf, sizeof(buf),
                "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                bytes[0], bytes[1], bytes[2], bytes[3],
                bytes[4], bytes[5],
                bytes[6], bytes[7],
                bytes[8], bytes[9],
                bytes[10], bytes[11], bytes[12], bytes[13], bytes[14], bytes[15]);
            return std::string(buf);
        }
    };
}
