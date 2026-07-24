#pragma once

#include <vector>
#include <string>
#include <cstdint>
#include <cstddef>
#include <span>

#include "../utils/Types.hpp"

class ByteBuffer{
    private:
        std::vector<uint8_t> m_buffer;
        size_t m_readPos{0};

        void ensureCanRead(size_t length) const;

    public:
        ByteBuffer() = default;
        explicit ByteBuffer(size_t capacity);
        explicit ByteBuffer(std::vector<uint8_t> buffer);

        // -------------------------------------------------------------------------
        // Scrittura (Serialization)
        // -------------------------------------------------------------------------
        void writeByte(int8_t value);
        void writeUnsignedByte(uint8_t value);
        void writeBoolean(bool value);
        void writeShort(int16_t value);
        void writeUnsignedShort(uint16_t value);
        void writeInt(int32_t value);
        void writeLong(int64_t value);
        void writeFloat(float value);
        void writeDouble(double value);

        // Scrittura Tipi Speciali Minecraft
        void writeVarInt(int32_t value);
        void writeVarLong(int64_t value);
        void writeString(const std::string& value);
        void writeUUID(const mc::UUID& uuid);
        void writeBytes(std::span<const uint8_t> bytes);

        // -------------------------------------------------------------------------
        // Lettura (Deserialization)
        // -------------------------------------------------------------------------
        [[nodiscard]] int8_t readByte();
        [[nodiscard]] uint8_t readUnsignedByte();
        [[nodiscard]] bool readBoolean();
        [[nodiscard]] int16_t readShort();
        [[nodiscard]] uint16_t readUnsignedShort();
        [[nodiscard]] int32_t readInt();
        [[nodiscard]] int64_t readLong();
        [[nodiscard]] float readFloat();
        [[nodiscard]] double readDouble();

        // Lettura Tipi Speciali Minecraft
        [[nodiscard]] int32_t readVarInt();
        [[nodiscard]] int64_t readVarLong();
        [[nodiscard]] std::string readString(size_t maxLength = 32767);
        [[nodiscard]] mc::UUID readUUID();
        std::vector<uint8_t> readBytes(size_t length);

        // -------------------------------------------------------------------------
        // Utility
        // -------------------------------------------------------------------------
        void clear();
        [[nodiscard]] size_t size() const { return m_buffer.size(); }
        [[nodiscard]] size_t readableBytes() const { return m_buffer.size() - m_readPos; }
        [[nodiscard]] size_t getReadPos() const { return m_readPos; }
        void setReadPos(size_t pos) { m_readPos = pos; }

        [[nodiscard]] const std::vector<uint8_t>& vector() const { return m_buffer; }
        [[nodiscard]] const uint8_t* data() const { return m_buffer.data(); }

        // Calcola quanti byte occupa un dato VarInt (utilissimo per costruire gli header dei pacchetti)
        static size_t getVarIntSize(int32_t value);
};


