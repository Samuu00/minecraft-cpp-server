#pragma once

#include "protocol/Packet.hpp"

namespace mc {

class PlayerPositionPacket : public Packet {
    private:
        double m_x{0.0};
        double m_y{64.0};
        double m_z{0.0};
        float m_yaw{0.0f};
        float m_pitch{0.0f};
        int32_t m_teleportId{1};

    public:
        PlayerPositionPacket(double x = 0.0, double y = 64.0, double z = 0.0, float yaw = 0.0f, float pitch = 0.0f)
            : m_x(x), m_y(y), m_z(z), m_yaw(yaw), m_pitch(pitch) {}

        [[nodiscard]] int32_t getId() const override { return 0x3E; } 

        [[nodiscard]] ConnectionState getState() const override { 
            return ConnectionState::Play; 
        }

        void write(ByteBuffer& buffer) const override {
            buffer.writeDouble(m_x);
            buffer.writeDouble(m_y);
            buffer.writeDouble(m_z);
            buffer.writeFloat(m_yaw);
            buffer.writeFloat(m_pitch);
            buffer.writeByte(0x00); // Flags (0 = coordinate assolute)
            buffer.writeVarInt(m_teleportId); // Teleport ID
        }

        void read(ByteBuffer& buffer) override { (void)buffer; }
    };
}
