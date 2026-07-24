#pragma once

#include "protocol/Packet.hpp"
#include <cstdint>

namespace mc {
    class GameEventPacket : public Packet {
        private:
            uint8_t m_event;
            float m_value;
        public:
            GameEventPacket(uint8_t event, float value) : m_event(event), m_value(value) {}

            [[nodiscard]] int32_t getId() const override { return 0x20; }
            [[nodiscard]] ConnectionState getState() const override { return ConnectionState::Play; }

            void write(ByteBuffer& buffer) const override {
                buffer.writeByte(m_event);
                buffer.writeFloat(m_value);
            }

            void read(ByteBuffer& buffer) override { (void)buffer; }
    };
}
