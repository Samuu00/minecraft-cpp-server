#pragma once

#include "protocol/Packet.hpp"

namespace mc {

    class CollectItemPacket : public Packet {
    private:
        int32_t m_collectedEntityId;
        int32_t m_collectorEntityId;
        int32_t m_pickupItemCount;

    public:
        CollectItemPacket(int32_t collectedId, int32_t collectorId, int32_t count)
            : m_collectedEntityId(collectedId), m_collectorEntityId(collectorId), m_pickupItemCount(count) {}

        [[nodiscard]] int32_t getId() const override { return 0x69; } // 1.20.4 Pickup Item
        [[nodiscard]] ConnectionState getState() const override { return ConnectionState::Play; }

        void write(ByteBuffer& buffer) const override {
            buffer.writeVarInt(m_collectedEntityId);
            buffer.writeVarInt(m_collectorEntityId);
            buffer.writeVarInt(m_pickupItemCount);
        }

        void read(ByteBuffer& buffer) override { (void)buffer; }
    };

}
