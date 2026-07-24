#pragma once

#include "protocol/Packet.hpp"
#include <string>

namespace mc {

class JoinGamePacket : public Packet {
    private:
        int32_t m_entityId{1};
    public:
        explicit JoinGamePacket(int32_t entityId = 1) : m_entityId(entityId) {}

        [[nodiscard]] int32_t getId() const override { return 0x29; } // 1.20.4 Join Game

        [[nodiscard]] ConnectionState getState() const override { 
            return ConnectionState::Play; 
        }

        void write(ByteBuffer& buffer) const override {
        // Entity ID
            buffer.writeInt(m_entityId);
        
        // Is Hardcore
            buffer.writeBoolean(false);

        // Dimension Names Array (Count = 1, Identifier = "minecraft:overworld")
            buffer.writeVarInt(1);
            buffer.writeString("minecraft:overworld");

        // Max Players, View Distance, Simulation Distance
            buffer.writeVarInt(20);
            buffer.writeVarInt(8);
            buffer.writeVarInt(8);

        // Game Flags
            buffer.writeBoolean(false); // Reduced Debug Info
            buffer.writeBoolean(true);  // Enable Respawn Screen
            buffer.writeBoolean(false); // Do Limited Crafting

        // Dimension Type Name & Dimension Name
            buffer.writeString("minecraft:overworld");
            buffer.writeString("minecraft:overworld");

        // Hashed Seed
            buffer.writeLong(0LL);

        // GameMode (0 = Survival) & Previous GameMode (255 = None)
            buffer.writeByte(0);
            buffer.writeByte(-1);

        // World Flags
            buffer.writeBoolean(false); // Is Debug
            buffer.writeBoolean(false);  // Is Flat

        // Death Location (false = no death location)
            buffer.writeBoolean(false);

        // Portal Cooldown
            buffer.writeVarInt(0);
        }

        void read(ByteBuffer& buffer) override { (void)buffer; }
    };

} 
