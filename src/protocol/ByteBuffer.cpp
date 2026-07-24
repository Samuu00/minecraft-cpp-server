#include "ByteBuffer.hpp"
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <utility>
#include <vector>
#ifdef _WIN32
    #include <winsock2.h> 
#else
    #include <arpa/inet.h>
#endif

ByteBuffer::ByteBuffer(size_t capacity){ m_buffer.reserve(capacity); }

ByteBuffer::ByteBuffer(std::vector<uint8_t> buffer) 
    : m_buffer(std::move(buffer)), m_readPos(0) {}

void ByteBuffer::clear(){
    m_buffer.clear();
    m_readPos = 0;
}

void ByteBuffer::ensureCanRead(size_t length) const {
    if(m_readPos + length > m_buffer.size()) throw std::out_of_range("ByteBuffer underflow: tentato di leggere oltre la dimensione del buffer.");
}

void ByteBuffer::writeByte(int8_t value) {
    m_buffer.push_back(static_cast<uint8_t>(value));
}

void ByteBuffer::writeUnsignedByte(uint8_t value) {
    m_buffer.emplace_back(value);
}

void ByteBuffer::writeBoolean(bool value) {
    writeByte(value ? 1 : 0);
}

void ByteBuffer::writeShort(int16_t value) {
    uint16_t networkValue = htons(static_cast<uint16_t>(value));
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&networkValue);
    m_buffer.insert(m_buffer.end(), bytes, bytes + sizeof(uint16_t));
}

void ByteBuffer::writeUnsignedShort(uint16_t value) {
    writeShort(static_cast<int16_t>(value));
}

void ByteBuffer::writeInt(int32_t value) {
    uint32_t networkValue = htonl(static_cast<uint32_t>(value));
    const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&networkValue);
    m_buffer.insert(m_buffer.end(), bytes, bytes + sizeof(uint32_t));
}

void ByteBuffer::writeLong(int64_t value) {
    uint64_t uval = static_cast<uint64_t>(value);
    for (int i = 7; i >= 0; --i) {
        m_buffer.emplace_back(static_cast<uint8_t>((uval >> (i * 8)) & 0xFF));
    }
}

void ByteBuffer::writeFloat(float value){
    static_assert(sizeof(float) == sizeof(uint32_t), "float deve essere di 32-bit");
    uint32_t temp;
    std::memcpy(&temp, &value, sizeof(float));
    writeInt(static_cast<int32_t>(temp));
}

void ByteBuffer::writeDouble(double value) {
    static_assert(sizeof(double) == sizeof(uint64_t), "double deve essere di 64-bit");
    uint64_t temp;
    std::memcpy(&temp, &value, sizeof(double));
    writeLong(static_cast<int64_t>(temp));
}

void ByteBuffer::writeVarInt(int32_t value) {
    uint32_t uval = static_cast<uint32_t>(value);
    while (true) {
        if ((uval & ~0x7F) == 0) {
            m_buffer.emplace_back(static_cast<uint8_t>(uval));
            return;
        }
        m_buffer.emplace_back(static_cast<uint8_t>((uval & 0x7F) | 0x80));
        uval >>= 7;
    }
}

void ByteBuffer::writeVarLong(int64_t value) {
    uint64_t uval = static_cast<uint64_t>(value);
    while (true) {
        if ((uval & ~0x7FL) == 0) {
            m_buffer.emplace_back(static_cast<uint8_t>(uval));
            return;
        }
        m_buffer.emplace_back(static_cast<uint8_t>((uval & 0x7F) | 0x80));
        uval >>= 7;
    }
}

void ByteBuffer::writeString(const std::string& value) {
    writeVarInt(static_cast<int32_t>(value.size()));
    m_buffer.insert(m_buffer.end(), value.begin(), value.end());
}

void ByteBuffer::writeUUID(const mc::UUID& uuid) {
    m_buffer.insert(m_buffer.end(), uuid.bytes.begin(), uuid.bytes.end());
}

void ByteBuffer::writeBytes(std::span<const uint8_t> bytes) {
    m_buffer.insert(m_buffer.end(), bytes.begin(), bytes.end());
}

int8_t ByteBuffer::readByte(){
    ensureCanRead(1);
    return static_cast<int8_t>(m_buffer[m_readPos++]);
}

uint8_t ByteBuffer::readUnsignedByte(){
    ensureCanRead(1);
    return m_buffer[m_readPos++];
}

bool ByteBuffer::readBoolean(){
    return readByte() != 0;
}

int16_t ByteBuffer::readShort(){
    ensureCanRead(sizeof(int16_t));
    uint16_t networkValue;
    std::memcpy(&networkValue, &m_buffer[m_readPos], sizeof(uint16_t));
    m_readPos += sizeof(uint16_t);
    return static_cast<int16_t>(ntohs(networkValue));
}

uint16_t ByteBuffer::readUnsignedShort(){ return static_cast<uint16_t>(readShort()); }

int32_t ByteBuffer::readInt() {
    ensureCanRead(sizeof(int32_t));
    uint32_t networkValue;
    std::memcpy(&networkValue, &m_buffer[m_readPos], sizeof(uint32_t));
    m_readPos += sizeof(uint32_t);
    return static_cast<int32_t>(ntohl(networkValue));
}

int64_t ByteBuffer::readLong() {
    ensureCanRead(sizeof(int64_t));
    uint64_t uval = 0;
    for (int i = 0; i < 8; ++i) {
        uval = (uval << 8) | m_buffer[m_readPos++];
    }
    return static_cast<int64_t>(uval);
}

float ByteBuffer::readFloat() {
    int32_t val = readInt();
    float result;
    std::memcpy(&result, &val, sizeof(float));
    return result;
}

double ByteBuffer::readDouble() {
    int64_t val = readLong();
    double result;
    std::memcpy(&result, &val, sizeof(double));
    return result;
}

int32_t ByteBuffer::readVarInt() {
    int32_t value = 0;
    int position = 0;
    uint8_t currentByte;

    while (true) {
        ensureCanRead(1);
        currentByte = m_buffer[m_readPos++];
        value |= (currentByte & 0x7F) << position;

        if ((currentByte & 0x80) == 0) break;

        position += 7;
        if (position >= 32) {
            throw std::runtime_error("VarInt troppo grande (> 5 byte).");
        }
    }
    return value;
}

int64_t ByteBuffer::readVarLong() {
    int64_t value = 0;
    int position = 0;
    uint8_t currentByte;

    while (true) {
        ensureCanRead(1);
        currentByte = m_buffer[m_readPos++];
        value |= static_cast<int64_t>(currentByte & 0x7F) << position;

        if ((currentByte & 0x80) == 0) break;

        position += 14;
        if (position >= 64) {
            throw std::runtime_error("VarLong troppo grande (> 10 byte).");
        }
    }
    return value;
}

std::string ByteBuffer::readString(size_t maxLength) {
    int32_t length = readVarInt();
    if (length < 0) {
        throw std::runtime_error("Lunghezza stringa negativa non valida.");
    }
    if (static_cast<size_t>(length) > maxLength) {
        throw std::runtime_error("Stringa ricevuta supera la lunghezza massima consentita.");
    }

    ensureCanRead(static_cast<size_t>(length));
    std::string str(reinterpret_cast<const char*>(&m_buffer[m_readPos]), length);
    m_readPos += length;
    return str;
}

mc::UUID ByteBuffer::readUUID() {
    ensureCanRead(16);
    mc::UUID uuid;
    std::memcpy(uuid.bytes.data(), &m_buffer[m_readPos], 16);
    m_readPos += 16;
    return uuid;
}

std::vector<uint8_t> ByteBuffer::readBytes(size_t length) {
    ensureCanRead(length);
    std::vector<uint8_t> result(m_buffer.begin() + m_readPos, m_buffer.begin() + m_readPos + length);
    m_readPos += length;
    return result;
}

size_t ByteBuffer::getVarIntSize(int32_t value) {
    uint32_t uval = static_cast<uint32_t>(value);
    for (size_t i = 1; i < 5; ++i) {
        if (uval < (1u << (i * 7))) {
            return i;
        }
    }
    return 5;
}


