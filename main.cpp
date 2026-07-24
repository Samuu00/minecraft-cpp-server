#include <cstdint>
#include <exception>
#include <atomic>
#include <minwindef.h>
#include <psdk_inc/_wsadata.h>

#ifdef _WIN32
    #ifndef WIN32_LEAN_AND_MEAN
    #endif
    #include <winsock2.h>
    #include <windows.h>
#endif

#include "core/Server.hpp"
#include "utils/Logger.hpp"

static std::atomic<bool> g_serverRunning{true};
static Server* g_serverInstance = nullptr;

#ifdef _WIN32
BOOL WINAPI ConsoleHandler(DWORD signal){
    if(signal == CTRL_C_EVENT || signal == CTRL_CLOSE_EVENT || signal == CTRL_BREAK_EVENT){
        LOG_INFO("Ricevuto segnale di interruzione. Avvio shutdown del server...");
        g_serverRunning = false;
        if(g_serverInstance){
            g_serverInstance->stop();
        }
        return TRUE;
    }
    return FALSE;
}
#endif

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv){
    Logger::init();
    LOG_INFO("Avvio minecraft server...");

#ifdef _WIN32
    WSADATA wsaData;
    int wsaResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if(wsaResult != 0){
        LOG_ERROR("WSAStartup fallito, errore: ", wsaResult);
        return 1;
    }
    LOG_INFO("Winsock 2.2 inizializzato con successo.");
 
    if (!SetConsoleCtrlHandler(ConsoleHandler, TRUE)) {
        LOG_WARN("Impossibile impostare il ConsoleCtrlHandler di Windows.");
    }
#endif

    try{
        constexpr uint16_t SERVER_PORT = 25565;
        g_serverInstance = new Server(SERVER_PORT);

        LOG_INFO("Server in ascolto sulla port: ", SERVER_PORT);
        g_serverInstance->run();

        delete g_serverInstance;
        g_serverInstance = nullptr;

    } catch(const std::exception& e){
        LOG_ERROR("Eccezione fatale nel Main loop: ", e.what());
    }

#ifdef _WIN32
    WSACleanup();
    LOG_INFO("Winsock2 de-inizializzato");
#endif

    LOG_INFO("Server arrestato correttamente.");
    return 0;
}
