cmake_minimum_required(VERSION 3.20)
project(MinecraftCppServer VERSION 0.1.0 LANGUAGES CXX)

# Imposta C++20 come standard globale
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Esporta i comandi di compilazione per clangd/IDE
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Opzioni di compilazione strict per Clean Code
if(MSVC)
    add_compile_options(/W4 /WX /permissive- /utf-8)
else()
    add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# Fetch nlohmann_json
include(FetchContent)
FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(json)

# Definizione dei sorgenti dell'applicazione
set(SERVER_SOURCES
    main.cpp
    src/core/Server.cpp
    src/core/World.cpp
    src/world/Chunk.cpp
    src/core/Player.cpp
    src/core/Entity.cpp
    src/network/Socket.cpp
    src/network/NetworkServer.cpp
    src/network/ClientConnection.cpp
    src/protocol/ByteBuffer.cpp
    src/protocol/PacketHandler.cpp
    src/utils/Logger.cpp
)

# Target Principale: Eseguibile del Server
add_executable(mc_server ${SERVER_SOURCES})

# Include directory per i sorgenti
target_include_directories(mc_server PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Linking Dipendenze Specifiche per OS
target_link_libraries(mc_server PRIVATE nlohmann_json::nlohmann_json)

if(WIN32)
    # Winsock2 Library per il networking su Windows
    target_link_libraries(mc_server PRIVATE ws2_32)
endif()

# ------------------------------------------------------------------------------
# Sezione Testing (GoogleTest via FetchContent)
# ------------------------------------------------------------------------------
option(BUILD_TESTS "Compila la suite di Unit Test" ON)

if(BUILD_TESTS)
    enable_testing()
    include(FetchContent)

    FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
    )
    # Per Windows MSVC: evita di sovrascrivere l'impostazione della CRT dinamica/statica
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    set(TEST_SOURCES
        tests/test_bytebuffer.cpp
        # File di produzione necessari per gli unit test (senza main.cpp)
        src/protocol/ByteBuffer.cpp
    )

    add_executable(unit_tests ${TEST_SOURCES})
    target_include_directories(unit_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)
    target_link_libraries(unit_tests PRIVATE gtest_main)
    if(WIN32)
        target_link_libraries(unit_tests PRIVATE ws2_32)
    endif()

    include(GoogleTest)
    gtest_discover_tests(unit_tests)
endif()