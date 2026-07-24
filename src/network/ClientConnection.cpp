#include "ClientConnection.hpp"
#include "protocol/PacketHandler.hpp"
#include "../utils/Logger.hpp"
#include "protocol/ByteBuffer.hpp"
#include "protocol/Packet.hpp"

#include <cstdint>
#include <utility>
#include <winerror.h>
#include <winscard.h>
#include <winsock.h>

ClientConnection::ClientConnection(Socket socket, std::string ip, uint16_t port, class World* world)
    : m_socket(std::move(socket)), m_ip(std::move(ip)), m_port(port), m_world(world) {
        m_socket.setNonBlocking(true);
        m_rxBuffer.reserve(4096);
        m_txBuffer.reserve(4096);
        LOG_INFO("Nuova connessione registrata da ", m_ip, ":", m_port);
} 

ClientConnection::~ClientConnection() { disconnect(); }

void ClientConnection::handleRead() {
    if (readIncomingData()) {
        if (!mc::PacketHandler::processIncomingData(*this, m_rxBuffer)) {
            disconnect();
        }
    }
}


void ClientConnection::sendPacket(const mc::Packet& packet) {
    ByteBuffer buffer = mc::PacketHandler::serializePacket(packet);
    sendRawBytes(buffer.vector().data(), buffer.vector().size());
}

bool ClientConnection::readIncomingData(){
    if(!m_connected) return false;

    uint8_t tempBuffer[4096];
    int bytesRead = ::recv(m_socket.getHandle(), reinterpret_cast<char*>(tempBuffer), sizeof(tempBuffer), 0);

    if(bytesRead > 0){
        m_rxBuffer.insert(m_rxBuffer.end(), tempBuffer, tempBuffer + bytesRead);
        return true;
    }
    else if(bytesRead == 0){
        // Il client ha chiuso la connessione normalmente
        LOG_INFO("Client ", m_ip, ":", m_port, " si e' disconnesso.");
        disconnect();
        return false;
    }
    else{
        int err = WSAGetLastError();
        if(err != WSAEWOULDBLOCK){
            LOG_ERROR("Errore durante la recv() da ", m_ip, ": ", err);
            disconnect();
            return false;
        }
    }
    return true;
}

bool ClientConnection::flushOutgoingData(){
    if(!m_connected || m_txBuffer.empty()) return true;

    int bytesSent = ::send(m_socket.getHandle(), 
                           reinterpret_cast<const char*>(m_txBuffer.data()), 
                           static_cast<int>(m_txBuffer.size()), 0);

    if(bytesSent > 0){
        m_txBuffer.erase(m_txBuffer.begin(), m_txBuffer.begin() + bytesSent);
        return true;
    }
    else{
        int err = WSAGetLastError();
        if(err != WSAEWOULDBLOCK){
            LOG_ERROR("Errore durante la send() verso ", m_ip, ": ", err);
            disconnect();
            return false;
        }
    }
    return true;
}

void ClientConnection::sendRawBytes(const uint8_t* data, size_t length){
    if(!m_connected) return;
    m_txBuffer.insert(m_txBuffer.end(), data, data + length);
}

void ClientConnection::disconnect(){ 
    if(m_connected){
        m_connected = false;
        m_socket.close();
        LOG_INFO("Connessione chiusa per ", m_ip, ":", m_port);
    }
} 

