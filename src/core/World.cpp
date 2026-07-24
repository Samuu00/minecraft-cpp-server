#include "World.hpp"
#include "Player.hpp"
#include "../utils/Logger.hpp"
#include <algorithm>
#include <mutex>

World::World(){
    m_threadPool = std::make_unique<ThreadPool>(4); // 4 threads per la generazione
    LOG_INFO("World engine inizializzato con ThreadPool a 4 thread");
}

World::~World() = default;

void World::tick(double deltaTime){
    std::lock_guard<std::mutex> lock(m_worldMutex);

    for(auto& player : m_players){
        if(player) player->tick(deltaTime);
    }

    // Processa le entità (item ecc)
    std::vector<std::shared_ptr<Entity>> removedEntities;
    {
        std::lock_guard<std::mutex> entityLock(m_entityMutex);
        for (auto it = m_entities.begin(); it != m_entities.end(); ) {
            auto& entity = *it;
            if (entity) {
                entity->tick(deltaTime);

                // Controllo raccolta da parte dei player (raggio 1.5 blocchi)
                if (entity->getType() == 55 && !entity->isMarkedForRemoval()) { // 55 = Item
                    for (auto& player : m_players) {
                        if (player) {
                            double dx = player->getPosition().x - entity->getX();
                            double dy = player->getPosition().y - entity->getY();
                            double dz = player->getPosition().z - entity->getZ();
                            if (dx*dx + dy*dy + dz*dz < 2.25) { // 1.5^2
                                if (onItemCollected) {
                                    onItemCollected(entity->getId(), player->getId());
                                }
                                entity->markForRemoval();
                                break;
                            }
                        }
                    }
                }

                if (entity->isMarkedForRemoval()) {
                    removedEntities.push_back(entity);
                    it = m_entities.erase(it);
                } else {
                    if (onEntityMoved) {
                        onEntityMoved(entity);
                    }
                    ++it;
                }
            } else {
                it = m_entities.erase(it);
            }
        }
    }

    for (auto& e : removedEntities) {
        if (onEntityDestroyed) {
            onEntityDestroyed(e);
        }
    }

    // Processa gli update dei fluidi (max 500 per tick per evitare lag)
    std::unique_lock<std::mutex> fluidLock(m_fluidMutex);
    int updatesToProcess = std::min(500, static_cast<int>(m_fluidUpdates.size()));
    std::vector<std::tuple<int, int, int>> currentUpdates;
    for (int i = 0; i < updatesToProcess; ++i) {
        currentUpdates.push_back(m_fluidUpdates.front());
        m_fluidUpdates.pop();
    }
    fluidLock.unlock();

    for (const auto& update : currentUpdates) {
        int x = std::get<0>(update);
        int y = std::get<1>(update);
        int z = std::get<2>(update);
        
        uint16_t block = getBlock(x, y, z);
        if (block == 80) { // WATER
            // Propaga l'acqua ai blocchi adiacenti se sono AIR (0)
            int adjacent[5][3] = { {x, y-1, z}, {x+1, y, z}, {x-1, y, z}, {x, y, z+1}, {x, y, z-1} };
            for (auto& pos : adjacent) {
                if (getBlock(pos[0], pos[1], pos[2]) == 0) {
                    setBlock(pos[0], pos[1], pos[2], 80); // Set to WATER
                }
            }
        } else if (block == 0) { // Se è diventato AIR, controlliamo se l'acqua deve cadere qui
            int adjacent[5][3] = { {x, y+1, z}, {x+1, y, z}, {x-1, y, z}, {x, y, z+1}, {x, y, z-1} };
            for (auto& pos : adjacent) {
                if (getBlock(pos[0], pos[1], pos[2]) == 80) {
                    setBlock(x, y, z, 80); // Set to WATER
                    break;
                }
            }
        }
    }
}

void World::addPlayer(std::shared_ptr<Player> player){
    std::lock_guard<std::mutex> lock(m_worldMutex);
    m_players.emplace_back(player);
    LOG_INFO("Giocatore aggiunto al mondo. Totale online: ", m_players.size());
}

void World::removePlayer(int playerId){
    std::lock_guard<std::mutex> lock(m_worldMutex);
    auto it = std::remove_if(m_players.begin(), m_players.end(),
            [playerId](const std::shared_ptr<Player>& p){
                return p && p->getId() == playerId;
            });

    if(it != m_players.end()){
        m_players.erase(it, m_players.end());
        LOG_INFO("Giocatore rimossa dal mondo. Totale online: ", m_players.size());
    }
}

size_t World::getPlayerCount() const {
    std::lock_guard<std::mutex> lock(m_worldMutex);
    return m_players.size();
}

std::shared_ptr<mc::Chunk> World::getChunk(int32_t x, int32_t z) {
    uint64_t key = getChunkKey(x, z);
    
    std::lock_guard<std::mutex> lock(m_chunkMutex);
    auto it = m_chunks.find(key);
    if(it != m_chunks.end()){
        return it->second;
    }

    // Se non esiste, lo generiamo
    auto newChunk = std::make_shared<mc::Chunk>(x, z);
    m_chunks[key] = newChunk;
    
    // Generazione asincrona
    m_threadPool->enqueue([newChunk]() {
        newChunk->generateTerrain();
    });

    return newChunk;
}

void World::setBlock(int x, int y, int z, uint16_t blockStateId) {
    int32_t chunkX = x >> 4;
    int32_t chunkZ = z >> 4;
    int localX = ((x % 16) + 16) % 16;
    int localZ = ((z % 16) + 16) % 16;

    auto chunk = getChunk(chunkX, chunkZ);
    if (!chunk || !chunk->isReady()) return;

    chunk->setBlock(localX, y, localZ, blockStateId);

    if (onBlockChanged) {
        onBlockChanged(x, y, z, blockStateId);
    }

    // Rischedula controlli fluidi nei blocchi adiacenti
    scheduleFluidUpdate(x, y, z);
    scheduleFluidUpdate(x+1, y, z);
    scheduleFluidUpdate(x-1, y, z);
    scheduleFluidUpdate(x, y+1, z);
    scheduleFluidUpdate(x, y-1, z);
    scheduleFluidUpdate(x, y, z+1);
    scheduleFluidUpdate(x, y, z-1);
}

uint16_t World::getBlock(int x, int y, int z) {
    int32_t chunkX = x >> 4;
    int32_t chunkZ = z >> 4;
    int localX = ((x % 16) + 16) % 16;
    int localZ = ((z % 16) + 16) % 16;

    uint64_t key = getChunkKey(chunkX, chunkZ);
    std::lock_guard<std::mutex> lock(m_chunkMutex);
    auto it = m_chunks.find(key);
    if(it != m_chunks.end() && it->second->isReady()){
        return it->second->getBlock(localX, y, localZ);
    }
    return 0; // Se il chunk non esiste ancora, ritorna AIR
}

void World::scheduleFluidUpdate(int x, int y, int z) {
    std::lock_guard<std::mutex> lock(m_fluidMutex);
    m_fluidUpdates.push({x, y, z});
}

std::shared_ptr<Entity> World::spawnItem(double x, double y, double z, uint16_t itemId) {
    std::lock_guard<std::mutex> lock(m_entityMutex);
    int32_t id = m_nextEntityId++;
    auto item = std::make_shared<ItemEntity>(id, this, x, y, z, itemId);
    
    // Un po' di spinta verso l'alto e casuale
    double vx = (rand() % 100 - 50) / 1000.0;
    double vy = 0.2;
    double vz = (rand() % 100 - 50) / 1000.0;
    item->setVelocity(vx, vy, vz);

    m_entities.push_back(item);

    if (onEntitySpawned) {
        onEntitySpawned(item);
    }

    return item;
}

