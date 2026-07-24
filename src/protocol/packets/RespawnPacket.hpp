#pragma once

#include "protocol/Packet.hpp"
#include <string>

namespace mc {

class RespawnPacket : public Packet {
    private:
        std::string m_dimensionType{"minecraft:overworld"};
        std::string m_dimensionName{"minecraft:overworld"};
        uint8_t m_gameMode{0};
        
    public:
        explicit RespawnPacket(uint8_t gameMode) : m_gameMode(gameMode) {}

        [[nodiscard]] int32_t getId() const override { return 0x43; } // 1.20.4 Respawn

        [[nodiscard]] ConnectionState getState() const override { 
            return ConnectionState::Play; 
        }

        void write(ByteBuffer& buffer) const override {
            buffer.writeString(m_dimensionType);
            buffer.writeString(m_dimensionName);
            buffer.writeLong(0LL); // Hashed Seed
            buffer.writeByte(m_gameMode); // GameMode
            buffer.writeByte(-1); // Previous GameMode
            buffer.writeBoolean(false); // Is Debug
            buffer.writeBoolean(false); // Is Flat
            buffer.writeByte(0); // Data Kept (flags)
            buffer.writeBoolean(false); // Has Death Location
            buffer.writeVarInt(0); // Portal Cooldown
        }

        void read(ByteBuffer& buffer) override { (void)buffer; }
    };

} 
