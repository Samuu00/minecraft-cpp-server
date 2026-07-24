#pragma once

#include "protocol/Packet.hpp"
#include <string>

namespace mc {

class HandshakePacket : public Packet {
    public:
        int32_t protocolVersion{0};
        std::string serverAddress;
        uint16_t serverPort{0};
        int32_t nextState{1}; // 1 per Status, 2 per Login

        [[nodiscard]] int32_t getId() const override { return 0x00; }
        [[nodiscard]] ConnectionState getState() const override { return ConnectionState::Handshaking; }

        void read(ByteBuffer& buffer) override {
            protocolVersion = buffer.readVarInt();
            serverAddress = buffer.readString(255);
            serverPort = buffer.readUnsignedShort();
            nextState = buffer.readVarInt();
        }

        void write(ByteBuffer& buffer) const override {
            buffer.writeVarInt(protocolVersion);
            buffer.writeString(serverAddress);
            buffer.writeUnsignedShort(serverPort);
            buffer.writeVarInt(nextState);
        }
    };

}   
