#pragma once

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
        #define WIN32_LEAN_AND_MEAN
    #endif
    
    #include <winsock2.h>
    #include <ws2tcpip.h>

#endif

#include <string>
#include <cstdint>

class Socket{
    private:
        SOCKET m_handle{INVALID_SOCKET};

    public:
        Socket();
        explicit Socket(SOCKET handle);
        ~Socket();

        Socket(const Socket&) = delete;
        Socket& operator=(const Socket&) = delete;

        Socket(Socket&& other) noexcept;
        Socket& operator=(Socket&& other) noexcept;

        bool create();
        bool bind(uint16_t port);
        bool listen(int backlog = SOMAXCONN);

        bool setNonBlocking(bool enable = true);
        bool setReuseAddr(bool enable = true);

        SOCKET accept(std::string& clientIp, uint16_t& clientPort) const;
        void close();

        [[nodiscard]] SOCKET getHandle() const { return m_handle; }
        [[nodiscard]] bool isValid() const { return m_handle != INVALID_SOCKET; }
}; 
