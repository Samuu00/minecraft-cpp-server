#pragma once

#include "protocol/Packet.hpp"
#include <string>
#include <cstdint>

namespace mc {

    class CommandsPacket : public Packet {
    public:
        CommandsPacket() {}

        [[nodiscard]] int32_t getId() const override { return 0x11; } // 1.20.4 Commands
        [[nodiscard]] ConnectionState getState() const override { return ConnectionState::Play; }

        void write(ByteBuffer& buffer) const override {
            // Count of nodes: 9
            buffer.writeVarInt(9);

            // Node 0: Root
            buffer.writeByte(0x00);
            buffer.writeVarInt(4); // Children count
            buffer.writeVarInt(1); // gamemode
            buffer.writeVarInt(6); // tp
            buffer.writeVarInt(7); // time
            buffer.writeVarInt(8); // spawn

            // Node 1: gamemode
            buffer.writeByte(0x01); // Literal
            buffer.writeVarInt(4); // 4 figli
            buffer.writeVarInt(2); // survival
            buffer.writeVarInt(3); // creative
            buffer.writeVarInt(4); // spectator
            buffer.writeVarInt(5); // adventure
            buffer.writeString("gamemode");

            // Node 2: survival
            buffer.writeByte(0x05); // Literal | Executable
            buffer.writeVarInt(0);
            buffer.writeString("survival");

            // Node 3: creative
            buffer.writeByte(0x05);
            buffer.writeVarInt(0);
            buffer.writeString("creative");

            // Node 4: spectator
            buffer.writeByte(0x05);
            buffer.writeVarInt(0);
            buffer.writeString("spectator");

            // Node 5: adventure
            buffer.writeByte(0x05);
            buffer.writeVarInt(0);
            buffer.writeString("adventure");

            // Node 6: Literal "tp"
            buffer.writeByte(0x05);
            buffer.writeVarInt(0);
            buffer.writeString("tp");

            // Node 7: Literal "time"
            buffer.writeByte(0x05);
            buffer.writeVarInt(0);
            buffer.writeString("time");

            // Node 8: Literal "spawn"
            buffer.writeByte(0x05);
            buffer.writeVarInt(0);
            buffer.writeString("spawn");

            // Root Index
            buffer.writeVarInt(0);
        }

        void read(ByteBuffer& buffer) override { (void)buffer; }
    };
}
