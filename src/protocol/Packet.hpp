#pragma once

#include <cstdint>
#include <memory>
#include "ByteBuffer.hpp"

namespace mc {
    
    enum class ConnectionState{
        Handshaking = 0,
        Status = 1,
        Login = 2,
        Play = 3
    };

    enum class PacketDirection{
        ServerBound,
        ClientBound
    };

    class Packet{
        public:
            virtual ~Packet() = default;

            [[nodiscard]] virtual int32_t getId() const = 0;
            [[nodiscard]] virtual ConnectionState getState() const = 0;

            virtual void read(ByteBuffer& buffer) = 0;
            virtual void write(ByteBuffer& buffer) const = 0;
    };

    using PacketPtr = std::unique_ptr<Packet>;
}
