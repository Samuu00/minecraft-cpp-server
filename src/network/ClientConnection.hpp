#pragma once

#include "Socket.hpp"
#include "protocol/Packet.hpp"
#include <vector>
#include <string>
#include <cstdint>
#include <set>
#include <utility>

enum class ProtocolState{
    Handshaking = 0,
    Status = 1,
    Login = 2,
    Configuration = 3,
    Play = 4
};

class Player;

class ClientConnection{
    private:
        Socket m_socket;
        std::string m_ip;
        uint16_t m_port;
        class World* m_world{nullptr};

        ProtocolState m_state{ProtocolState::Handshaking};
        bool m_connected{true};

        std::vector<uint8_t> m_rxBuffer;
        std::vector<uint8_t> m_txBuffer;

        uint64_t m_lastKeepAliveTick{0};
        int64_t m_pendingKeepAliveId{0};
        bool m_keepAlivePending{false};

        std::vector<std::pair<int32_t, int32_t>> m_pendingChunks;
        bool m_chunkBatchStarted{false};
        int32_t m_chunkBatchCount{0};

        // Tracking posizione giocatore per caricamento chunk dinamico
        int32_t m_currentChunkX{0};
        int32_t m_currentChunkZ{0};
        double m_playerX{0.0};
        double m_playerY{120.0};
        double m_playerZ{0.0};
        std::set<std::pair<int32_t, int32_t>> m_loadedChunks;
        bool m_initialChunksSent{false};
        bool m_isCreative{false};

        std::shared_ptr<Player> m_player;
        double m_highestY{-1000.0};

    public:
        ClientConnection(Socket socket, std::string ip, uint16_t port, class World* world = nullptr);
        ~ClientConnection();

        ClientConnection(const ClientConnection&) = delete;
        ClientConnection& operator=(const ClientConnection&) = delete;

        bool readIncomingData();
        bool flushOutgoingData();

        void sendRawBytes(const uint8_t* data, size_t length);
        void sendRawBytes(std::vector<uint8_t>& data);

        [[nodiscard]] ProtocolState getState() const { return m_state; }
        void setState(ProtocolState state) { m_state = state; }

        void sendPacket(const mc::Packet& packet);
        void handleRead();
        void sendBuffer(const std::vector<uint8_t>& data);

        [[nodiscard]] bool isConnected() const { return m_connected; }
        void disconnect();

        [[nodiscard]] const std::string& getIp() const { return m_ip; } 
        [[nodiscard]] uint16_t getPort() const { return m_port; }
        
        [[nodiscard]] std::vector<uint8_t>& getRxBuffer() { return m_rxBuffer; }

        [[nodiscard]] uint64_t getLastKeepAliveTick() const { return m_lastKeepAliveTick; }
        void setLastKeepAliveTick(uint64_t tick) { m_lastKeepAliveTick = tick; }

        [[nodiscard]] int64_t getPendingKeepAliveId() const { return m_pendingKeepAliveId; }
        void setPendingKeepAliveId(int64_t id) { m_pendingKeepAliveId = id; }

        [[nodiscard]] bool isKeepAlivePending() const { return m_keepAlivePending; }
        void setKeepAlivePending(bool pending) { m_keepAlivePending = pending; }
        
        [[nodiscard]] std::vector<std::pair<int32_t, int32_t>>& getPendingChunks() { return m_pendingChunks; }
        [[nodiscard]] bool isChunkBatchStarted() const { return m_chunkBatchStarted; }
        void setChunkBatchStarted(bool started) { m_chunkBatchStarted = started; }
        [[nodiscard]] int32_t getChunkBatchCount() const { return m_chunkBatchCount; }
        void incrementChunkBatchCount() { m_chunkBatchCount++; }
        void resetChunkBatchCount() { m_chunkBatchCount = 0; }

        // Posizione e chunk tracking
        void setPlayerPosition(double x, double y, double z) { m_playerX = x; m_playerY = y; m_playerZ = z; }
        [[nodiscard]] double getPlayerX() const { return m_playerX; }
        [[nodiscard]] double getPlayerY() const { return m_playerY; }
        [[nodiscard]] double getPlayerZ() const { return m_playerZ; }

        [[nodiscard]] int32_t getCurrentChunkX() const { return m_currentChunkX; }
        [[nodiscard]] int32_t getCurrentChunkZ() const { return m_currentChunkZ; }
        void setCurrentChunk(int32_t cx, int32_t cz) { m_currentChunkX = cx; m_currentChunkZ = cz; }

        [[nodiscard]] std::set<std::pair<int32_t, int32_t>>& getLoadedChunks() { return m_loadedChunks; }
        
        [[nodiscard]] bool isInitialChunksSent() const { return m_initialChunksSent; }
        void setInitialChunksSent(bool sent) { m_initialChunksSent = sent; }
        
        [[nodiscard]] bool isCreative() const { return m_isCreative; }
        void setCreative(bool creative) { m_isCreative = creative; }

        [[nodiscard]] std::shared_ptr<Player> getPlayer() const { return m_player; }
        void setPlayer(std::shared_ptr<Player> p) { m_player = p; }

        double getHighestY() const { return m_highestY; }
        void setHighestY(double y) { m_highestY = y; }

        // Accoda nuovi chunk intorno a una posizione (view distance = raggio)
        void queueChunksAround(int32_t cx, int32_t cz, int radius) {
            for (int dx = -radius; dx <= radius; ++dx) {
                for (int dz = -radius; dz <= radius; ++dz) {
                    // Spirale dal centro: prima carica i chunk più vicini
                    if (dx * dx + dz * dz > (radius + 1) * (radius + 1)) continue;
                    std::pair<int32_t, int32_t> key = {cx + dx, cz + dz};
                    if (m_loadedChunks.find(key) == m_loadedChunks.end()) {
                        m_pendingChunks.push_back(key);
                        m_loadedChunks.insert(key);
                    }
                }
            }
        }

        // Scarica chunk troppo lontani
        void unloadDistantChunks(int32_t cx, int32_t cz, int maxDist) {
            auto it = m_loadedChunks.begin();
            while (it != m_loadedChunks.end()) {
                int dx = it->first - cx;
                int dz = it->second - cz;
                if (dx * dx + dz * dz > (maxDist + 2) * (maxDist + 2)) {
                    it = m_loadedChunks.erase(it);
                } else {
                    ++it;
                }
            }
        }

        [[nodiscard]] class World* getWorld() const { return m_world; }
};
