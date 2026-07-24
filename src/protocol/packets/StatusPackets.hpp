#pragma once

#include "protocol/ByteBuffer.hpp"
#include "protocol/Packet.hpp"
#include <cstdint>
#include <string>

namespace mc {
    
    class StatusRequestPacket : public Packet{
        public:
            [[nodiscard]] int32_t getId() const override { return 0x00; }
            [[nodiscard]] ConnectionState getState() const override { return ConnectionState::Status; }

            void read(ByteBuffer& /*buffer*/) override {}

            void write(ByteBuffer& /*buffer*/) const override {}
    };

    class StatusResponsePacket : public Packet{
        public:
            std::string jsonResponse;

            StatusResponsePacket() = default;
            explicit StatusResponsePacket(std::string response) : jsonResponse(std::move(response)) {}

            [[nodiscard]] int32_t getId() const override { return 0x00; }
            [[nodiscard]] ConnectionState getState() const override { return ConnectionState::Status; }

            void read(ByteBuffer& buffer) override {
                jsonResponse = buffer.readString(32767);
            }

            void write(ByteBuffer& buffer) const override {
                buffer.writeString(jsonResponse);
            }
    };

    class PingPacket : public Packet{
        public:
            int64_t payload{0};

            PingPacket() = default;
            explicit PingPacket(int64_t timePayload) : payload(timePayload) {}

            [[nodiscard]] int32_t getId() const override { return 0x01; }
            [[nodiscard]] ConnectionState getState() const override { return ConnectionState::Status; }

            void read(ByteBuffer& buffer) override {
                payload = buffer.readLong();
            }

            void write(ByteBuffer& buffer) const override {
                buffer.writeLong(payload);
            }
    };
}
