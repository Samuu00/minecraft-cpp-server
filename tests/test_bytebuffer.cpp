#include <gtest/gtest.h>
#include "protocol/ByteBuffer.hpp"

// -----------------------------------------------------------------------------
// Test VarInt Encoding & Decoding
// -----------------------------------------------------------------------------

TEST(ByteBufferTest, VarIntEncodingDecoding) {
    ByteBuffer buffer;

    // Test valori notevoli del protocollo Minecraft
    const int32_t testValues[] = {
        0,
        1,
        2,
        127,
        128,
        255,
        25565,      // Porta standard Minecraft
        2147483647, // Max Int32
        -1,
        -2147483648 // Min Int32
    };

    for (int32_t val : testValues) {
        buffer.writeVarInt(val);
    }

    for (int32_t expected : testValues) {
        int32_t actual = buffer.readVarInt();
        EXPECT_EQ(actual, expected) << "Fallito per valore VarInt: " << expected;
    }
}

TEST(ByteBufferTest, VarIntSizeCalculation) {
    EXPECT_EQ(ByteBuffer::getVarIntSize(0), 1);
    EXPECT_EQ(ByteBuffer::getVarIntSize(127), 1);
    EXPECT_EQ(ByteBuffer::getVarIntSize(128), 2);
    EXPECT_EQ(ByteBuffer::getVarIntSize(16383), 2);   // Max valore a 2 byte (2^14 - 1)
    EXPECT_EQ(ByteBuffer::getVarIntSize(25565), 3);   // 25565 > 16383 -> richiede 3 byte!
    EXPECT_EQ(ByteBuffer::getVarIntSize(2097151), 3); // Max valore a 3 byte (2^21 - 1)
    EXPECT_EQ(ByteBuffer::getVarIntSize(-1), 5);      // I negativi occupano sempre 5 byte
}

// -----------------------------------------------------------------------------
// Test Primitivi Big-Endian (Short, Int, Long, Float, Double)
// -----------------------------------------------------------------------------

TEST(ByteBufferTest, PrimitiveTypesBigEndian) {
    ByteBuffer buffer;

    buffer.writeShort(-32000);
    buffer.writeInt(123456789);
    buffer.writeLong(987654321098765432LL);
    buffer.writeFloat(3.14159f);
    buffer.writeDouble(2.718281828459045);

    EXPECT_EQ(buffer.readShort(), -32000);
    EXPECT_EQ(buffer.readInt(), 123456789);
    EXPECT_EQ(buffer.readLong(), 987654321098765432LL);
    EXPECT_FLOAT_EQ(buffer.readFloat(), 3.14159f);
    EXPECT_DOUBLE_EQ(buffer.readDouble(), 2.718281828459045);
}

// -----------------------------------------------------------------------------
// Test Stringhe UTF-8 e UUID
// -----------------------------------------------------------------------------

TEST(ByteBufferTest, StringSerialization) {
    ByteBuffer buffer;
    const std::string text = "Minecraft C++ Server Test 2026!";

    buffer.writeString(text);

    EXPECT_EQ(buffer.readString(), text);
}

TEST(ByteBufferTest, UUIDSerialization) {
    ByteBuffer buffer;
    mc::UUID uuid;
    for (size_t i = 0; i < 16; ++i) {
        uuid.bytes[i] = static_cast<uint8_t>(i * 10);
    }

    buffer.writeUUID(uuid);
    mc::UUID readUuid = buffer.readUUID();

    EXPECT_EQ(uuid.bytes, readUuid.bytes);
}

// -----------------------------------------------------------------------------
// Test Bounds Checking & Underflow
// -----------------------------------------------------------------------------

TEST(ByteBufferTest, UnderflowThrowsException) {
    ByteBuffer buffer;
    buffer.writeByte(42);

    EXPECT_EQ(buffer.readByte(), 42);
    
    // Tenta di leggere oltre la fine del buffer
    EXPECT_THROW((void)buffer.readInt(), std::out_of_range);
}
