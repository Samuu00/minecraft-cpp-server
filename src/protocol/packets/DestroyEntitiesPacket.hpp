#pragma once

#include "protocol/Packet.hpp"
#include <vector>

namespace mc {

    class DestroyEntitiesPacket : public Packet {
    private:
        std::vector<int32_t> m_entityIds;

    public:
        DestroyEntitiesPacket(const std::vector<int32_t>& entityIds)
            : m_entityIds(entityIds) {}

        [[nodiscard]] int32_t getId() const override { return 0x40; } // 1.20.4 Destroy Entities
        [[nodiscard]] ConnectionState getState() const override { return ConnectionState::Play; }

        void write(ByteBuffer& buffer) const override {
            buffer.writeVarInt(static_cast<int32_t>(m_entityIds.size()));
            for (int32_t id : m_entityIds) {
                buffer.writeVarInt(id);
            }
        }

        void read(ByteBuffer& buffer) override { (void)buffer; }
    };

}
