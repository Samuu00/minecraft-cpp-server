#pragma once

#include <vector>
#include <cstdint>
#include "protocol/Packet.hpp"
#include <protocol/ByteBuffer.hpp>

class ClientConnection;

namespace mc {

    class PacketHandler{
        private:
            static void handlePacket(ClientConnection& client, int32_t packetId, ByteBuffer& payloadBuffer);
        public:
            PacketHandler() = default;

            static bool processIncomingData(ClientConnection& client, std::vector<uint8_t>& receiveBuffer);
            static ByteBuffer serializePacket(const Packet& packet);
    };
}
