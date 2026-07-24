#pragma once

#include "protocol/Packet.hpp"
#include <string>

namespace mc {

    class SpawnEntityPacket : public Packet {
    private:
        int32_t m_entityId;
        int32_t m_entityType;
        double m_x, m_y, m_z;
        int16_t m_vx, m_vy, m_vz; // Velocity / 8000.0

    public:
        SpawnEntityPacket(int32_t entityId, int32_t entityType, double x, double y, double z, double vx=0, double vy=0, double vz=0)
            : m_entityId(entityId), m_entityType(entityType), m_x(x), m_y(y), m_z(z) {
            m_vx = static_cast<int16_t>(vx * 8000.0);
            m_vy = static_cast<int16_t>(vy * 8000.0);
            m_vz = static_cast<int16_t>(vz * 8000.0);
        }

        [[nodiscard]] int32_t getId() const override { return 0x01; } // 1.20.4 Spawn Entity
        [[nodiscard]] ConnectionState getState() const override { return ConnectionState::Play; }

        void write(ByteBuffer& buffer) const override {
            buffer.writeVarInt(m_entityId);
            
            // UUID fittizio
            buffer.writeLong(0);
            buffer.writeLong(m_entityId);

            buffer.writeVarInt(m_entityType);
            buffer.writeDouble(m_x);
            buffer.writeDouble(m_y);
            buffer.writeDouble(m_z);
            buffer.writeByte(0); // Pitch
            buffer.writeByte(0); // Yaw
            buffer.writeByte(0); // Head Yaw
            buffer.writeVarInt(0); // Object Data
            buffer.writeShort(m_vx);
            buffer.writeShort(m_vy);
            buffer.writeShort(m_vz);
        }

        void read(ByteBuffer& buffer) override { (void)buffer; }
    };

}
