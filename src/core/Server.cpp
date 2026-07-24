#include "Server.hpp"
#include "World.hpp"
#include "../network/NetworkServer.hpp"
#include "../utils/Logger.hpp"

#include <chrono>
#include <cstdint>
#include <inaddr.h>
#include <memory>
#include <thread>
#include <winnt.h>

Server::Server(uint16_t port)
    : m_port(port){ 
        init();
}

Server::~Server() {
    if (m_running) {
        stop();
    }
}

void Server::init(){
    LOG_INFO("Inizializzazione sottosistemi core e network...");
    
    m_world = std::make_unique<World>();
    m_networkServer = std::make_unique<NetworkServer>(m_port, m_world.get());

    LOG_INFO("Inizializzazione completata.");
}

void Server::run(){
    m_running = true;

    using clock = std::chrono::high_resolution_clock;
    constexpr auto targetTickDuration = std::chrono::milliseconds(50);

    auto lastTime = clock::now();

    LOG_INFO("Game Loop avviato (Target: 20 TPS / 50ms per tick).");

    while(m_running){
        auto startTime = clock::now();

        std::chrono::duration<double> elapsedTime = startTime - lastTime;
        lastTime = startTime;

        if(m_networkServer) m_networkServer->pollEvents();
        
        tick(elapsedTime.count());

        auto workDuration = clock::now() - startTime;
        if(workDuration < targetTickDuration){
            std::this_thread::sleep_for(targetTickDuration - workDuration);

        } else{

            LOG_WARN("Tick lento! Il server ha impiegato ", 
                     std::chrono::duration_cast<std::chrono::milliseconds>(workDuration).count(), 
                     "ms (Target: 50ms)");
        }
    }

    LOG_INFO("Arresto del Game Loop in corso...");
}

void Server::tick(double deltaTime){
    m_tickCounter++;

    if(m_networkServer) m_networkServer->tick(m_tickCounter);
    if(m_world) m_world->tick(deltaTime);
}

void Server::stop(){
    LOG_INFO("Richiesta di stop ricevuta...");
    m_running = false;
    if(m_networkServer) m_networkServer->stop();
}
