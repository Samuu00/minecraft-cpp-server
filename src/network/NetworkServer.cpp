#include "NetworkServer.hpp"
#include "../utils/Logger.hpp"
#include "Socket.hpp"
#include "ClientConnection.hpp"
#include "../protocol/packets/SpawnEntityPacket.hpp"
#include "../protocol/packets/EntityMetadataPacket.hpp"
#include "../protocol/packets/CollectItemPacket.hpp"
#include "../protocol/packets/DestroyEntitiesPacket.hpp"
#include "../core/Entity.hpp"
#include "../protocol/packets/KeepAlivePacket.hpp"
#include "../protocol/packets/ChunkDataPacket.hpp"
#include "../protocol/packets/PlayerPositionPacket.hpp" 
#include "../core/World.hpp"
#include "../core/Player.hpp"

#include <algorithm>
#include <cstdint>
#include <memory>

NetworkServer::NetworkServer(uint16_t port, World* world) : m_port(port), m_world(world){
    if (!m_listenSocket.create()) {
        LOG_ERROR("Impossibile creare il socket di ascolto.");
        return;
    }


    m_listenSocket.setReuseAddr(true);
    m_listenSocket.setNonBlocking(true);

    if(!m_listenSocket.bind(m_port) || !m_listenSocket.listen()){
        LOG_ERROR("Inizializzazione del server sulla porta ", m_port, " fallita.");
        return;
    }

    m_active = true;
    LOG_INFO("NetworkServer avviato con successo sulla porta ", m_port);

    if (m_world) {
        m_world->onBlockChanged = [this](int x, int y, int z, uint16_t blockId) {
            // Invia Block Update a tutti i client in Play state
            int64_t location = (static_cast<int64_t>(x & 0x3FFFFFF) << 38) |
                               (static_cast<int64_t>(z & 0x3FFFFFF) << 12) |
                               (static_cast<int64_t>(y & 0xFFF));
            
            ByteBuffer blockUpdatePayload;
            blockUpdatePayload.writeLong(location);
            blockUpdatePayload.writeVarInt(blockId);
            ByteBuffer finalBlockUpdate;
            int32_t blockUpdateId = 0x09;
            int32_t blockUpdateLen = static_cast<int32_t>(ByteBuffer::getVarIntSize(blockUpdateId) + blockUpdatePayload.size());
            finalBlockUpdate.writeVarInt(blockUpdateLen);
            finalBlockUpdate.writeVarInt(blockUpdateId);
            finalBlockUpdate.writeBytes(blockUpdatePayload.vector());
            
            for (auto& client : m_clients) {
                if (client && client->isConnected() && client->getState() == ProtocolState::Play) {
                    client->sendRawBytes(finalBlockUpdate.vector().data(), finalBlockUpdate.vector().size());
                }
            }
        };

        m_world->onEntitySpawned = [this](std::shared_ptr<Entity> entity) {
            mc::SpawnEntityPacket spawnPacket(entity->getId(), entity->getType(), entity->getX(), entity->getY(), entity->getZ(), entity->getVx(), entity->getVy(), entity->getVz());
            for (auto& client : m_clients) {
                if (client && client->isConnected() && client->getState() == ProtocolState::Play) {
                    client->sendPacket(spawnPacket);
                    
                    if (entity->getType() == 55) { // Item
                        auto itemEntity = std::static_pointer_cast<ItemEntity>(entity);
                        mc::EntityMetadataPacket metadataPacket(entity->getId(), itemEntity->getItemId());
                        client->sendPacket(metadataPacket);
                    }
                }
            }
        };

        m_world->onEntityDestroyed = [this](std::shared_ptr<Entity> entity) {
            std::vector<int32_t> ids = { entity->getId() };
            mc::DestroyEntitiesPacket destroyPacket(ids);
            for (auto& client : m_clients) {
                if (client && client->isConnected() && client->getState() == ProtocolState::Play) {
                    client->sendPacket(destroyPacket);
                }
            }
        };

        m_world->onItemCollected = [this](int32_t collectedId, int32_t collectorId, uint16_t itemId) {
            mc::CollectItemPacket collectPacket(collectedId, collectorId, 1);
            for (auto& client : m_clients) {
                if (client && client->isConnected() && client->getState() == ProtocolState::Play) {
                    client->sendPacket(collectPacket);
                    
                    // Sincronizza l'inventario per il giocatore che ha raccolto l'oggetto
                    if (client->getPlayer() && client->getPlayer()->getId() == collectorId) {
                        int slot = client->getPlayer()->addInventoryItem(itemId, 1);
                        if (slot != -1) {
                            auto item = client->getPlayer()->getInventoryItem(slot);
                            
                            ByteBuffer slotPayload;
                            slotPayload.writeByte(0); // Window ID 0 (Player Inventory)
                            slotPayload.writeVarInt(0); // State ID
                            slotPayload.writeShort(slot); // Slot trovato
                            slotPayload.writeBoolean(true); // Present
                            slotPayload.writeVarInt(item.first); // Item ID
                            slotPayload.writeByte(item.second); // Count aggiornato
                            slotPayload.writeByte(0); // NBT (nessuno)
                            
                            ByteBuffer finalSlot;
                            int32_t slotId = 0x15; // Set Container Slot (1.20.4)
                            int32_t slotLen = static_cast<int32_t>(ByteBuffer::getVarIntSize(slotId) + slotPayload.size());
                            finalSlot.writeVarInt(slotLen);
                            finalSlot.writeVarInt(slotId);
                            finalSlot.writeBytes(slotPayload.vector());
                            
                            client->sendRawBytes(finalSlot.vector().data(), finalSlot.vector().size());
                        }
                    }
                }
            }
        };

        /*m_world->onEntityMoved = [this](std::shared_ptr<Entity> entity) {
            (void)entity;
            // Per ora non inviamo la posizione in continuo per gli oggetti per evitare spam
            // ma potremmo inviare Update Entity Position. Lo faremo se l'utente lo richiede.
        }; */

        m_world->onTimeUpdated = [this](int64_t worldAge, int64_t timeOfDay) {
            ByteBuffer timePayload;
            timePayload.writeLong(worldAge);
            timePayload.writeLong(timeOfDay);
            ByteBuffer finalTime;
            int32_t timeId = 0x62; // Update Time packet ID (1.20.4)
            int32_t timeLen = static_cast<int32_t>(ByteBuffer::getVarIntSize(timeId) + timePayload.size());
            finalTime.writeVarInt(timeLen);
            finalTime.writeVarInt(timeId);
            finalTime.writeBytes(timePayload.vector());
            
            for (auto& client : m_clients) {
                if (client && client->isConnected() && client->getState() == ProtocolState::Play) {
                    client->sendRawBytes(finalTime.vector().data(), finalTime.vector().size());
                }
            }
        };
    }
}

NetworkServer::~NetworkServer() { stop(); }

void NetworkServer::pollEvents() {
    if (!m_active) return;

    acceptNewConnections(); 

    for (auto it = m_clients.begin(); it != m_clients.end(); ) {
        auto& client = *it;

        if (client && client->isConnected()) {
            client->handleRead();
            client->flushOutgoingData();
        }

        if (!client || !client->isConnected()) {
            it = m_clients.erase(it);
        } else {
            ++it;
        }
    }
}

void NetworkServer::tick(uint64_t currentTick) {
    if (!m_active) return;

    for (auto& client : m_clients) {
        if (client && client->isConnected() && client->getState() == ProtocolState::Play) {
            
            auto player = client->getPlayer();
            if (player) {
                if (player->popHealthChanged()) {
                    ByteBuffer healthPayload;
                    healthPayload.writeFloat(player->getHealth());
                    healthPayload.writeVarInt(player->getFoodLevel());
                    healthPayload.writeFloat(5.0f); // Saturation
                    
                    ByteBuffer finalHealth;
                    int32_t healthId = 0x5B; // 1.20.4 Update Health
                    int32_t healthLen = static_cast<int32_t>(ByteBuffer::getVarIntSize(healthId) + healthPayload.size());
                    finalHealth.writeVarInt(healthLen);
                    finalHealth.writeVarInt(healthId);
                    finalHealth.writeBytes(healthPayload.vector());
                    client->sendRawBytes(finalHealth.vector().data(), finalHealth.vector().size());
                }

                if (player->popDamageEvent()) {
                    // Entity Event (0x1D) - Status 2 (Hurt Animation)
                    ByteBuffer eventPayload;
                    eventPayload.writeInt(player->getId());
                    eventPayload.writeByte(2); 
                    
                    ByteBuffer finalEvent;
                    int32_t eventId = 0x1D; // Entity Status
                    int32_t eventLen = static_cast<int32_t>(ByteBuffer::getVarIntSize(eventId) + eventPayload.size());
                    finalEvent.writeVarInt(eventLen);
                    finalEvent.writeVarInt(eventId);
                    finalEvent.writeBytes(eventPayload.vector());
                    client->sendRawBytes(finalEvent.vector().data(), finalEvent.vector().size());

                    // Damage Event (0x19)
                    ByteBuffer dmgPayload;
                    dmgPayload.writeVarInt(player->getId());
                    dmgPayload.writeVarInt(1); // Damage type ID (Generic)
                    dmgPayload.writeVarInt(0);
                    dmgPayload.writeVarInt(0);
                    dmgPayload.writeBoolean(false);
                    
                    ByteBuffer finalDmg;
                    int32_t dmgId = 0x19; 
                    int32_t dmgLen = static_cast<int32_t>(ByteBuffer::getVarIntSize(dmgId) + dmgPayload.size());
                    finalDmg.writeVarInt(dmgLen);
                    finalDmg.writeVarInt(dmgId);
                    finalDmg.writeBytes(dmgPayload.vector());
                    client->sendRawBytes(finalDmg.vector().data(), finalDmg.vector().size());
                }
            }

            // Invio di Keep Alive ogni ~15 secondi (300 ticks a 20 TPS)
            if (currentTick - client->getLastKeepAliveTick() >= 300) {
                if (client->isKeepAlivePending()) {
                    LOG_WARN("[", client->getIp(), "] Il client non ha risposto al Keep Alive in tempo, disconnessione (Timeout).");
                    client->disconnect();
                    continue;
                }

                int64_t keepAliveId = currentTick; // Usiamo il tick come ID per semplicità
                mc::ClientboundKeepAlivePacket keepAlivePacket(keepAliveId);
                client->sendPacket(keepAlivePacket);

                client->setLastKeepAliveTick(currentTick);
                client->setPendingKeepAliveId(keepAliveId);
                client->setKeepAlivePending(true);
            }
            
            // Gestione dei Chunk in caricamento (Asincrono) - fino a 16 per tick
            auto& pendingChunks = client->getPendingChunks();
            int chunksSentThisTick = 0;
            constexpr int MAX_CHUNKS_PER_TICK = 16;

            while (!pendingChunks.empty() && chunksSentThisTick < MAX_CHUNKS_PER_TICK) {
                auto frontChunk = pendingChunks.front();
                auto chunk = m_world->getChunk(frontChunk.first, frontChunk.second);
                
                if (!chunk->isReady()) break; // Aspetta che questo chunk sia pronto

                if (!client->isChunkBatchStarted()) {
                    ByteBuffer batchStart;
                    batchStart.writeVarInt(1); // Length
                    batchStart.writeVarInt(0x0D); // Chunk Batch Start ID
                    client->sendRawBytes(batchStart.vector().data(), batchStart.vector().size());
                    client->setChunkBatchStarted(true);
                }
                
                mc::ChunkDataPacket chunkPacket(chunk);
                ByteBuffer chunkPayload;
                chunkPacket.write(chunkPayload);
                ByteBuffer finalChunk;
                int32_t chunkId = chunkPacket.getId();
                int32_t chunkTotalLen = ByteBuffer::getVarIntSize(chunkId) + static_cast<int32_t>(chunkPayload.size());
                finalChunk.writeVarInt(chunkTotalLen);
                finalChunk.writeVarInt(chunkId);
                finalChunk.writeBytes(chunkPayload.vector());
                client->sendRawBytes(finalChunk.vector().data(), finalChunk.vector().size());
                
                client->incrementChunkBatchCount();
                pendingChunks.erase(pendingChunks.begin());
                chunksSentThisTick++;
            }

            if (pendingChunks.empty() && client->isChunkBatchStarted()) {
                // Chunk Batch Finished (0x0C)
                ByteBuffer batchFinishedPayload;
                batchFinishedPayload.writeVarInt(client->getChunkBatchCount());
                ByteBuffer batchFinished;
                batchFinished.writeVarInt(ByteBuffer::getVarIntSize(0x0C) + batchFinishedPayload.size());
                batchFinished.writeVarInt(0x0C);
                batchFinished.writeBytes(batchFinishedPayload.vector());
                client->sendRawBytes(batchFinished.vector().data(), batchFinished.vector().size());
                
                // Invia Player Position SOLO la prima volta (spawn iniziale)
                if (!client->isInitialChunksSent()) {
                    double spawnY = 120.0;
                    auto spawnChunk = m_world->getChunk(0, 0);
                    if (spawnChunk && spawnChunk->isReady()) {
                        for (int y = 120; y > -60; --y) {
                            if (spawnChunk->getBlock(0, y, 0) != 0) { // 0 = AIR
                                spawnY = y + 1.0;
                                break;
                            }
                        }
                    }
                    
                    if (client->getPlayer()) {
                        client->getPlayer()->setPosition(0.5, spawnY, 0.5);
                        // Imposta il client in modalità Creativa internamente per allinearsi col pacchetto JoinGame
                        client->setCreative(true);
                    }

                    mc::PlayerPositionPacket posPacket(0.5, spawnY, 0.5);
                    ByteBuffer posPayload;
                    posPacket.write(posPayload);
                    ByteBuffer finalPosPacket;
                    int32_t posId = posPacket.getId();
                    int32_t posLen = static_cast<int32_t>(ByteBuffer::getVarIntSize(posId) + posPayload.size());
                    finalPosPacket.writeVarInt(posLen);
                    finalPosPacket.writeVarInt(posId);
                    finalPosPacket.writeBytes(posPayload.vector());
                    client->sendRawBytes(finalPosPacket.vector().data(), finalPosPacket.vector().size());
                    client->setInitialChunksSent(true);
                    LOG_INFO("[", client->getIp(), "] Spawn iniziale inviato con ", client->getChunkBatchCount(), " chunks.");
                }
                
                client->setChunkBatchStarted(false);
                client->resetChunkBatchCount();
            }
        }
    }
}

void NetworkServer::acceptNewConnections(){
    std::string clientIp;
    uint16_t clientPort = 0;

    SOCKET clientSocketHandle = m_listenSocket.accept(clientIp, clientPort);
    while(clientSocketHandle != INVALID_SOCKET){
        Socket clientSocket(clientSocketHandle);
        auto client = std::make_shared<ClientConnection>(std::move(clientSocket), clientIp, clientPort, m_world);
        m_clients.emplace_back(client);

        clientSocketHandle = m_listenSocket.accept(clientIp, clientPort);
    }
}

void NetworkServer::processClientIO(){
    for(auto& client : m_clients){
        if(!client || !client->isConnected()) continue;

        client->readIncomingData();
        client->flushOutgoingData();
    }
}

void NetworkServer::cleanupDisconnectedClients(){
    auto it = std::remove_if(m_clients.begin(), m_clients.end(),
            [](const std::shared_ptr<ClientConnection>& client){
                return !client || !client->isConnected();
            });

    if(it != m_clients.end()) m_clients.erase(it, m_clients.end());
}

void NetworkServer::stop(){
    if(!m_active) return;

    m_active = false;
    LOG_INFO("Disconnessione di tutti i client e arresto del NetworkServer...");

    for(auto& client : m_clients){
        if(client) client->disconnect();
    }
    
    m_clients.clear();
    m_listenSocket.close();
} 

