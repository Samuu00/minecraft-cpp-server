#pragma once

#include "protocol/Packet.hpp"
#include <cstdint>

namespace mc {
    class UpdateTagsPacket : public Packet {
    public:
        UpdateTagsPacket() {}

        [[nodiscard]] int32_t getId() const override { return 0x74; } // 1.20.4 Update Tags
        [[nodiscard]] ConnectionState getState() const override { return ConnectionState::Play; }

        void write(ByteBuffer& buffer) const override {
            buffer.writeVarInt(1); // 1 registry

            buffer.writeString("minecraft:fluid");
            buffer.writeVarInt(1); // 1 tag

            buffer.writeString("minecraft:water");
            buffer.writeVarInt(2); // 2 entries
            buffer.writeVarInt(1); // flowing_water
            buffer.writeVarInt(2); // water
        }

        void read(ByteBuffer& buffer) override { (void)buffer; }
    };
}
