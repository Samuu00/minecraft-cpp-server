#pragma once

#include <atomic>
#include <memory>
#include <cstdint>

class NetworkServer;
class World;

class Server{
    private:
        void init();
        void tick(double deltaTime);

        uint16_t m_port;
        std::atomic<bool> m_running{false};
        uint64_t m_tickCounter{0};

        std::unique_ptr<NetworkServer> m_networkServer;
        std::unique_ptr<World> m_world;

    public:
        explicit Server(uint16_t port);
        ~Server();

        Server(const Server&) = delete;
        Server operator=(const Server&) = delete;
        Server(Server&&) = delete;
        Server operator=(Server&&) = delete;

        void run();
        void stop();

        [[nodiscard]] bool isRunning() const { return m_running; }
        [[nodiscard]] uint64_t getCurrentTick() const { return m_tickCounter; }
}; 
