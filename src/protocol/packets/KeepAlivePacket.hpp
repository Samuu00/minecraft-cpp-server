#pragma once

#include "../Packet.hpp"
#include <cstdint>

namespace mc {
    class ClientboundKeepAlivePacket : public Packet {
    private:
        int64_t m_keepAliveId;
    public:
        ClientboundKeepAlivePacket(int64_t keepAliveId = 0) : m_keepAliveId(keepAliveId) {}

        [[nodiscard]] int32_t getId() const override { return 0x24; } // 1.20.4 Play state Clientbound Keep Alive
        [[nodiscard]] ConnectionState getState() const override { return ConnectionState::Play; }

        void read(ByteBuffer& buffer) override {
            m_keepAliveId = buffer.readLong();
        }

        void write(ByteBuffer& buffer) const override {
            buffer.writeLong(m_keepAliveId);
        }
    };
}
