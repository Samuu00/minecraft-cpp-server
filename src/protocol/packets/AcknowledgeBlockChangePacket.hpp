#pragma once

#include "protocol/Packet.hpp"
#include <cstdint>

namespace mc {
    class AcknowledgeBlockChangePacket : public Packet {
        private:
            int32_t m_sequence;
        public:
            AcknowledgeBlockChangePacket(int32_t sequence) : m_sequence(sequence) {}

            [[nodiscard]] int32_t getId() const override { return 0x05; } // Acknowledge Block Change in 1.20.4
            [[nodiscard]] ConnectionState getState() const override { return ConnectionState::Play; }

            void write(ByteBuffer& buffer) const override {
                buffer.writeVarInt(m_sequence);
            }

            void read(ByteBuffer& buffer) override { (void)buffer; }
    };
}
