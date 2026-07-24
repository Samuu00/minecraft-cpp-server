#pragma once

#include "protocol/Packet.hpp"

namespace mc {

    class EntityVelocityPacket : public Packet {
    private:
        int32_t m_entityId;
        int16_t m_vx, m_vy, m_vz; // Velocity / 8000.0

    public:
        EntityVelocityPacket(int32_t entityId, double vx, double vy, double vz)
            : m_entityId(entityId) {
            m_vx = static_cast<int16_t>(vx * 8000.0);
            m_vy = static_cast<int16_t>(vy * 8000.0);
            m_vz = static_cast<int16_t>(vz * 8000.0);
        }

        [[nodiscard]] int32_t getId() const override { return 0x56; } // 1.20.4 Set Entity Velocity
        [[nodiscard]] ConnectionState getState() const override { return ConnectionState::Play; }

        void write(ByteBuffer& buffer) const override {
            buffer.writeVarInt(m_entityId);
            buffer.writeShort(m_vx);
            buffer.writeShort(m_vy);
            buffer.writeShort(m_vz);
        }

        void read(ByteBuffer& buffer) override { (void)buffer; }
    };

}
