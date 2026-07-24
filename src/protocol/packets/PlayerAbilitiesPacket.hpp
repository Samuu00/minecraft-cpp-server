#pragma once

#include "protocol/Packet.hpp"

namespace mc {

    class PlayerAbilitiesPacket : public Packet {
    private:
        int8_t m_flags;
        float m_flyingSpeed;
        float m_walkingSpeed;
    public:
        PlayerAbilitiesPacket(int8_t flags, float flyingSpeed, float walkingSpeed) 
            : m_flags(flags), m_flyingSpeed(flyingSpeed), m_walkingSpeed(walkingSpeed) {}

        [[nodiscard]] int32_t getId() const override { return 0x36; } // 1.20.4 Player Abilities
        [[nodiscard]] ConnectionState getState() const override { return ConnectionState::Play; }

        void write(ByteBuffer& buffer) const override {
            buffer.writeByte(m_flags);
            buffer.writeFloat(m_flyingSpeed);
            buffer.writeFloat(m_walkingSpeed);
        }

        void read(ByteBuffer& buffer) override { (void)buffer; }
    };

}
