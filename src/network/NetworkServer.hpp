#pragma once

#include "Socket.hpp"
#include "ClientConnection.hpp"

#include <vector>
#include <memory>
#include <cstdint>
#include <windows.h>
#include <winnt.h>
#include <winsock2.h>

class World;

class NetworkServer{
    private:
        Socket m_listenSocket;
        uint16_t m_port;
        bool m_active{false};
        World* m_world{nullptr};

        std::vector<std::shared_ptr<ClientConnection>> m_clients;
        std::vector<WSAPOLLFD> m_pollFds;

        void acceptNewConnections();
        void processClientIO();
        void cleanupDisconnectedClients();

    public:
        explicit NetworkServer(uint16_t port, World* world = nullptr);
        ~NetworkServer();
        
        NetworkServer(const NetworkServer&) = delete;
        NetworkServer& operator=(const NetworkServer&) = delete;

        void pollEvents();
        void tick(uint64_t currentTick);
        void stop();

        [[nodiscard]] size_t getClientCount() const { return m_clients.size(); }
        [[nodiscard]] World* getWorld() const { return m_world; }
};
