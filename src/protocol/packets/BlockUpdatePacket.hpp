#pragma once

#include "protocol/Packet.hpp"
#include <cstdint>

namespace mc {
    class BlockUpdatePacket : public Packet {
        private:
            int64_t m_position; // Position encoded as long
            uint16_t m_blockStateId; // VarInt in protocol, but uint16_t is fine here
        public:
            BlockUpdatePacket(int32_t x, int32_t y, int32_t z, uint16_t blockStateId) 
                : m_blockStateId(blockStateId) {
                m_position = ((static_cast<int64_t>(x & 0x3FFFFFF) << 38) |
                              (static_cast<int64_t>(z & 0x3FFFFFF) << 12) |
                              (static_cast<int64_t>(y & 0xFFF)));
            }

            [[nodiscard]] int32_t getId() const override { return 0x09; } // Block Update in 1.20.4
            [[nodiscard]] ConnectionState getState() const override { return ConnectionState::Play; }

            void write(ByteBuffer& buffer) const override {
                buffer.writeLong(m_position);
                buffer.writeVarInt(m_blockStateId);
            }

            void read(ByteBuffer& buffer) override { (void)buffer; }
    };
}
