#include "protocol/PacketHandler.hpp"
#include "network/ClientConnection.hpp"
#include "protocol/ByteBuffer.hpp"
#include "protocol/packets/HandshakePacket.hpp"
#include "protocol/packets/JoinGamePacket.hpp"
#include "protocol/packets/PlayerPositionPacket.hpp"
#include "protocol/packets/StatusPackets.hpp"
#include "protocol/packets/RegistryData.hpp"
#include "protocol/packets/GameEventPacket.hpp"
#include "protocol/packets/AcknowledgeBlockChangePacket.hpp"
#include "protocol/packets/CommandsPacket.hpp"
#include "protocol/packets/PlayerAbilitiesPacket.hpp"
#include "protocol/packets/UpdateTagsPacket.hpp"
#include "core/World.hpp"
#include "utils/Logger.hpp"

#include <cstdint>
#include <nlohmann/json.hpp> 
#include <vector>
#include <sstream>
#include <cmath>
#include <windows.h>
#include <wingdi.h>

namespace mc {

    bool PacketHandler::processIncomingData(ClientConnection& client, std::vector<uint8_t>& receiveBuffer) {
        ByteBuffer readBuffer(receiveBuffer);

        while (readBuffer.readableBytes() > 0) {
            size_t startReadPos = readBuffer.getReadPos();

            int32_t packetLength = 0;
            try {
                packetLength = readBuffer.readVarInt();
            } catch (const std::out_of_range&) {
                readBuffer.setReadPos(startReadPos);
                break;
            }

            if (readBuffer.readableBytes() < static_cast<size_t>(packetLength)) {
                readBuffer.setReadPos(startReadPos);
                break;
            }

            size_t packetDataStart = readBuffer.getReadPos();
            int32_t packetId = readBuffer.readVarInt();

            size_t idSize = readBuffer.getReadPos() - packetDataStart;
            size_t payloadSize = static_cast<size_t>(packetLength) - idSize;

            std::vector<uint8_t> payloadBytes = readBuffer.readBytes(payloadSize);
            ByteBuffer payloadBuffer(std::move(payloadBytes));

            try {
                handlePacket(client, packetId, payloadBuffer);
            } catch (const std::exception& e) {
                LOG_ERROR("Errore durante la gestione del pacchetto 0x", std::hex, packetId, ": ", e.what());
                return false;
            }
        }

        size_t unreadBytes = readBuffer.readableBytes();
        if (unreadBytes > 0) {
            std::memmove(receiveBuffer.data(), receiveBuffer.data() + readBuffer.getReadPos(), unreadBytes);
        }
        receiveBuffer.resize(unreadBytes);

        return true;
    }

    void PacketHandler::handlePacket(ClientConnection& client, int32_t packetId, ByteBuffer& payload) {
        ProtocolState state = client.getState();

        // -------------------------------------------------------------------------
        // Stato: Handshaking
        // -------------------------------------------------------------------------
        if (state == ProtocolState::Handshaking) {
            if (packetId == 0x00) { // Handshake Packet
                HandshakePacket handshake;
                handshake.read(payload);

                LOG_INFO("Handshake ricevuto! Protocollo: ", handshake.protocolVersion,
                        ", Host: ", handshake.serverAddress, ":", handshake.serverPort,
                        ", NextState: ", handshake.nextState);

                if (handshake.nextState == 1) {
                    client.setState(ProtocolState::Status);
                } else if (handshake.nextState == 2) {
                    client.setState(ProtocolState::Login);
                } else {
                    LOG_WARN("Stato richiesto nell'Handshake non valido: ", handshake.nextState);
                }
            }
            return;
        }

        // -------------------------------------------------------------------------
        // Stato: Status (Server List Ping & MOTD)
        // -------------------------------------------------------------------------
        if (state == ProtocolState::Status) {
            if (packetId == 0x00) { // Status Request
                StatusRequestPacket request;
                request.read(payload);

                nlohmann::json responseJson = {
                    {"version", {
                        {"name", "1.20.4 C++ Server"},
                        {"protocol", 765}
                    }},
                    {"players", {
                        {"max", 20},
                        {"online", 0},
                        {"sample", nlohmann::json::array()}
                    }},
                    {"description", {
                    {"text", "§aCustom C++20 Minecraft Server!"}
                    }}
                };

                StatusResponsePacket response(responseJson.dump());
                ByteBuffer responseBuf = PacketHandler::serializePacket(response);
                client.sendRawBytes(responseBuf.data(), responseBuf.size());
                
            } else if (packetId == 0x01) { // Ping Request
                PingPacket ping;
                ping.read(payload);

                // stesso payload del timestamp (Pong)
                PingPacket pong(ping.payload);
                ByteBuffer pongBuf = PacketHandler::serializePacket(pong);
                client.sendRawBytes(pongBuf.data(), pongBuf.size());
            }
            return;
        }
        
        // -------------------------------------------------------------------------
        // Stato: Login
        // -------------------------------------------------------------------------
        if(state == ProtocolState::Login){
            if(packetId == 0x00){
                std::string username = payload.readString();
                LOG_INFO("[", client.getIp(), "] Tentativo di login dell'utente: ", username);

                std::vector<uint8_t> uuidBytes(16, 0x00);
                ByteBuffer loginSuccessPayload;
                loginSuccessPayload.writeBytes(uuidBytes);
                loginSuccessPayload.writeString(username);
                loginSuccessPayload.writeVarInt(0);

                ByteBuffer finalLoginSuccess;
                int32_t successId = 0x02;
                int32_t successLen = static_cast<int32_t>(ByteBuffer::getVarIntSize(successId) + loginSuccessPayload.size());

                finalLoginSuccess.writeVarInt(successLen);
                finalLoginSuccess.writeVarInt(successId);
                finalLoginSuccess.writeBytes(loginSuccessPayload.vector());

                client.sendRawBytes(finalLoginSuccess.vector().data(), finalLoginSuccess.vector().size());
                LOG_INFO("[", client.getIp(), "] Inviato Login Success. In attesa di Login Acknowledged...");
            }
            else if (packetId == 0x03) { // Login Acknowledged
                client.setState(ProtocolState::Configuration);
                LOG_INFO("[", client.getIp(), "] Stato transizionato a CONFIGURATION.");

                // 1) Invia Registry Data (0x05)
                ByteBuffer registryPayload;
                registryPayload.writeBytes(REGISTRY_DATA_NBT);
                
                ByteBuffer finalRegistry;
                int32_t registryId = 0x05;
                int32_t registryLen = static_cast<int32_t>(ByteBuffer::getVarIntSize(registryId) + registryPayload.size());
                finalRegistry.writeVarInt(registryLen);
                finalRegistry.writeVarInt(registryId);
                finalRegistry.writeBytes(registryPayload.vector());
                client.sendRawBytes(finalRegistry.vector().data(), finalRegistry.vector().size());
                LOG_INFO("[", client.getIp(), "] Inviato Registry Data (0x05).");

                // 2) Invia Finish Configuration (0x02)
                ByteBuffer finishConfig;
                finishConfig.writeVarInt(1); // Length (1 byte for ID)
                finishConfig.writeVarInt(0x02); // Packet ID
                client.sendRawBytes(finishConfig.vector().data(), finishConfig.vector().size());
                LOG_INFO("[", client.getIp(), "] Inviato Finish Configuration.");
            }
            else {
                LOG_INFO("[", client.getIp(), "] Ignorato pacchetto in stato Login: 0x", std::hex, packetId);
            }
            return;
        }

        // -------------------------------------------------------------------------
        // Stato: Configuration
        // -------------------------------------------------------------------------
        if(state == ProtocolState::Configuration){
            if(packetId == 0x02){ // Finish Configuration (Serverbound)
                client.setState(ProtocolState::Play);
                LOG_INFO("[", client.getIp(), "] Stato transizionato a PLAY.");

                JoinGamePacket joinPacket(1);
                ByteBuffer joinPayload;
                joinPacket.write(joinPayload); 

                ByteBuffer finalJoin;
                int32_t joinId = joinPacket.getId(); // 0x29
                int32_t totalLen = ByteBuffer::getVarIntSize(joinId) + static_cast<int32_t>(joinPayload.size());

                finalJoin.writeVarInt(totalLen);
                finalJoin.writeVarInt(joinId);
                finalJoin.writeBytes(joinPayload.vector());

                client.sendRawBytes(finalJoin.vector().data(), finalJoin.vector().size());

                // Update Tags (0x74) per far funzionare l'acqua
                UpdateTagsPacket tagsPacket;
                ByteBuffer tagsPayload;
                tagsPacket.write(tagsPayload);
                ByteBuffer finalTags;
                int32_t tagsId = tagsPacket.getId();
                int32_t tagsLen = static_cast<int32_t>(ByteBuffer::getVarIntSize(tagsId) + tagsPayload.size());
                finalTags.writeVarInt(tagsLen);
                finalTags.writeVarInt(tagsId);
                finalTags.writeBytes(tagsPayload.vector());
                client.sendRawBytes(finalTags.vector().data(), finalTags.vector().size());

                // Update View Position (0x52)
                ByteBuffer viewPosPayload;
                viewPosPayload.writeVarInt(0); // Chunk X
                viewPosPayload.writeVarInt(0); // Chunk Z
                
                ByteBuffer finalViewPos;
                int32_t viewPosId = 0x52;
                int32_t viewPosLen = static_cast<int32_t>(ByteBuffer::getVarIntSize(viewPosId) + viewPosPayload.size());
                finalViewPos.writeVarInt(viewPosLen);
                finalViewPos.writeVarInt(viewPosId);
                finalViewPos.writeBytes(viewPosPayload.vector());
                client.sendRawBytes(finalViewPos.vector().data(), finalViewPos.vector().size());

                // Commands (0x12)
                CommandsPacket commandsPacket;
                ByteBuffer commandsPayload;
                commandsPacket.write(commandsPayload);
                
                ByteBuffer finalCommands;
                int32_t commandsId = commandsPacket.getId();
                int32_t commandsLen = static_cast<int32_t>(ByteBuffer::getVarIntSize(commandsId) + commandsPayload.size());
                finalCommands.writeVarInt(commandsLen);
                finalCommands.writeVarInt(commandsId);
                finalCommands.writeBytes(commandsPayload.vector());
                client.sendRawBytes(finalCommands.vector().data(), finalCommands.vector().size());

                // Default spawn position (0x54)
                ByteBuffer spawnPosPayload;
                int64_t x = 0; int64_t y = 120; int64_t z = 0;
                int64_t spawnPos = ((x & 0x3FFFFFF) << 38) | ((z & 0x3FFFFFF) << 12) | (y & 0xFFF);
                spawnPosPayload.writeLong(spawnPos);
                spawnPosPayload.writeFloat(0.0f); // Angle
                
                ByteBuffer finalSpawnPos;
                int32_t spawnPosId = 0x54;
                int32_t spawnPosLen = static_cast<int32_t>(ByteBuffer::getVarIntSize(spawnPosId) + spawnPosPayload.size());
                finalSpawnPos.writeVarInt(spawnPosLen);
                finalSpawnPos.writeVarInt(spawnPosId);
                finalSpawnPos.writeBytes(spawnPosPayload.vector());
                client.sendRawBytes(finalSpawnPos.vector().data(), finalSpawnPos.vector().size());

                // Player Abilities (0x36) - Force Survival
                mc::PlayerAbilitiesPacket abilitiesPacket(0x00, 0.05f, 0.1f); // No flags = survival
                ByteBuffer abilitiesPayload;
                abilitiesPacket.write(abilitiesPayload);
                ByteBuffer finalAbilities;
                int32_t abilitiesId = abilitiesPacket.getId();
                int32_t abilitiesLen = static_cast<int32_t>(ByteBuffer::getVarIntSize(abilitiesId) + abilitiesPayload.size());
                finalAbilities.writeVarInt(abilitiesLen);
                finalAbilities.writeVarInt(abilitiesId);
                finalAbilities.writeBytes(abilitiesPayload.vector());
                client.sendRawBytes(finalAbilities.vector().data(), finalAbilities.vector().size());

                // Synchronize Player Position (0x3E)
                mc::PlayerPositionPacket posPacket(0.0, 120.0, 0.0, 0.0f, 0.0f);
                ByteBuffer posPayload;
                posPacket.write(posPayload);
                ByteBuffer finalPos;
                int32_t posId = posPacket.getId();
                int32_t posLen = static_cast<int32_t>(ByteBuffer::getVarIntSize(posId) + posPayload.size());
                finalPos.writeVarInt(posLen);
                finalPos.writeVarInt(posId);
                finalPos.writeBytes(posPayload.vector());
                client.sendRawBytes(finalPos.vector().data(), finalPos.vector().size());

                // Accodiamo i chunk intorno allo spawn usando il sistema dinamico
                client.setCurrentChunk(0, 0);
                client.queueChunksAround(0, 0, 8);
                LOG_INFO("[", client.getIp(), "] Accodati ", client.getPendingChunks().size(), " chunk per la generazione asincrona.");

                LOG_INFO("[", client.getIp(), "] Inviato JoinGame (0x29), Position, ViewPos e SpawnPos.");
            }
            else {
                // Ignore other configuration packets (like Client Information)
                LOG_INFO("[", client.getIp(), "] Ignorato pacchetto in stato Configuration: 0x", std::hex, packetId);
            }
            return;
        }

        if (state == ProtocolState::Play) {
            if (packetId == 0x00) { // Teleport Confirm
                int32_t teleportId = payload.readVarInt();
                LOG_INFO("[", client.getIp(), "] Teleport Confirm ricevuto! ID: ", teleportId);
                return;
            }
            if (packetId == 0x15) { // Serverbound Keep Alive (1.20.4)
                int64_t keepAliveId = payload.readLong();
                if(client.isKeepAlivePending() && keepAliveId == client.getPendingKeepAliveId()) {
                    client.setKeepAlivePending(false);
                }
                return;
            }
            if (packetId == 0x03 || packetId == 0x01) return; // Ignores client settings etc

            // --- Player Position packets ---
            if (packetId == 0x17) { // Set Player Position
                double px = payload.readDouble();
                double py = payload.readDouble();
                double pz = payload.readDouble();
                bool onGround = payload.readBoolean(); (void)onGround;
                client.setPlayerPosition(px, py, pz);
                
                int32_t newCX = static_cast<int32_t>(std::floor(px)) >> 4;
                int32_t newCZ = static_cast<int32_t>(std::floor(pz)) >> 4;
                if (newCX != client.getCurrentChunkX() || newCZ != client.getCurrentChunkZ()) {
                    client.setCurrentChunk(newCX, newCZ);
                    // Aggiorna View Position per il client
                    ByteBuffer viewPosPayload2;
                    viewPosPayload2.writeVarInt(newCX);
                    viewPosPayload2.writeVarInt(newCZ);
                    ByteBuffer finalViewPos2;
                    int32_t vpId = 0x52;
                    int32_t vpLen = static_cast<int32_t>(ByteBuffer::getVarIntSize(vpId) + viewPosPayload2.size());
                    finalViewPos2.writeVarInt(vpLen);
                    finalViewPos2.writeVarInt(vpId);
                    finalViewPos2.writeBytes(viewPosPayload2.vector());
                    client.sendRawBytes(finalViewPos2.vector().data(), finalViewPos2.vector().size());
                    // Carica nuovi chunk intorno alla nuova posizione
                    client.queueChunksAround(newCX, newCZ, 8);
                    client.unloadDistantChunks(newCX, newCZ, 10);
                }
                return;
            }
            if (packetId == 0x18) { // Set Player Position And Rotation
                double px = payload.readDouble();
                double py = payload.readDouble();
                double pz = payload.readDouble();
                float yaw = payload.readFloat(); (void)yaw;
                float pitch = payload.readFloat(); (void)pitch;
                bool onGround = payload.readBoolean(); (void)onGround;
                client.setPlayerPosition(px, py, pz);
                
                int32_t newCX = static_cast<int32_t>(std::floor(px)) >> 4;
                int32_t newCZ = static_cast<int32_t>(std::floor(pz)) >> 4;
                if (newCX != client.getCurrentChunkX() || newCZ != client.getCurrentChunkZ()) {
                    client.setCurrentChunk(newCX, newCZ);
                    ByteBuffer viewPosPayload2;
                    viewPosPayload2.writeVarInt(newCX);
                    viewPosPayload2.writeVarInt(newCZ);
                    ByteBuffer finalViewPos2;
                    int32_t vpId = 0x52;
                    int32_t vpLen = static_cast<int32_t>(ByteBuffer::getVarIntSize(vpId) + viewPosPayload2.size());
                    finalViewPos2.writeVarInt(vpLen);
                    finalViewPos2.writeVarInt(vpId);
                    finalViewPos2.writeBytes(viewPosPayload2.vector());
                    client.sendRawBytes(finalViewPos2.vector().data(), finalViewPos2.vector().size());
                    client.queueChunksAround(newCX, newCZ, 8);
                    client.unloadDistantChunks(newCX, newCZ, 10);
                }
                return;
            }
            if (packetId == 0x19) { // Set Player Rotation
                return; // Solo rotazione, non serve aggiornare chunk
            }
            if (packetId == 0x1A) { // Set Player On Ground
                return;
            }

            if (packetId == 0x04) { // Serverbound Chat Command
                std::string command = payload.readString();
                LOG_INFO("[", client.getIp(), "] Comando eseguito: /", command);
                
                if (command.rfind("gamemode creative", 0) == 0 || command.rfind("gamemode 1", 0) == 0) {
                    GameEventPacket gmPacket(3, 1.0f); // 3 = Change Game Mode, 1 = Creative
                    client.sendPacket(gmPacket);
                    LOG_INFO("Gamemode impostato a Creative per ", client.getIp());
                } else if (command.rfind("gamemode survival", 0) == 0 || command.rfind("gamemode 0", 0) == 0) {
                    GameEventPacket gmPacket(3, 0.0f); // 3 = Change Game Mode, 0 = Survival
                    client.sendPacket(gmPacket);
                    LOG_INFO("Gamemode impostato a Survival per ", client.getIp());
                } else if (command.rfind("tp ", 0) == 0) {
                    std::istringstream iss(command.substr(3));
                    double x, y, z;
                    if (iss >> x >> y >> z) {
                        PlayerPositionPacket posPacket(x, y, z);
                        ByteBuffer posPayload;
                        posPacket.write(posPayload);
                        ByteBuffer finalPos;
                        int32_t posId = posPacket.getId();
                        int32_t posLen = static_cast<int32_t>(ByteBuffer::getVarIntSize(posId) + posPayload.size());
                        finalPos.writeVarInt(posLen);
                        finalPos.writeVarInt(posId);
                        finalPos.writeBytes(posPayload.vector());
                        client.sendRawBytes(finalPos.vector().data(), finalPos.vector().size());
                        LOG_INFO("Teletrasportato a ", x, " ", y, " ", z, " per ", client.getIp());
                    }
                } else if (command.rfind("time ", 0) == 0) {
                    std::string arg = command.substr(5);
                    int64_t timeValue = 0;
                    if (arg == "day") timeValue = 1000;
                    else if (arg == "night") timeValue = 13000;
                    
                    ByteBuffer timePayload;
                    timePayload.writeLong(0); // World Age
                    timePayload.writeLong(timeValue); // Time of day
                    
                    ByteBuffer finalTime;
                    int32_t timeId = 0x62; // Update Time packet ID (1.20.4)
                    int32_t timeLen = static_cast<int32_t>(ByteBuffer::getVarIntSize(timeId) + timePayload.size());
                    finalTime.writeVarInt(timeLen);
                    finalTime.writeVarInt(timeId);
                    finalTime.writeBytes(timePayload.vector());
                    client.sendRawBytes(finalTime.vector().data(), finalTime.vector().size());
                    LOG_INFO("Tempo impostato a ", arg, " per ", client.getIp());
                }
                return;
            }

            if (packetId == 0x05) { // Serverbound Chat Message
                std::string message = payload.readString();
                LOG_INFO("[", client.getIp(), "] Messaggio: ", message);
                return;
            }

            if (packetId == 0x21) { // Player Action (Digging)
                int32_t status = payload.readVarInt();
                int64_t location = payload.readLong();
                int8_t face = payload.readByte(); (void)face;
                int32_t sequence = payload.readVarInt();
                
                // Decodifica la posizione dal Long codificato
                int bx = static_cast<int>(location >> 38);
                int by = static_cast<int>((location << 52) >> 52); // 12 bit con segno
                int bz = static_cast<int>((location << 26) >> 38);
                // Aggiusta segno
                if (bx >= (1 << 25)) bx -= (1 << 26);
                if (by >= (1 << 11)) by -= (1 << 12);
                if (bz >= (1 << 25)) bz -= (1 << 26);

                if (status == 0 || status == 2) { // Started/Finished Digging
                    // In creative, status=0 = instant break
                    // In survival, status=2 = finished digging
                    World* world = client.getWorld();
                    if (world) {
                        uint16_t oldBlock = world->getBlock(bx, by, bz);
                        world->setBlock(bx, by, bz, 0); // AIR - gestisce update e fluidi internamente
                        
                        // Genera un item entity
                        if (oldBlock != 0 && oldBlock != 80 && oldBlock != 79) { 
                            // In 1.20.4, diamo lo stesso ID per l'item (28 = Dirt per ora come test)
                            world->spawnItem(bx + 0.5, by + 0.5, bz + 0.5, 28);
                        }
                    }
                }
                
                AcknowledgeBlockChangePacket ackPacket(sequence);
                client.sendPacket(ackPacket);
                return;
            }

            if (packetId == 0x38) { // Use Item On (Block placement)
                int32_t hand = payload.readVarInt(); (void)hand;
                int64_t location = payload.readLong();
                int32_t face = payload.readVarInt();
                float cursorX = payload.readFloat(); (void)cursorX;
                float cursorY = payload.readFloat(); (void)cursorY;
                float cursorZ = payload.readFloat(); (void)cursorZ;
                bool insideBlock = payload.readBoolean(); (void)insideBlock;
                int32_t sequence = payload.readVarInt();

                // Decodifica la posizione
                int bx = static_cast<int>(location >> 38);
                int by = static_cast<int>((location << 52) >> 52);
                int bz = static_cast<int>((location << 26) >> 38);
                if (bx >= (1 << 25)) bx -= (1 << 26);
                if (by >= (1 << 11)) by -= (1 << 12);
                if (bz >= (1 << 25)) bz -= (1 << 26);

                // Offset in base alla faccia cliccata
                switch (face) {
                    case 0: by--; break; // Bottom
                    case 1: by++; break; // Top
                    case 2: bz--; break; // North
                    case 3: bz++; break; // South
                    case 4: bx--; break; // West
                    case 5: bx++; break; // East
                    default: break;
                }

                World* world = client.getWorld();
                if (world) {
                    uint16_t blockToPlace = 1; // Stone come placeholder
                    world->setBlock(bx, by, bz, blockToPlace); // Gestisce update e fluidi internamente
                }

                AcknowledgeBlockChangePacket ackPacket(sequence);
                client.sendPacket(ackPacket);
                return;
            }
            
            if (packetId == 0x35) { // Use Item (Air)
                int32_t hand = payload.readVarInt(); (void)hand;
                int32_t sequence = payload.readVarInt();
                AcknowledgeBlockChangePacket ackPacket(sequence);
                client.sendPacket(ackPacket);
                return;
            }

            LOG_INFO("Ricevuto pacchetto Play ID: 0x", std::hex, packetId);
            return;
        }

        LOG_WARN("Pacchetto sconosciuto o non gestito nello stato attuale! ID: 0x", std::hex, packetId);
    }

    ByteBuffer PacketHandler::serializePacket(const Packet& packet) {
        ByteBuffer payloadBuf;
        packet.write(payloadBuf);

        int32_t packetId = packet.getId();
        size_t packetIdSize = ByteBuffer::getVarIntSize(packetId);
        int32_t packetLength = static_cast<int32_t>(packetIdSize + payloadBuf.size());

        ByteBuffer finalBuffer;
        finalBuffer.writeVarInt(packetLength);
        finalBuffer.writeVarInt(packetId);
        finalBuffer.writeBytes(payloadBuf.vector());

        return finalBuffer;
    }

} 


