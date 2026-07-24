#pragma once

#include "protocol/Packet.hpp"

namespace mc {

    class EntityMetadataPacket : public Packet {
    private:
        int32_t m_entityId;
        uint16_t m_itemId;

    public:
        EntityMetadataPacket(int32_t entityId, uint16_t itemId)
            : m_entityId(entityId), m_itemId(itemId) {}

        [[nodiscard]] int32_t getId() const override { return 0x56; } // 1.20.4 Set Entity Metadata
        [[nodiscard]] ConnectionState getState() const override { return ConnectionState::Play; }

        void write(ByteBuffer& buffer) const override {
            buffer.writeVarInt(m_entityId);
            
            // Per gli Item Entities, il metadata index per lo Slot è 8.
            // Il tipo di metadata per lo slot è 7.
            buffer.writeByte(8); // Index
            buffer.writeVarInt(7); // Type (Slot)
            
            // Scriviamo lo Slot data
            buffer.writeBoolean(true); // Present
            buffer.writeVarInt(m_itemId); // Item ID
            buffer.writeByte(1); // Count
            // Componenti NBT (1.20.5+ sono componenti, 1.20.4 è NBT. In 1.20.4, un NBT tag vuoto è uno 0)
            buffer.writeByte(0); // Nessun NBT tag

            buffer.writeByte(static_cast<int8_t>(255));
        }

        void read(ByteBuffer& buffer) override { (void)buffer; }
    };

}
