#pragma once

#include <unordered_map>
#include <memory>
#include <mutex>
#include <functional>
#include <queue>
#include <tuple>

#include "world/Chunk.hpp"
#include "../utils/ThreadPool.hpp"
#include "Entity.hpp"

class Player;

class World{
    private:
        std::vector<std::shared_ptr<Player>> m_players;
        mutable std::mutex m_worldMutex;

        std::unordered_map<uint64_t, std::shared_ptr<mc::Chunk>> m_chunks;
        mutable std::mutex m_chunkMutex;
        std::unique_ptr<ThreadPool> m_threadPool;

        int64_t m_worldAge{0};
        int64_t m_timeOfDay{0};
        double m_timeAccumulator{0.0};

        // Code per fisica dei fluidi
        std::queue<std::tuple<int, int, int>> m_fluidUpdates;
        std::mutex m_fluidMutex;

        // Entità (Item, ecc)
        std::vector<std::shared_ptr<Entity>> m_entities;
        mutable std::mutex m_entityMutex;
        int32_t m_nextEntityId{1000}; // Partiamo da 1000 per non collidere coi player

        static uint64_t getChunkKey(int32_t x, int32_t z) {
            return (static_cast<uint64_t>(static_cast<uint32_t>(x)) << 32) | static_cast<uint32_t>(z);
        }

    public:
        World();
        ~World();

        std::function<void(int, int, int, uint16_t)> onBlockChanged;
        std::function<void(std::shared_ptr<Entity>)> onEntitySpawned;
        std::function<void(std::shared_ptr<Entity>)> onEntityDestroyed;
        std::function<void(std::shared_ptr<Entity>)> onEntityMoved;
        std::function<void(int32_t, int32_t, uint16_t)> onItemCollected;
        std::function<void(int64_t, int64_t)> onTimeUpdated;

        void tick(double deltaTime);

        void addPlayer(std::shared_ptr<Player> player);
        void removePlayer(int playerId);

        [[nodiscard]] size_t getPlayerCount() const;

        std::shared_ptr<mc::Chunk> getChunk(int32_t x, int32_t z);
        void setBlock(int x, int y, int z, uint16_t blockStateId);
        uint16_t getBlock(int x, int y, int z);

        void scheduleFluidUpdate(int x, int y, int z);

        void spawnEntity(std::shared_ptr<Entity> entity);
        void spawnItem(double x, double y, double z, uint16_t itemId, double vx = 0, double vy = 0, double vz = 0);
        void spawnLightning(double x, double y, double z);
        std::shared_ptr<Entity> spawnMob(double x, double y, double z, int32_t typeId);
        [[nodiscard]] const std::vector<std::shared_ptr<Entity>>& getEntities() const { return m_entities; }

        void setTime(int64_t timeOfDay) { m_timeOfDay = timeOfDay; }
        [[nodiscard]] int64_t getTimeOfDay() const { return m_timeOfDay; }
        [[nodiscard]] int64_t getWorldAge() const { return m_worldAge; }
};
