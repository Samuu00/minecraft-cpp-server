#include "Socket.hpp"
#include "../utils/Logger.hpp"

#include <cstdint>
#include <psdk_inc/_ip_types.h>
#include <psdk_inc/_socket_types.h>
#include <winsock.h>
#include <winsock2.h>
#include <ws2tcpip.h>

Socket::Socket() = default;

Socket::Socket(SOCKET handle) : m_handle(handle){}

Socket::~Socket() { close(); }

Socket::Socket(Socket&& other) noexcept : m_handle(other.m_handle){
    other.m_handle = INVALID_SOCKET;
}

Socket& Socket::operator=(Socket&& other) noexcept {
    if(this != &other){
        close();
        m_handle = other.m_handle;
        other.m_handle = INVALID_SOCKET;
    }
    return *this;
}

bool Socket::create(){
    close();
    m_handle = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(m_handle == INVALID_SOCKET){
        LOG_ERROR("Creazione del socket fallita con errore Winsock: ", WSAGetLastError());
        return false;
    }
    return true;
}

bool Socket::bind(uint16_t port){
    if(!isValid()) return false;

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (::bind(m_handle, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == SOCKET_ERROR) {
        LOG_ERROR("Bind fallito sulla porta ", port, ". Errore Winsock: ", WSAGetLastError());
        return false;
    }
    return true;
}

bool Socket::listen(int backlog){
    if(!isValid()) return false;

    if(::listen(m_handle, backlog) == SOCKET_ERROR){
        LOG_ERROR("Listen fallita. Errore Winsock: ", WSAGetLastError());
        return false;
    }
    return true;
}

bool Socket::setNonBlocking(bool enable) {
    if (!isValid()) return false;

    u_long mode = enable ? 1 : 0;
    if (::ioctlsocket(m_handle, FIONBIO, &mode) == SOCKET_ERROR) {
        LOG_ERROR("Impostazione Non-Blocking fallita. Errore: ", WSAGetLastError());
        return false;
    }

    return true;
}

bool Socket::setReuseAddr(bool enable) {
    if (!isValid()) return false;

    int optval = enable ? 1 : 0;
    if (::setsockopt(m_handle, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&optval), sizeof(optval)) == SOCKET_ERROR) {
        LOG_ERROR("SetReuseAddr fallito. Errore: ", WSAGetLastError());
        return false;
    }

    return true;
}

SOCKET Socket::accept(std::string& clientIp, uint16_t& clientPort) const {
    if(!isValid()) return INVALID_SOCKET;

    sockaddr_in clientAddr{};
    int addrLen = sizeof(clientAddr);

    SOCKET clientSocket = ::accept(m_handle, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen); 
    if(clientSocket != INVALID_SOCKET){
        char ipStr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), ipStr, INET_ADDRSTRLEN);
        clientIp = ipStr;
        clientPort = ntohs(clientAddr.sin_port);
    }
    return clientSocket;
}

void Socket::close(){
    if(m_handle != INVALID_SOCKET){
        ::closesocket(m_handle);
        m_handle = INVALID_SOCKET;
    }
}


